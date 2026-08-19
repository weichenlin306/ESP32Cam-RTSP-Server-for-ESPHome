#include "OV2640.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "OV2640"

struct ResolutionEntry {
    framesize_t size;
    int width;
    int height;
};

static const ResolutionEntry resolution_table[] = {
    { FRAMESIZE_96X96, 96, 96 },
    { FRAMESIZE_QQVGA, 160, 120 },
    { FRAMESIZE_QCIF, 176, 144 },
    { FRAMESIZE_HQVGA, 240, 176 },
    { FRAMESIZE_240X240, 240, 240 },
    { FRAMESIZE_QVGA, 320, 240 },
    { FRAMESIZE_CIF, 400, 296 },
    { FRAMESIZE_HVGA, 480, 320 },
    { FRAMESIZE_VGA, 640, 480 },
    { FRAMESIZE_SVGA, 800, 600 },
    { FRAMESIZE_XGA, 1024, 768 },
    { FRAMESIZE_HD, 1280, 720 },
    { FRAMESIZE_SXGA, 1280, 1024 },
    { FRAMESIZE_UXGA, 1600, 1200 },
    { FRAMESIZE_FHD, 1920, 1080 },
    { FRAMESIZE_QXGA, 2048, 1536 },
    { FRAMESIZE_QHD, 2560, 1440 },
    { FRAMESIZE_QSXGA, 2560, 1920 }
};

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
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 12,
    .fb_count = 2
};

camera_config_t esp32cam_aithinker_config {

    .pin_pwdn = 32,
    .pin_reset = -1,

    .pin_xclk = 0,

    .pin_sscb_sda = 26,
    .pin_sscb_scl = 27,

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
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 12,
    .fb_count = 2
};

camera_config_t esp32cam_s3_eye_config {

    .pin_pwdn = -1,
    .pin_reset = -1,

    .pin_xclk = 15,

    .pin_sscb_sda = 4,
    .pin_sscb_scl = 5,

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
    .jpeg_quality = 12,
    .fb_count = 2
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
    .jpeg_quality = 12,
    .fb_count = 2
};

camera_fb_t *OV2640::getFrame(void)
{
    return esp_camera_fb_get();
}

void OV2640::returnFrame(camera_fb_t *fb)
{
    if (fb != nullptr) {
        esp_camera_fb_return(fb);
    }
}

int OV2640::getWidth(void)
{
    for (const auto &entry : resolution_table) {
        if (entry.size == _cam_config.frame_size) {
            return entry.width;
        }
    }
    return 640;
}

int OV2640::getHeight(void)
{
    for (const auto &entry : resolution_table) {
        if (entry.size == _cam_config.frame_size) {
            return entry.height;
        }
    }
    return 480;
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
    if (initialized) {
        return ESP_OK;
    }

    memset(&_cam_config, 0, sizeof(_cam_config));
    memcpy(&_cam_config, &config, sizeof(config));

    size_t psram_size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t free_dram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    ESP_LOGI(TAG, "Camera Memory Check: PSRAM total=%zu bytes, Internal Heap free=%zu bytes", psram_size, free_dram);

    if (psram_size > 0) {
        _cam_config.fb_location = CAMERA_FB_IN_PSRAM;
        _cam_config.grab_mode = CAMERA_GRAB_LATEST;
        if (_cam_config.fb_count < 2) {
            _cam_config.fb_count = 2;
        }
    } else {
        _cam_config.fb_location = CAMERA_FB_IN_DRAM;
        _cam_config.fb_count = 1;
        _cam_config.grab_mode = CAMERA_GRAB_LATEST;
        if (_cam_config.frame_size > FRAMESIZE_VGA) {
            ESP_LOGW(TAG, "No PSRAM detected. Capping resolution at VGA (640x480) to prevent DRAM exhaustion.");
            _cam_config.frame_size = FRAMESIZE_VGA;
        }
        if (_cam_config.jpeg_quality < 15) {
            _cam_config.jpeg_quality = 15;
        }
    }

    esp_err_t err = esp_camera_init(&_cam_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera probe failed with error 0x%x (%s)", err, esp_err_to_name(err));
        return err;
    }

    initialized = true;
    ESP_LOGI(TAG, "Camera hardware successfully initialized with on-demand streaming mode.");

    return ESP_OK;
}

