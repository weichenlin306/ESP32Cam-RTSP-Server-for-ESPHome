# ESP32Cam-RTSP-Server-for-ESPHome
 ESP32Cam RTSP Server for ESPHome

## Based on
- Micro-RTSP: https://github.com/geeksville/Micro-RTSP
- esp32cam-rtsp: https://github.com/rzeldent/esp32cam-rtsp

## Modifications
- WebServer did not pass compiling on ESPHome and hence the HTTP protocol is totally removed
- Include only those header files being used for camera image capture and pure RTSP connection
- Encapulate Micro-RTSP as a class for use in ESPHome

## Usage
- Place "esp32cam-rtsp-server-single-client.h" file and "micro_rtsp" folder in the same folder as your ESPHome project (.yaml) file
- Configure your YAML project file
    - Add "psram" component to use pseudo SRAM (PSRAM) to avoid "core panic'd" and infinite reboot
    - Put a custom component named "custom_component" and make an Esp32camRtsp() instance using lambda function
    - Use class instancization of Esp32camRtsp() without any parameter for default rtsp port 554 and with a number for a specific rtsp port
- Use VLC to open stream "rtsp://YOUR_RTSP_SERVER_IP/mjpeg/1" to test this RTSP server function

## References
- Micro-RTSP: https://github.com/geeksville/Micro-RTSP
- esp32cam-rtsp: https://github.com/rzeldent/esp32cam-rtsp
- https://youyouyou.pixnet.net/blog/post/120778494
- https://blog.csdn.net/katerdaisy/article/details/128318785
- https://blog.csdn.net/katerdaisy/article/details/128345987
- https://blog.csdn.net/katerdaisy/article/details/128483624
- Lewis Van Winkle (2019) Hands-On Network Programming with C.