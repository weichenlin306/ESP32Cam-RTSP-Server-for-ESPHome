#include "esphome.h"
#include "micro_rtsp/OV2640.h"
#include "micro_rtsp/OV2640Streamer.h"
#include "micro_rtsp/CRtspSession.h"
#include <vector>

#define MSEC_PER_FRAME 100    // Frame interval, Unit: ms

struct client_stream {
  CStreamer *streamer{nullptr};
  CRtspSession *session{nullptr};
  WiFiClient client;
  uint32_t lastFrameTime;
} clientStreamInit;

class Esp32camRtsp : public Component {
private:
  uint16_t rtsp_port;
  uint8_t  max_client_count;
  // Camera sensor settings
  int8_t _vflip;
  int8_t _hmirror;
  int8_t _brightness;
  int8_t _contrast;
  int8_t _saturation;

public:
  // Constructor
  Esp32camRtsp(uint16_t port=554, uint8_t max_clients=1,
    int8_t vflip=0, int8_t hmirror=0, int8_t brightness=0, int8_t contrast=0, int8_t saturation=0)
    : rtsp_port(port), max_client_count(max_clients),
      _vflip(vflip), _hmirror(hmirror), _brightness(brightness), _contrast(contrast), _saturation(saturation) {};
  // Destructor
  ~Esp32camRtsp() {};

  float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }

  OV2640 cam;  
  WiFiServer rtspServer;
  std::vector<client_stream> clientList;

  void setup() override
  {
    // Supported boards: esp32cam_config, esp32cam_aithinker_config, esp32cam_ttgo_t_config
    camera_config_t config = esp32cam_aithinker_config;
    // Possible frame sizes: UXGA(1600x1200),SXGA(1280x1024),XGA(1024x768),SVGA(800x600),VGA(640x480),CIF(400x296),QVGA(320x240),HQVGA(240x176),QQVGA(160x120)
    config.frame_size = FRAMESIZE_XGA;
    // config.xclk_freq_hz = 10000000;  // default: 20000000
    cam.init(config);

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, _vflip);              // Vertical flip: 0|1
    s->set_hmirror(s, _hmirror);          // Horizontal mirror: 0|1
    s->set_brightness(s, _brightness);    // -2 ~ 2
    s->set_contrast(s, _contrast);        // -2 ~ 2
    s->set_saturation(s, _saturation);    // -2 ~ 2

    rtspServer = WiFiServer(rtsp_port, max_client_count);
    rtspServer.begin(rtsp_port);

    // Allocate client spaces for connection
    clientList.reserve(max_client_count);
    for (int i = 0; i < max_client_count; i++)
      clientList.emplace_back(clientStreamInit);
  }

  void loop() override
  {
    for (int i = 0; i < clientList.size(); i++) {   // Walk through all active/possible clients
      if (clientList[i].session) {
        // If we have an active client connection, just service that until gone
        clientList[i].session->handleRequests(0);   // We don't use a timeout here,
                                                    // instead we send only if we have new enough frames
        uint32_t now = millis();
        if (now > clientList[i].lastFrameTime + MSEC_PER_FRAME || /*handle clock rollover*/ now < clientList[i].lastFrameTime) {
          clientList[i].session->broadcastCurrentFrame(now);
          clientList[i].lastFrameTime = now;

          // Check if we are overrunning our max frame rate
          now = millis();
          if (now > clientList[i].lastFrameTime + MSEC_PER_FRAME)
            printf("WARNING: The real time-per-frame, %d ms, exceeds that of the current frame rate.\n", now - clientList[i].lastFrameTime);
        }

        if(clientList[i].session->m_stopped) {
          delete clientList[i].session;
          delete clientList[i].streamer;
          clientList[i].session = nullptr;
          clientList[i].streamer = nullptr;
        }
      } else {
        clientList[i].client = rtspServer.accept();
        if (clientList[i].client) {
          clientList[i].streamer = new OV2640Streamer(&clientList[i].client, cam);                    // Our streamer for UDP/TCP based RTP transport
          clientList[i].session = new CRtspSession(&clientList[i].client, clientList[i].streamer);    // Our threads RTSP session and state  

          clientList[i].lastFrameTime = millis();   // Initial time for image streamming
        }
      }
    }
  }
};
