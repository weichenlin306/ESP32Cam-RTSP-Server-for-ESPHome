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
  1. <font color="skyblue">#define MSEC_PER_FRAME 200</font>
  2. <font color="skyblue">config.xclk_freq_hz = 10000000;</font>

## Usage

- Place "esp32cam-rtsp-server.h" file and "micro_rtsp" folder in the same folder as your ESPHome project (.yaml) file
- Configure your YAML project file

  - Add "psram" component to use pseudo SRAM (PSRAM) to avoid "core panic'd" and infinite reboot
  - Put a custom component named "custom_component" and make an Esp32camRtsp() instance using lambda function
  - Syntax

        auto Esp32CamRtspServer = new Esp32camRtsp(
            [RTSP_PORT], [MAX_CLIENT_COUNT],
            [VERTICAL_FLIP], [HORRIZONTAL_MIRROR], [BRIGHTNESS], [SATURATION]
        )

        Value Ranges:
          VERTICAL_FLIP: 0 | 1
          HORRIZONTAL_MIRROR: 0 | 1
          BRIGHTNESS: -2 ~ 2 (integer)
          SATURATION: -2 ~ 2 (integer)


  - Esp32camRtsp() usage examples:
    - <font color="yellow">Esp32camRtsp()</font> - default rtsp port 554, max. client count 1
    - <font color="yellow">Esp32camRtsp(8554)</font> - custom rtsp port 8554, max. client count 1
    - <font color="yellow">Esp32camRtsp(8554,4)</font> - custom rtsp port 8554, max. client count 4
    - <font color="yellow">Esp32camRtsp(554,1,0,0,1,-1)</font> - default rtsp port 554, max. client count 1, brightness = 1, saturation = -1

- Use VLC to open stream <font color="lime">rtsp://YOUR_RTSP_SERVER_IP:PORT/mjpeg/1</font> to test RTSP server function

## References

- Micro-RTSP: <https://github.com/geeksville/Micro-RTSP>
- esp32cam-rtsp: <https://github.com/rzeldent/esp32cam-rtsp>
- 夜巿小霸王 <https://youyouyou.pixnet.net/blog/post/120778494>
- Micro-RTSP代碼解析
  - <https://blog.csdn.net/katerdaisy/article/details/128318785>
  - <https://blog.csdn.net/katerdaisy/article/details/128345987>
  - <https://blog.csdn.net/katerdaisy/article/details/128483624>
- Lewis Van Winkle (2019) Hands-On Network Programming with C.
