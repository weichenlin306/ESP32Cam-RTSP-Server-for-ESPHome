# ESP32Cam-RTSP-Server-for-ESPHome (UNDER CONSTRUCTION)

 ESP32Cam RTSP Server for ESPHome (revision for ESPHome 2025.7.0 and later)

## Based on

- Micro-RTSP: <https://github.com/geeksville/Micro-RTSP>
- esp32cam-rtsp: <https://github.com/rzeldent/esp32cam-rtsp>

## Breaking Changes

- Key RSTP transmission infrastructure has been revised to suite ESPHome 2025.7.0 and later
- Both Arduino framework and ESP-IDF framework are supported, respectively
- Upgrades is assisted by AI assistant, Antigravity
- The micro_rtsp folder is no longer needed; it is now a specified submodule under each framework folder
- The module under ESP-IDF framework uses more modern RTP/MJPEG transmission features than the one under Arduino framework, requiring a newer decoder to decode
- The module under Arduino framework is more compatible with older ESP32Cam boards, but it is not recommended to use it for new projects

## Usage

- Place "esp32cam-rtsp-server" and "micro_rtsp" folders in the /config/esphome/components folder
- Use esp32cam-rtsp-server as an external component in ESPHome
- Configure your YAML project file

  - The component "psram" is autoloaded, so you don't need to add it to your YAML file
  - Configure your YAML file as follows:

        esphome:
          # Change to your preferred camera name
          name: test-cam    
          friendly_name: test-cam
          includes: components/micro_rtsp

        external_components:
          - source: components

        esp32:
          board: esp32dev
          framework:
            # Choose between arduino and esp-idf
            type: arduino   

        # Enable logging
        logger:

        # Enable Home Assistant API
        api:
          encryption:
            # Change to your preferred key
            key: "9dk/J/MEmJycgU0ZfO2ZkvSoalMQVYzmjT4WKAor8L8=" 

        ota:
          - platform: esphome
            # Change to your preferred password
            password: "2a96d7ecbd32092ef6a7a8c78ccd9cc2" 

        wifi:
            ssid: "your_ssid"
          password: "your_password"

          # Optional: Set a static IP address
          manual_ip:
            static_ip: 192.168.1.100
            gateway: 192.168.1.254
            subnet: 255.255.255.0
            dns1: 192.168.1.254

          # Enable fallback hotspot (captive portal) in case wifi connection fails
          ap:
            ssid: "Test-Cam Fallback Hotspot"
            password: "oAcws4IqWTxf"

        captive_portal:

        esp32cam_rtsp_server:
          # Optional, default value is esp32cam_aithinker
          # Available camera types: esp32cam_aithinker, esp32cam_s3_eye, esp32cam_ttgo_t
          camera: esp32cam_aithinker
          # Optional, default value is 20000000 (20MHz)
          # Available values: 10000000 - 20000000
          external_clock_frequency: 20000000
          # Optional, default value is 5 pfs
          # Available values: 1 - 60
          max_framerate: 5 pfs
          # Optional, default value is 554
          port: 554
          # Required, available values: UXGA(1600x1200), SXGA(1280x1024), XGA(1024x768), SVGA(800x600), VGA(640x480), CIF(400x296), QVGA(320x240), HQVGA(240x176), QQVGA(160x120)
          resolution: VGA
          # Optional, default value is 0
          # Available values: -2 - 2
          brightness: 0
          # Optional, default value is 0
          # Available values: -2 - 2
          contrast: 0
          # Optional, default value is 0
          # Available values: -2 - 2
          saturation: 0
          # Optional, default value is none
          # Available values: none, negative, grayscale, red_tint, green_tint, blue_tint, sepia
          special_effect: none
          # Optional, default value is true
          white_balance: true
          # Optional, default value is true
          awb_gain: true
          # Optional, default value is auto
          # Available values: auto, sunny, cloudy, office, home
          wb_mode: auto
          # Optional, default value is true
          exposure_control: true
          # Optional, default value is true
          aec2: true
          # Optional, default value is 0
          # Available values: -2 - 2
          ae_level: 0
          # Optional, default value is 300
          # Available values: 0 - 1200
          aec_value: 300
          # Optional, default value is false
          gain_control: false
          # Optional, default value is 6
          # Available values: 0 - 30
          agc_gain: 6
          *# Optional, default value is 2x*
          *# Available values: 2x, 4x, 8x, 16x, 32x, 64x, 128x*
          gain_ceiling: 2x
          # Optional, default value is false
          bpc: false
          # Optional, default value is true
          wpc: true
          # Optional, default value is true
          raw_gma: true
          # Optional, default value is false
          horizontal_mirror: false
          # Optional, default value is false
          vertical_flip: false 
          # Optional, default value is true
          lenc: true
          # Optional, default value is true
          dcw: true

    Please refer to https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/ for more settings.

- Use VLC to open stream "rtsp://YOUR_RTSP_SERVER_IP:PORT/mjpeg/1" to test RTSP server function

## References

- Micro-RTSP: <https://github.com/geeksville/Micro-RTSP>
- esp32cam-rtsp: <https://github.com/rzeldent/esp32cam-rtsp>
- 夜巿小霸王 <https://youyouyou.pixnet.net/blog/post/120778494>
- Micro-RTSP代碼解析
  - <https://blog.csdn.net/katerdaisy/article/details/128318785>
  - <https://blog.csdn.net/katerdaisy/article/details/128345987>
  - <https://blog.csdn.net/katerdaisy/article/details/128483624>
- Lewis Van Winkle (2019) Hands-On Network Programming with C.
