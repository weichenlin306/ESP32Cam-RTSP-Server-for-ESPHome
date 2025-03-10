#pragma once
#include "esphome.h"
#include "micro_rtsp/OV2640.h"
#include "micro_rtsp/OV2640Streamer.h"
#include "micro_rtsp/CRtspSession.h"

namespace esphome {
namespace esp32cam_rtsp_server {

class Esp32camRtsp : public Component {
public:
  // Constructor
  Esp32camRtsp();
  // Destructor
  ~Esp32camRtsp();

  float get_setup_priority() const override;

  WiFiServer rtspServer;
  void setup() override;

  void set_camera(int8_t camera);
  void set_xclk_freq_hz(uint32_t xclk_freq_hz) { this->xclk_freq_hz_ = xclk_freq_hz; } 
  void set_port(uint16_t port) { this->rtsp_port_ = port; }
  void set_resolution(int8_t framesize) { this->framesize_ = (framesize_t)framesize; }

  void set_brightness(int8_t brightness) { this->brightness_ = brightness; }
  void set_contrast(int8_t contrast) { this->contrast_ = contrast; }
  void set_saturation(int8_t saturation) { this->saturation_ = saturation; }
  void set_special_effect(int8_t special_effect) { this->special_effect_ = special_effect; }
  void set_white_balance(bool whitebal) { this->whitebal_ = whitebal?1:0; }
  void set_awb_gain(bool awb_gain) { this->awb_gain_ = awb_gain?1:0; }
  void set_wb_mode(int8_t wb_mode) { this->wb_mode_ = wb_mode; }
  void set_exposure_control(bool exposure_ctrl) { this->exposure_ctrl_ = exposure_ctrl?1:0; }
  void set_aec2(bool aec2) { this->aec2_ = aec2?1:0; }
  void set_ae_level(int8_t ae_level) { this->ae_level_ = ae_level; }
  void set_aec_value(uint16_t aec_value) { this->aec_value_ = aec_value; }
  void set_gain_control(bool gain_ctrl) { this->gain_ctrl_ = gain_ctrl?1:0; }
  void set_agc_gain(int8_t agc_gain) { this->agc_gain_ = agc_gain; }
  void set_gainceiling(int8_t gainceiling) { this->gainceiling_ = gainceiling; }
  void set_bpc(bool bpc) { this->bpc_ = bpc?1:0; }
  void set_wpc(bool wpc) { this->wpc_ = wpc?1:0; }
  void set_raw_gma(bool raw_gma) { this->raw_gma_ = raw_gma?1:0; }
  void set_horizontal_mirror(bool hmirror) { this->hmirror_ = hmirror?1:0; }
  void set_vertical_flip(bool vflip) { this->vflip_ = vflip?1:0; }
  void set_lenc(bool lenc) { this->lenc_ = lenc?1:0; }
  void set_dcw(bool dcw) { this->dcw_ = dcw?1:0; }

  void loop() override;
  void dump_config() override;

protected:
  uint32_t xclk_freq_hz_{16000000};
  uint16_t rtsp_port_{554};
  framesize_t framesize_{FRAMESIZE_SVGA};

  int8_t brightness_{0};
  int8_t contrast_{0};
  int8_t saturation_{0};
  int8_t special_effect_{0};
  int8_t whitebal_{1};
  int8_t awb_gain_{1};
  int8_t wb_mode_{0};
  int8_t exposure_ctrl_{1};
  int8_t aec2_{1};
  int8_t ae_level_{0};
  uint16_t aec_value_{300};
  int8_t gain_ctrl_{0};
  int8_t agc_gain_{0};
  int8_t gainceiling_{0};
  int8_t bpc_{0};
  int8_t wpc_{1};
  int8_t raw_gma_{1};
  int8_t hmirror_{0};
  int8_t vflip_{0};
  int8_t lenc_{1};
  int8_t dcw_{1};

  esp_err_t init_error_{ESP_OK};
};

} // namespace esp32cam_rtsp_server
} // namespace esphome