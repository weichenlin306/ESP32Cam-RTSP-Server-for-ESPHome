# ESP32Cam-RTSP-Server-for-ESPHome

 ESP32Cam RTSP Server for ESPHome

## Based on

- Micro-RTSP: <https://github.com/geeksville/Micro-RTSP>
- esp32cam-rtsp: <https://github.com/rzeldent/esp32cam-rtsp>

## Modifications & Improvement

- WebServer did not pass compiling on ESPHome and hence the HTTP protocol is totally removed
- Include only those header files being used for camera image capture and pure RTSP connection
- Encapulate Micro-RTSP as a class for use in ESPHome
- Now support multiple simultaneous client connections (2024/7/30)
- Add camera sensor settings as input arguments on class construction
- If the connector of a flexible printed circuit (FPC) of OV2640 is overheated, try to change values in esp32cam-rtsp-server.h
  - #define MSEC_PER_FRAME 200 (default:150)
  - config.xclk_freq_hz = 10000000; (default:16000000, max.=20000000)

## Usage

- Place "esp32cam-rtsp-server.h" file and "micro_rtsp" folder in the same folder as your ESPHome project (.yaml) file
- Configure your YAML project file

  - Add "psram" component to use pseudo SRAM (PSRAM) to avoid "core panic'd" and infinite reboot
  - Put a custom component named "custom_component" and make an Esp32camRtsp() instance using lambda function
  - Syntax

        auto Esp32CamRtspServer = new Esp32camRtsp(
            [RTSP_PORT], [MAX_CLIENT_COUNT], [FRAME_SIZE], [VERTICAL_FLIP], [HORRIZONTAL_MIRROR],
            [BRIGHTNESS], [CONTRAST], [SATURATION], [GAIN_CONTROL], [AGC_GAIN]
        )

        Value Ranges:
          FRAME_SIZE*: FRAMESIZE_UXGA | FRAMESIZE_SXGA | FRAMESIZE_XGA | FRAMESIZE_SVGA |
                      FRAMESIZE_VGA | FRAMESIZE_CIF | FRAMESIZE_QVGA | FRAMESIZE_HQVGA |
                      FRAMESIZE_QQVGA
          VERTICAL_FLIP: 0 | 1
          HORRIZONTAL_MIRROR: 0 | 1
          BRIGHTNESS: -2 ~ 2 (integer)
          CONTRAST:   -2 ~ 2 (integer)
          SATURATION: -2 ~ 2 (integer)
          GAIN_CONTROL: 0 | 1 (default=1, enabled)
          AGC_GAIN: 0 ~ 30 (default = 6) (effective if GAIN_CONTROL is disabled (=0))
        
        * UXGA(1600x1200), SXGA(1280x1024), XGA(1024x768), SVGA(800x600), VGA(640x480),
          CIF(400x296), QVGA(320x240), HQVGA(240x176), QQVGA(160x120)

    Please refer to https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/ for more settings.

  - Esp32camRtsp() usage examples:
    - Esp32camRtsp() - default rtsp port 554, max. client count 1
    - Esp32camRtsp(8554) - custom rtsp port 8554, max. client count 1
    - Esp32camRtsp(8554,4) - custom rtsp port 8554, max. client count 4
    - Esp32camRtsp(554,1,FRAMESIZE_SXGA, 0,0,1,0,-1) - default rtsp port 554, max. client count 1, frame size = SXGA, brightness = 1, contrast = 0, saturation = -1
    - Esp32camRtsp(554,1,FRAMESIZE_SXGA, 0,0,1,0,-1,0,7) - set AGC_GAIN value to 7 on GAIN_CONTROL disabled

- Use VLC to open stream "rtsp://YOUR_RTSP_SERVER_IP:PORT/mjpeg/1" to test RTSP server function

## Upgrade to External Component for Local Use

- See `EXTERNAL COMPONENT SETUP INSTRUCTIONS.md` for detail.

## References

- Micro-RTSP: <https://github.com/geeksville/Micro-RTSP>
- esp32cam-rtsp: <https://github.com/rzeldent/esp32cam-rtsp>
- 夜巿小霸王 <https://youyouyou.pixnet.net/blog/post/120778494>
- Micro-RTSP代碼解析
  - <https://blog.csdn.net/katerdaisy/article/details/128318785>
  - <https://blog.csdn.net/katerdaisy/article/details/128345987>
  - <https://blog.csdn.net/katerdaisy/article/details/128483624>
- Lewis Van Winkle (2019) Hands-On Network Programming with C.
