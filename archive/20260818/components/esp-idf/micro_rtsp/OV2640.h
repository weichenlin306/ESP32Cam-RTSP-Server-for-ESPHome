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
#include "freertos/semphr.h"

extern camera_config_t esp32cam_config, esp32cam_aithinker_config, esp32cam_s3_eye_config, esp32cam_ttgo_t_config;

class OV2640
{
public:
    OV2640(){
        fb = NULL;
        fb_mutex = NULL;
        capture_task_handle = NULL;
        capture_running = false;
    };
    ~OV2640(){
    };
    esp_err_t init(camera_config_t config);
    void run(void);
    size_t getSize(void);
    uint8_t *getfb(void);
    int getWidth(void);
    int getHeight(void);
    framesize_t getFrameSize(void);
    pixformat_t getPixelFormat(void);

    void setFrameSize(framesize_t size);
    void setPixelFormat(pixformat_t format);

    bool lockFrame(void);
    void unlockFrame(void);

private:
    void runIfNeeded(); // grab a frame if we don't already have one
    static void capture_task_func(void *param);

    camera_config_t _cam_config;
    camera_fb_t *fb;
    SemaphoreHandle_t fb_mutex;
    TaskHandle_t capture_task_handle;
    bool capture_running;
};

#endif //OV2640_H_
