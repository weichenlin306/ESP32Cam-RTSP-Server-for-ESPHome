#pragma once

#if defined(USE_ESP_IDF) || defined(ESP_PLATFORM)
#include "platglue-esp32-idf.h"
#elif defined(ARDUINO_ARCH_ESP32)
#include "platglue-esp32.h"
#else
#include "platglue-posix.h"
#endif
