#include "esphome.h"
#include "micro_rtsp/OV2640.h"
#include "micro_rtsp/OV2640Streamer.h"
#include "micro_rtsp/CRtspSession.h"

#define MSEC_PER_FRAME 100

class Esp32camRtsp : public Component {
private:
  uint16_t rtsp_port;

public:
  // Constructor
  Esp32camRtsp(uint16_t port=554)
    : rtsp_port(port) {};
  // Destructor
  ~Esp32camRtsp() {};

  float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }

  OV2640 cam;  
  WiFiServer rtspServer;

  void setup() override
  {
    // Supported boards: esp32cam_config, esp32cam_aithinker_config, esp32cam_ttgo_t_config
    camera_config_t config = esp32cam_aithinker_config;
    // Possible frame sizes: UXGA(1600x1200),SXGA(1280x1024),XGA(1024x768),SVGA(800x600),VGA(640x480),CIF(400x296),QVGA(320x240),HQVGA(240x176),QQVGA(160x120)
    config.frame_size = FRAMESIZE_SVGA;
    // config.xclk_freq_hz = 10000000;  // default: 20000000
    cam.init(config);

    sensor_t *s = esp_camera_sensor_get();
    // s->set_vflip(s, 1);        // Vertical flip
    // s->set_hmirror(s, 1);      // Horizontal mirror
    // s->set_brightness(s, 1);   // -2 ~ 2
    // s->set_saturation(s, -2);  // -2 ~ 2

    rtspServer = WiFiServer(rtsp_port, 1);
    rtspServer.begin(rtsp_port);
  }

  CStreamer *streamer{nullptr};
  CRtspSession *session{nullptr};
  WiFiClient client;

  void loop() override
  {
    static uint32_t lastFrameTime = millis();

    // If we have an active client connection, just service that until gone
    if (session) {
      // we don't use a timeout here
      // instead we send only if we have new enough frames
      session->handleRequests(0); 

      uint32_t now = millis();
      if (now > lastFrameTime + MSEC_PER_FRAME || /*handle clock rollover*/ now < lastFrameTime) {
        session->broadcastCurrentFrame(now);
        lastFrameTime = now;

        // check if we are overrunning our max frame rate
        now = millis();
        if (now > lastFrameTime + MSEC_PER_FRAME)
          printf("WARNING: The real time-per-frame, %d ms, exceeds that of the current frame rate.\n", now - lastFrameTime);
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
        streamer = new OV2640Streamer(&client, cam);      // our streamer for UDP/TCP based RTP transport
        session = new CRtspSession(&client, streamer);    // our threads RTSP session and state
      }
    }
  }
};
