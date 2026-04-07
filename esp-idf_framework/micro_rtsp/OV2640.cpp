#include "OV2640.h"

#define TAG "OV2640"

// definitions appropriate for the ESP32-CAM devboard (and most clones)
camera_config_t esp32cam_config {

    .pin_pwdn = -1,
    .pin_reset = 15,

    .pin_xclk = 27,

    .pin_sscb_sda = 25,
    .pin_sscb_scl = 23,

    .pin_d7 = 19,
    .pin_d6 = 36,
    .pin_d5 = 18,
    .pin_d4 = 39,
    .pin_d3 = 5,
    .pin_d2 = 34,
    .pin_d1 = 35,
    .pin_d0 = 17,
    .pin_vsync = 22,
    .pin_href = 26,
    .pin_pclk = 21,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    // .frame_size = FRAMESIZE_UXGA, // needs 234K of framebuffer space
    // .frame_size = FRAMESIZE_SXGA, // needs 160K for framebuffer
    // .frame_size = FRAMESIZE_XGA, // needs 96K or even smaller FRAMESIZE_SVGA - can work if using only 1 fb
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 12,               //0-63 lower numbers are higher quality
    .fb_count = 2 // if more than one i2s runs in continous mode.  Use only with jpeg
};

camera_config_t esp32cam_aithinker_config {

    .pin_pwdn = 32,
    .pin_reset = -1,

    .pin_xclk = 0,

    .pin_sscb_sda = 26,
    .pin_sscb_scl = 27,

    // Note: LED GPIO is apparently 4 not sure where that goes
    // per https://github.com/donny681/ESP32_CAMERA_QR/blob/e4ef44549876457cd841f33a0892c82a71f35358/main/led.c
    .pin_d7 = 35,
    .pin_d6 = 34,
    .pin_d5 = 39,
    .pin_d4 = 36,
    .pin_d3 = 21,
    .pin_d2 = 19,
    .pin_d1 = 18,
    .pin_d0 = 5,
    .pin_vsync = 25,
    .pin_href = 23,
    .pin_pclk = 22,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_1,
    .ledc_channel = LEDC_CHANNEL_1,
    .pixel_format = PIXFORMAT_JPEG,
    // .frame_size = FRAMESIZE_UXGA, // needs 234K of framebuffer space
    // .frame_size = FRAMESIZE_SXGA, // needs 160K for framebuffer
    // .frame_size = FRAMESIZE_XGA, // needs 96K or even smaller FRAMESIZE_SVGA - can work if using only 1 fb
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 12,               //0-63 lower numbers are higher quality
    .fb_count = 2 // if more than one i2s runs in continous mode.  Use only with jpeg
};

camera_config_t esp32cam_s3_eye_config {

    .pin_pwdn = -1,
    .pin_reset = -1,

    .pin_xclk = 15,

    .pin_sscb_sda = 4,
    .pin_sscb_scl = 5,

    // Note: LED GPIO is apparently 4 not sure where that goes
    // per https://github.com/donny681/ESP32_CAMERA_QR/blob/e4ef44549876457cd841f33a0892c82a71f35358/main/led.c
    .pin_d7 = 16,
    .pin_d6 = 17,
    .pin_d5 = 18,
    .pin_d4 = 12,
    .pin_d3 = 10,
    .pin_d2 = 8,
    .pin_d1 = 9,
    .pin_d0 = 11,
    .pin_vsync = 6,
    .pin_href = 7,
    .pin_pclk = 13,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_1,
    .ledc_channel = LEDC_CHANNEL_1,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 12,               //0-63 lower numbers are higher quality
    .fb_count = 2 // if more than one i2s runs in continous mode.  Use only with jpeg
};

camera_config_t esp32cam_ttgo_t_config {

    .pin_pwdn = 26,
    .pin_reset = -1,

    .pin_xclk = 32,

    .pin_sscb_sda = 13,
    .pin_sscb_scl = 12,

    .pin_d7 = 39,
    .pin_d6 = 36,
    .pin_d5 = 23,
    .pin_d4 = 18,
    .pin_d3 = 15,
    .pin_d2 = 4,
    .pin_d1 = 14,
    .pin_d0 = 5,
    .pin_vsync = 27,
    .pin_href = 25,
    .pin_pclk = 19,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 12, //0-63 lower numbers are higher quality
    .fb_count = 2 // if more than one i2s runs in continous mode.  Use only with jpeg
};




void OV2640::run(void)
{
    camera_fb_t *new_fb = esp_camera_fb_get();
    if (new_fb != NULL) {
        if (fb) {
            esp_camera_fb_return(fb);
        }
        fb = new_fb;
    } else {
        // 如果無法取得新畫面，則繼續使用最後一張成功的畫面 (fb) 來避免畫面閃爍斷流
        ESP_LOGD(TAG, "Failed to get new frame, reusing cached frame");
    }
}

void OV2640::runIfNeeded(void)
{
    int retries = 0;
    while (!fb && retries < 20) {
        run();
        if (!fb) {
            vTaskDelay(pdMS_TO_TICKS(100));
            retries++;
        }
    }
}

int OV2640::getWidth(void)
{
    runIfNeeded();
    return fb ? fb->width : 0;
}

int OV2640::getHeight(void)
{
    runIfNeeded();
    return fb ? fb->height : 0;
}

size_t OV2640::getSize(void)
{
    runIfNeeded();
    return fb ? fb->len : 0;
}

uint8_t *OV2640::getfb(void)
{
    runIfNeeded();
    return fb ? fb->buf : nullptr;
}

framesize_t OV2640::getFrameSize(void)
{
    return _cam_config.frame_size;
}

void OV2640::setFrameSize(framesize_t size)
{
    _cam_config.frame_size = size;
}

pixformat_t OV2640::getPixelFormat(void)
{
    return _cam_config.pixel_format;
}

void OV2640::setPixelFormat(pixformat_t format)
{
    switch (format)
    {
    case PIXFORMAT_RGB565:
    case PIXFORMAT_YUV422:
    case PIXFORMAT_GRAYSCALE:
    case PIXFORMAT_JPEG:
        _cam_config.pixel_format = format;
        break;
    default:
        _cam_config.pixel_format = PIXFORMAT_GRAYSCALE;
        break;
    }
}


esp_err_t OV2640::init(camera_config_t config)
{
    memset(&_cam_config, 0, sizeof(_cam_config));
    memcpy(&_cam_config, &config, sizeof(config));

    esp_err_t err = esp_camera_init(&_cam_config);
    if (err != ESP_OK)
    {
        printf("Camera probe failed with error 0x%x", err);
        return err;
    }
    // ESP_ERROR_CHECK(gpio_install_isr_service(0));

    return ESP_OK;
}
