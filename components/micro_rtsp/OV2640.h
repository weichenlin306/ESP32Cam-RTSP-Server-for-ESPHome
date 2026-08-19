#ifndef OV2640_H_
#define OV2640_H_

#if defined(USE_ESP_IDF) || defined(ESP_PLATFORM)
#include <stdio.h>
#include <string.h>
#else
#include <Arduino.h>
#include <pgmspace.h>
#include <stdio.h>
#endif

#include "esp_log.h"
#include "esp_attr.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern camera_config_t esp32cam_config, esp32cam_aithinker_config, esp32cam_s3_eye_config, esp32cam_ttgo_t_config;

class OV2640
{
public:
    OV2640() : initialized(false) {};
    ~OV2640() {};

    esp_err_t init(camera_config_t config);

    // 直通取幀與歸還介面（無鎖、低延遲、高頻寬效率）
    camera_fb_t *getFrame(void);
    void returnFrame(camera_fb_t *fb);

    void run(void) {};
    int getWidth(void);
    int getHeight(void);
    framesize_t getFrameSize(void);
    pixformat_t getPixelFormat(void);

    void setFrameSize(framesize_t size);
    void setPixelFormat(pixformat_t format);

private:
    camera_config_t _cam_config;
    bool initialized;
};

#endif //OV2640_H_

