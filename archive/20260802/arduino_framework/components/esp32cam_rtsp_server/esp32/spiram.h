#pragma once
#include "esp_psram.h"

// Provide backward compatibility for older esp32-camera SDK with IDF 5 (Arduino 3.0)
#ifndef esp_spiram_is_initialized
#define esp_spiram_is_initialized esp_psram_is_initialized
#endif

#ifndef esp_spiram_test
#define esp_spiram_test esp_psram_extram_test
#endif
