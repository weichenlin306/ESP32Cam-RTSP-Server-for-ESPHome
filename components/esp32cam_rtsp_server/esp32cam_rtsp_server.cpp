#include "esp32cam_rtsp_server.h"

#define MSEC_PER_FRAME 150    // Frame interval, Unit: ms

namespace esphome {
namespace esp32cam_rtsp_server {

int8_t camera_{1};
// Camera board config
camera_config_t camera_config[] = {
  esp32cam_config,
  esp32cam_aithinker_config,
  esp32cam_ttgo_t_config
};
camera_config_t config;

OV2640 cam;
CStreamer *streamer{nullptr};
CRtspSession *session{nullptr};
WiFiClient client;
uint32_t lastFrameTime;

static const char *const TAG = "camera_config";

Esp32camRtsp::Esp32camRtsp() {};
Esp32camRtsp::~Esp32camRtsp() {};

void  Esp32camRtsp::set_camera(int8_t camera) { camera_ = camera; }
float Esp32camRtsp::get_setup_priority() const { return esphome::setup_priority::AFTER_WIFI; }

void Esp32camRtsp::setup()
{
  ESP_LOGCONFIG(TAG, "Setting up rtsp server...");
  // Supported camera boards: esp32cam_config, esp32cam_aithinker_config, esp32cam_ttgo_t_config
  config = camera_config[camera_];

  // Possible frame sizes: UXGA(1600x1200),SXGA(1280x1024),XGA(1024x768),SVGA(800x600),VGA(640x480),CIF(400x296),QVGA(320x240),HQVGA(240x176),QQVGA(160x120)
  config.frame_size = framesize_;
  config.xclk_freq_hz = xclk_freq_hz_;  // min. 10000000, max. 20000000
  esp_err_t err = cam.init(config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_camera_init failed: %s", esp_err_to_name(err));
    ESP_LOGE(TAG, "Wait for 5 seconds to restart...");
    delay(5000);
    ESP.restart();
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, vflip_);              // Vertical flip: 0|1
  s->set_hmirror(s, hmirror_);          // Horizontal mirror: 0|1
  s->set_brightness(s, brightness_);    // -2 ~ 2
  s->set_contrast(s, contrast_);        // -2 ~ 2
  s->set_saturation(s, saturation_);    // -2 ~ 2
  s->set_special_effect(s, special_effect_); 	// 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, whitebal_);       	// 0 = disable , 1 = enable
  s->set_awb_gain(s, awb_gain_);       	// 0 = disable , 1 = enable
  s->set_wb_mode(s, wb_mode_); 	        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, exposure_ctrl_);  	// 0 = disable , 1 = enable
  s->set_aec2(s, aec2_);           	    // 0 = disable , 1 = enable
  s->set_ae_level(s, ae_level_);       	// -2 to 2
  s->set_aec_value(s, aec_value_);    	// 0 to 1200
  s->set_gain_ctrl(s, gain_ctrl_);	    // 0 = disable , 1 = enable
  s->set_agc_gain(s, agc_gain_);        // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)gainceiling_);  // 0 to 6
  s->set_bpc(s, bpc_);            	    // 0 = disable , 1 = enable
  s->set_wpc(s, wpc_);            	    // 0 = disable , 1 = enable
  s->set_raw_gma(s, raw_gma_);        	// 0 = disable , 1 = enable
  s->set_lenc(s, lenc_);           	    // 0 = disable , 1 = enable
  s->set_dcw(s, dcw_);            	    // 0 = disable , 1 = enable

  rtspServer = WiFiServer(rtsp_port_, 1);
  rtspServer.begin(rtsp_port_);
}

void Esp32camRtsp::loop()
{
  if (session) {
    // If we have an active client connection, just service that until gone
    session->handleRequests(0);   // We don't use a timeout here,
                                                // instead we send only if we have new enough frames
    uint32_t now = millis();
    if (now > lastFrameTime + MSEC_PER_FRAME || /*handle clock rollover*/ now < lastFrameTime) {
      session->broadcastCurrentFrame(now);
      lastFrameTime = now;

      // Check if we are overrunning our max frame rate
      now = millis();
      if (now > lastFrameTime + MSEC_PER_FRAME)
        ESP_LOGCONFIG(TAG, "WARNING: The real time-per-frame, %d ms, exceeds that of the current frame rate.\n", now - lastFrameTime);
    }

    if(session->m_stopped) {
      delete session;
      delete streamer;
      session = nullptr;
      streamer = nullptr;
    }
  } else {
    client = rtspServer.accept();
    if (client) {
      streamer = new OV2640Streamer(&client, cam);      // Our streamer for UDP/TCP based RTP transport
      session = new CRtspSession(&client, streamer);    // Our threads RTSP session and state  

      lastFrameTime = millis();   // Initial time for image streamming
    }
  }
}

void Esp32camRtsp::dump_config() {
  String camera_names[] = { "ESP32 Camera", "ESP32 Camera - Ai-Thinker", "ESP32 Camera - TTGO T-Display" };
  ESP_LOGCONFIG(TAG, "%s", camera_names[camera_].c_str());
  ESP_LOGCONFIG(TAG, "  Data Pins: D0:%d D1:%d D2:%d D3:%d D4:%d D5:%d D6:%d D7:%d", config.pin_d0, config.pin_d1,
                config.pin_d2, config.pin_d3, config.pin_d4, config.pin_d5, config.pin_d6, config.pin_d7);
  ESP_LOGCONFIG(TAG, "  VSYNC Pin: %d", config.pin_vsync);
  ESP_LOGCONFIG(TAG, "  HREF Pin: %d", config.pin_href);
  ESP_LOGCONFIG(TAG, "  Pixel Clock Pin: %d", config.pin_pclk);
  ESP_LOGCONFIG(TAG, "  External Clock: Pin:%d Frequency:%u", config.pin_xclk, xclk_freq_hz_);
  ESP_LOGCONFIG(TAG, "  I2C Pins: SDA:%d SCL:%d", config.pin_sccb_sda, config.pin_sccb_scl);
  ESP_LOGCONFIG(TAG, "  Reset Pin: %d", config.pin_reset);
  switch (framesize_) {
    case 0:
      ESP_LOGCONFIG(TAG, "  Resolution: 96x96");
      break;
    case 1:
      ESP_LOGCONFIG(TAG, "  Resolution: 160x120 (QQVGA)");
      break;
    case 2:
      ESP_LOGCONFIG(TAG, "  Resolution: 128x128");
      break;
    case 3:
      ESP_LOGCONFIG(TAG, "  Resolution: 176x144 (QCIF)");
      break;
    case 4:
      ESP_LOGCONFIG(TAG, "  Resolution: 240x176 (HQVGA)");
      break;
    case 5:
      ESP_LOGCONFIG(TAG, "  Resolution: 240x240");
      break;
    case 6:
      ESP_LOGCONFIG(TAG, "  Resolution: 320x240 (QVGA)");
      break;
    case 7:
      ESP_LOGCONFIG(TAG, "  Resolution: 320x320");
      break;
    case 8:
      ESP_LOGCONFIG(TAG, "  Resolution: 400x296 (CIF)");
      break;
    case 9:
      ESP_LOGCONFIG(TAG, "  Resolution: 480x320 (HVGA)");
      break;
    case 10:
      ESP_LOGCONFIG(TAG, "  Resolution: 640x480 (VGA)");
      break;
    case 11:
      ESP_LOGCONFIG(TAG, "  Resolution: 800x600 (SVGA)");
      break;
    case 12:
      ESP_LOGCONFIG(TAG, "  Resolution: 1024x768 (XGA)");
      break;
    case 13:
      ESP_LOGCONFIG(TAG, "  Resolution: 1280x720 (HD)");
      break;
    case 14:
      ESP_LOGCONFIG(TAG, "  Resolution: 1280x1024 (SXGA)");
      break;
    case 15:
      ESP_LOGCONFIG(TAG, "  Resolution: 1600x1200 (UXGA)");
      break;
    case 16:
      ESP_LOGCONFIG(TAG, "  Resolution: 1920x1080 (FHD)");
      break;
    case 17:
      ESP_LOGCONFIG(TAG, "  Resolution: 720x1280 (P_HD)");
      break;
    case 18:
      ESP_LOGCONFIG(TAG, "  Resolution: 864x1536 (P_3MP)");
      break;
    case 19:
      ESP_LOGCONFIG(TAG, "  Resolution: 2048x1536 (QXGA)");
      break;
    case 20:
      ESP_LOGCONFIG(TAG, "  Resolution: 2560x1440 (QHD)");
      break;
    case 21:
      ESP_LOGCONFIG(TAG, "  Resolution: 2560x1600 (FWQXGA)");
      break;
    case 22:
      ESP_LOGCONFIG(TAG, "  Resolution: 1080x1920 (P_FHD)");
      break;
    case 23:
      ESP_LOGCONFIG(TAG, "  Resolution: 2560x1920 (QSXGA)");
      break;
    case 24:
      ESP_LOGCONFIG(TAG, "  Resolution: 2592x1944 (5MP)");
      break;
    default:
      break;
  }

  sensor_t *s = esp_camera_sensor_get();
  auto st = s->status;
  ESP_LOGCONFIG(TAG, "  JPEG Quality: %u", st.quality);
  ESP_LOGCONFIG(TAG, "  Framebuffer Count: %u", config.fb_count);
  ESP_LOGCONFIG(TAG, "  Contrast: %d", st.contrast);
  ESP_LOGCONFIG(TAG, "  Brightness: %d", st.brightness);
  ESP_LOGCONFIG(TAG, "  Saturation: %d", st.saturation);
  ESP_LOGCONFIG(TAG, "  Vertical Flip: %s", ONOFF(st.vflip));
  ESP_LOGCONFIG(TAG, "  Horizontal Mirror: %s", ONOFF(st.hmirror));
  ESP_LOGCONFIG(TAG, "  Special Effect: %u", st.special_effect);
  ESP_LOGCONFIG(TAG, "  White Balance Mode: %u", st.wb_mode);
  ESP_LOGCONFIG(TAG, "  Auto White Balance: %s", ONOFF(st.awb));
  ESP_LOGCONFIG(TAG, "  Auto White Balance Gain: %s", ONOFF(st.awb_gain));
  ESP_LOGCONFIG(TAG, "  Auto Exposure Control: %s", ONOFF(st.aec));
  ESP_LOGCONFIG(TAG, "  Auto Exposure Control 2: %s", ONOFF(st.aec2));
  ESP_LOGCONFIG(TAG, "  Auto Exposure Level: %d", st.ae_level);
  ESP_LOGCONFIG(TAG, "  Auto Exposure Value: %u", st.aec_value);
  ESP_LOGCONFIG(TAG, "  AGC: %s", ONOFF(st.agc));
  ESP_LOGCONFIG(TAG, "  AGC Gain: %u", st.agc_gain);
  ESP_LOGCONFIG(TAG, "  Gain Ceiling: %u", st.gainceiling);
  ESP_LOGCONFIG(TAG, "  BPC: %s", ONOFF(st.bpc));
  ESP_LOGCONFIG(TAG, "  WPC: %s", ONOFF(st.wpc));
  ESP_LOGCONFIG(TAG, "  RAW_GMA: %s", ONOFF(st.raw_gma));
  ESP_LOGCONFIG(TAG, "  Lens Correction: %s", ONOFF(st.lenc));
  ESP_LOGCONFIG(TAG, "  DCW: %s", ONOFF(st.dcw));
  ESP_LOGCONFIG(TAG, "  Test Pattern: %s", YESNO(st.colorbar));
}

} // namespace esp32cam_rtsp_server
} // namespace esphome
