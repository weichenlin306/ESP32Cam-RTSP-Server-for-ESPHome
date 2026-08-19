#include "esp32cam_rtsp_server.h"
#include <cinttypes>

namespace esphome {
namespace esp32cam_rtsp_server {

int8_t camera_{2}; // ESP32-S3 預設選用 ESP32CAM_S3_EYE (Index 2)
// Camera board config
camera_config_t camera_config[] = {esp32cam_config, esp32cam_aithinker_config,
                                   esp32cam_s3_eye_config,
                                   esp32cam_ttgo_t_config};
camera_config_t config;
static uint32_t msec_per_frame;

OV2640 cam;
CStreamer *streamer{nullptr};
CRtspSession *session{nullptr};
SOCKET client_fd{-1};

static const char *const TAG = "camera_config";

Esp32camRtsp::Esp32camRtsp() {
  this->factory_defaults_.brightness = 0;
  this->factory_defaults_.contrast = 0;
  this->factory_defaults_.saturation = 0;
  this->factory_defaults_.special_effect = 0;
  this->factory_defaults_.whitebal = 1;
  this->factory_defaults_.awb_gain = 1;
  this->factory_defaults_.wb_mode = 0;
  this->factory_defaults_.exposure_ctrl = 1;
  this->factory_defaults_.aec2 = 1;
  this->factory_defaults_.ae_level = 0;
  this->factory_defaults_.aec_value = 300;
  this->factory_defaults_.gain_ctrl = 0;
  this->factory_defaults_.agc_gain = 6;
  this->factory_defaults_.gainceiling = 0;
  this->factory_defaults_.bpc = 0;
  this->factory_defaults_.wpc = 1;
  this->factory_defaults_.raw_gma = 1;
  this->factory_defaults_.hmirror = 0;
  this->factory_defaults_.vflip = 0;
  this->factory_defaults_.lenc = 1;
  this->factory_defaults_.dcw = 1;
};
Esp32camRtsp::~Esp32camRtsp() {
  this->running_ = false;
  if (this->task_handle_ != nullptr) {
    vTaskDelay(pdMS_TO_TICKS(50));
    this->task_handle_ = nullptr;
  }
  if (server_fd_ >= 0) {
    close(server_fd_);
    server_fd_ = -1;
  }
};

void Esp32camRtsp::set_camera(int8_t camera) { camera_ = camera; }
float Esp32camRtsp::get_setup_priority() const {
  return esphome::setup_priority::LATE;
}

void Esp32camRtsp::setup() {
  ESP_LOGCONFIG(TAG, "Setting up RTSP server...");
  // Supported camera boards: esp32cam_config, esp32cam_aithinker_config,
  // esp32cam_s3_eye_config, esp32cam_ttgo_t_config

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ESP32_S3)
  // Ai-Thinker pins (GPIO 34/35/36/39) do not exist on ESP32-S3.
  // Auto-select ESP32CAM_S3_EYE (index 2) if Ai-Thinker (index 1) or default is set.
  if (camera_ < 0 || camera_ >= 4 || camera_ == 1) {
    ESP_LOGI(TAG, "ESP32-S3 chip detected. Defaulting to ESP32-S3-EYE board pinout.");
    camera_ = 2; // ESP32CAM_S3_EYE
  }
#elif defined(CONFIG_IDF_TARGET_ESP32) || defined(ESP32)
  // ESP32 chip detected (e.g. ESP32-CAM Ai-Thinker).
  // If camera is unconfigured or S3 specific (index 2), default to Ai-Thinker (index 1).
  if (camera_ < 0 || camera_ >= 4 || camera_ == 2) {
    ESP_LOGI(TAG, "ESP32 chip detected. Defaulting to ESP32-CAM Ai-Thinker board pinout.");
    camera_ = 1; // ESP32CAM_AITHINKER
  }
#else
  if (camera_ < 0 || camera_ >= 4) {
    camera_ = 1; // ESP32 default: Ai-Thinker
  }
#endif

  // First: Create and listen on TCP server socket on rtsp_port_ (554)
  server_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_fd_ >= 0) {
    int reuse = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    int flags = fcntl(server_fd_, F_GETFL, 0);
    fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(rtsp_port_);

    if (bind(server_fd_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) {
      if (listen(server_fd_, 1) == 0) {
        ESP_LOGCONFIG(TAG, "RTSP server successfully listening on port %u", rtsp_port_);
      } else {
        ESP_LOGE(TAG, "Failed to listen on RTSP server socket (errno %d)", errno);
        close(server_fd_);
        server_fd_ = -1;
      }
    } else {
      ESP_LOGE(TAG, "Failed to bind RTSP server socket on port %u (errno %d)", rtsp_port_, errno);
      close(server_fd_);
      server_fd_ = -1;
    }
  } else {
    ESP_LOGE(TAG, "Failed to create RTSP server socket (errno %d)", errno);
  }

  // Second: Initialize camera hardware
  config = camera_config[camera_];
  config.frame_size = framesize_;
  config.xclk_freq_hz = xclk_freq_hz_;

  esp_err_t err = cam.init(config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_camera_init failed: %s (%d). Disabling RTSP component.", esp_err_to_name(err), err);
    if (server_fd_ >= 0) {
      close(server_fd_);
      server_fd_ = -1;
    }
    this->mark_failed();
    return;
  }

  // 記錄由 YAML 與 __init__.py 編譯設定整合的最終出廠預設值
  this->factory_defaults_.framesize = framesize_;
  this->factory_defaults_.brightness = brightness_;
  this->factory_defaults_.contrast = contrast_;
  this->factory_defaults_.saturation = saturation_;
  this->factory_defaults_.special_effect = special_effect_;
  this->factory_defaults_.whitebal = whitebal_;
  this->factory_defaults_.awb_gain = awb_gain_;
  this->factory_defaults_.wb_mode = wb_mode_;
  this->factory_defaults_.exposure_ctrl = exposure_ctrl_;
  this->factory_defaults_.aec2 = aec2_;
  this->factory_defaults_.ae_level = ae_level_;
  this->factory_defaults_.aec_value = aec_value_;
  this->factory_defaults_.gain_ctrl = gain_ctrl_;
  this->factory_defaults_.agc_gain = agc_gain_;
  this->factory_defaults_.gainceiling = gainceiling_;
  this->factory_defaults_.bpc = bpc_;
  this->factory_defaults_.wpc = wpc_;
  this->factory_defaults_.raw_gma = raw_gma_;
  this->factory_defaults_.hmirror = hmirror_;
  this->factory_defaults_.vflip = vflip_;
  this->factory_defaults_.lenc = lenc_;
  this->factory_defaults_.dcw = dcw_;

  // Interval between frames
  msec_per_frame = 1000 / max_framerate_;

  this->apply_sensor_settings();

  // Third: Spawn dedicated FreeRTOS Task (Pinned to Core 1 for DMA & cache stability)
  this->running_ = true;
  xTaskCreatePinnedToCore(
      Esp32camRtsp::task_func,
      "rtsp_task",
      8192,
      this,
      5,
      &this->task_handle_,
      1
  );
  ESP_LOGI(TAG, "RTSP server FreeRTOS worker task created on Core 1.");
}

void Esp32camRtsp::task_func(void *pvParameters) {
  auto *self = static_cast<Esp32camRtsp *>(pvParameters);
  self->task_loop();
  vTaskDelete(NULL);
}

void Esp32camRtsp::task_loop() {
  uint32_t last_frame_time = millis();
  uint32_t now;

  while (this->running_) {
    if (session != nullptr) {
      session->handleRequests(0);
      now = millis();

      if (now >= last_frame_time + msec_per_frame || now < last_frame_time) {
        uint32_t frame_start = millis();
        session->broadcastCurrentFrame(now);
        last_frame_time += msec_per_frame;

        now = millis();
        if (now > last_frame_time + msec_per_frame) {
          uint32_t elapsed = now - frame_start;
          ESP_LOGD(TAG,
                   "The real time-per-frame, %" PRIu32 " ms, exceeds that "
                   "of the current frame rate %4.1f fps.",
                   elapsed, max_framerate_);
          last_frame_time = now;
        }
      }

      if (session->m_stopped) {
        delete session;
        delete streamer;
        session = nullptr;
        streamer = nullptr;
        client_fd = -1;
      }
    } else if (server_fd_ >= 0) {
      struct sockaddr_in client_addr;
      socklen_t client_len = sizeof(client_addr);
      int new_client = accept(server_fd_, (struct sockaddr *)&client_addr, &client_len);
      if (new_client >= 0) {
        int flags = fcntl(new_client, F_GETFL, 0);
        fcntl(new_client, F_SETFL, flags | O_NONBLOCK);

        int flag = 1;
        setsockopt(new_client, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

        int send_buf_size = 16384;
        setsockopt(new_client, SOL_SOCKET, SO_SNDBUF, (char *)&send_buf_size, sizeof(send_buf_size));

        client_fd = new_client;
        streamer = new OV2640Streamer(client_fd, cam);
        session = new CRtspSession(client_fd, streamer);
        last_frame_time = millis();
      }
    }

    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

void Esp32camRtsp::loop() {
  // All RTSP streaming processing runs in dedicated FreeRTOS worker task on Core 1
}

void Esp32camRtsp::dump_config() {
  const char *camera_names[] = {"ESP32 Camera", "ESP32 Camera - Ai-Thinker",
                                "ESP32 Camera - S3 EYE", "ESP32 Camera - TTGO T-Display"};
  if (camera_ >= 0 && camera_ < 4) {
    ESP_LOGCONFIG(TAG, "%s", camera_names[camera_]);
  } else {
    ESP_LOGCONFIG(TAG, "ESP32 Camera (Unknown Board: %d)", camera_);
  }
  ESP_LOGCONFIG(TAG,
                "  Data Pins: D0:%d D1:%d D2:%d D3:%d D4:%d D5:%d D6:%d D7:%d",
                config.pin_d0, config.pin_d1, config.pin_d2, config.pin_d3,
                config.pin_d4, config.pin_d5, config.pin_d6, config.pin_d7);
  ESP_LOGCONFIG(TAG, "  VSYNC Pin: %d", config.pin_vsync);
  ESP_LOGCONFIG(TAG, "  HREF Pin: %d", config.pin_href);
  ESP_LOGCONFIG(TAG, "  Pixel Clock Pin: %d", config.pin_pclk);
  ESP_LOGCONFIG(TAG, "  External Clock: Pin:%d Frequency:%" PRIu32, config.pin_xclk,
                xclk_freq_hz_);
  ESP_LOGCONFIG(TAG, "  Maximum Frame Rate: %4.1f fps", max_framerate_);
  ESP_LOGCONFIG(TAG, "  I2C Pins: SDA:%d SCL:%d", config.pin_sccb_sda,
                config.pin_sccb_scl);
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
  if (s != nullptr) {
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
}

void Esp32camRtsp::apply_sensor_settings() {
  sensor_t *s = esp_camera_sensor_get();
  if (s != nullptr) {
    s->set_vflip(s, vflip_);
    s->set_hmirror(s, hmirror_);
    s->set_brightness(s, brightness_);
    s->set_contrast(s, contrast_);
    s->set_saturation(s, saturation_);
    s->set_special_effect(s, special_effect_);
    s->set_whitebal(s, whitebal_);
    s->set_awb_gain(s, awb_gain_);
    s->set_wb_mode(s, wb_mode_);
    s->set_exposure_ctrl(s, exposure_ctrl_);
    s->set_aec2(s, aec2_);
    s->set_ae_level(s, ae_level_);
    s->set_aec_value(s, aec_value_);
    s->set_gain_ctrl(s, gain_ctrl_);
    s->set_agc_gain(s, agc_gain_);
    s->set_gainceiling(s, (gainceiling_t)gainceiling_);
    s->set_bpc(s, bpc_);
    s->set_wpc(s, wpc_);
    s->set_raw_gma(s, raw_gma_);
    s->set_lenc(s, lenc_);
    s->set_dcw(s, dcw_);
  }
}

const char *Esp32camRtsp::get_resolution_name(framesize_t fs) {
  switch (fs) {
    case FRAMESIZE_96X96: return "96X96";
    case FRAMESIZE_QQVGA: return "QQVGA (160x120)";
    case FRAMESIZE_128X128: return "128X128";
    case FRAMESIZE_QCIF: return "QCIF (176x144)";
    case FRAMESIZE_HQVGA: return "HQVGA (240x176)";
    case FRAMESIZE_240X240: return "240X240";
    case FRAMESIZE_QVGA: return "QVGA (320x240)";
    case FRAMESIZE_320X320: return "320X320";
    case FRAMESIZE_CIF: return "CIF (400x296)";
    case FRAMESIZE_HVGA: return "HVGA (480x320)";
    case FRAMESIZE_VGA: return "VGA (640x480)";
    case FRAMESIZE_SVGA: return "SVGA (800x600)";
    case FRAMESIZE_XGA: return "XGA (1024x768)";
    case FRAMESIZE_HD: return "HD (1280x720)";
    case FRAMESIZE_SXGA: return "SXGA (1280x1024)";
    case FRAMESIZE_UXGA: return "UXGA (1600x1200)";
    case FRAMESIZE_FHD: return "FHD (1920x1080)";
    case FRAMESIZE_P_HD: return "P_HD (720x1280)";
    case FRAMESIZE_P_3MP: return "P_3MP (864x1536)";
    case FRAMESIZE_QXGA: return "QXGA (2048x1536)";
    case FRAMESIZE_QHD: return "QHD (2560x1440)";
    case FRAMESIZE_WQXGA: return "WQXGA (2560x1600)";
    case FRAMESIZE_P_FHD: return "P_FHD (1080x1920)";
    case FRAMESIZE_QSXGA: return "QSXGA (2560x1920)";
    default: return "VGA (640x480)";
  }
}

void Esp32camRtsp::set_dynamic_resolution(framesize_t framesize) {
  this->framesize_ = framesize;
  cam.setFrameSize(framesize);
  sensor_t *s = esp_camera_sensor_get();
  if (s != nullptr) {
    s->set_framesize(s, framesize);
    ESP_LOGI(TAG, "Dynamic camera resolution set to: %s (%d)", get_resolution_name(framesize), (int)framesize);
  }
}

void Esp32camRtsp::restore_factory_defaults() {
  ESP_LOGI(TAG, "Restoring camera settings to factory defaults (from YAML and component defaults)...");
  this->set_dynamic_resolution(this->factory_defaults_.framesize);
  this->brightness_ = this->factory_defaults_.brightness;
  this->contrast_ = this->factory_defaults_.contrast;
  this->saturation_ = this->factory_defaults_.saturation;
  this->special_effect_ = this->factory_defaults_.special_effect;
  this->whitebal_ = this->factory_defaults_.whitebal;
  this->awb_gain_ = this->factory_defaults_.awb_gain;
  this->wb_mode_ = this->factory_defaults_.wb_mode;
  this->exposure_ctrl_ = this->factory_defaults_.exposure_ctrl;
  this->aec2_ = this->factory_defaults_.aec2;
  this->ae_level_ = this->factory_defaults_.ae_level;
  this->aec_value_ = this->factory_defaults_.aec_value;
  this->gain_ctrl_ = this->factory_defaults_.gain_ctrl;
  this->agc_gain_ = this->factory_defaults_.agc_gain;
  this->gainceiling_ = this->factory_defaults_.gainceiling;
  this->bpc_ = this->factory_defaults_.bpc;
  this->wpc_ = this->factory_defaults_.wpc;
  this->raw_gma_ = this->factory_defaults_.raw_gma;
  this->hmirror_ = this->factory_defaults_.hmirror;
  this->vflip_ = this->factory_defaults_.vflip;
  this->lenc_ = this->factory_defaults_.lenc;
  this->dcw_ = this->factory_defaults_.dcw;

  this->apply_sensor_settings();
  ESP_LOGI(TAG, "Factory default settings successfully restored.");
}

void Esp32camRtsp::reset_sensor() {
  ESP_LOGI(TAG, "Reinitializing camera sensor registers and JPEG pipeline...");
  sensor_t *s = esp_camera_sensor_get();
  if (s != nullptr) {
    s->reset(s);
    vTaskDelay(pdMS_TO_TICKS(50));
    s->set_pixformat(s, PIXFORMAT_JPEG);
    s->set_framesize(s, this->framesize_);
    this->apply_sensor_settings();
    ESP_LOGI(TAG, "Camera sensor registers and JPEG pipeline reinitialized successfully.");
  } else {
    ESP_LOGE(TAG, "Failed to get camera sensor handle for reset.");
  }
}

void Esp32camRtsp::cold_restart(uint32_t power_down_ms) {
  ESP_LOGI(TAG, "Executing camera hardware power cycle / cold restart (power down for %u ms)...", (unsigned int)power_down_ms);

  if (config.pin_pwdn >= 0) {
    gpio_set_direction((gpio_num_t)config.pin_pwdn, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)config.pin_pwdn, 1); // Power Down (切斷供電/休眠)
    vTaskDelay(pdMS_TO_TICKS(power_down_ms));
    gpio_set_level((gpio_num_t)config.pin_pwdn, 0); // Power On (重新供電喚醒)
    vTaskDelay(pdMS_TO_TICKS(150));
  } else {
    vTaskDelay(pdMS_TO_TICKS(power_down_ms));
  }
  if (config.pin_reset >= 0) {
    gpio_set_direction((gpio_num_t)config.pin_reset, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)config.pin_reset, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level((gpio_num_t)config.pin_reset, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  this->reset_sensor();
  ESP_LOGI(TAG, "Camera hardware cold restart completed successfully.");
}

} // namespace esp32cam_rtsp_server
} // namespace esphome
