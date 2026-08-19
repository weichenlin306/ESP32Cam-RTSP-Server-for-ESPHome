# ESP32CAM RTSP Server三部曲- Part 2 Micro-RTSP為ESPHome改裝

###### <p align=right>更新日期: 2024/8/7</p>

## 置頂

改裝後的 Micro-RTSP 程式 esp32cam-rtsp-server-single-client.h 放在這裡：  
<https://github.com/weichenlin306/ESP32Cam-RTSP-Server-for-ESPHome>

## 楔子

在寫稿的此時此刻，在 ESPHome 已發表的正式元件中，ESP32CAM 只有以 HTTP 通訊為基礎的 WebServer 版，而我想使用的運行於Home Assistant (HA) 具有強大功能的監視系統 [Frigate](https://github.com/blakeblackshear/frigate) 可直接使用 RTSP 串流不必耗用 CPU 再轉格式 (若我認知有誤，請不吝指正)。是以，將 Micro-RTSP 改裝成 ESPHome 可用的元件是個不錯的目標。

## 挫折

先前已成功讓 Micro-RTSP 在 Arduino IDE 上編譯成功，也上傳 ESP32-CAM 執行無誤，理論上把它改裝成 class 再在 ESPHome 重新編譯應該不是問題才對？但是，編譯過程中硬是出現許多 MACRO 重複定義錯誤，不然就是找不到標頭檔 (.h) 的錯誤。仔細尋找根源，是 WebServer 相關函式庫無法相容，也注定了原來與 HTTP 相關的程式部分無法使用。反正我只要純血 RTSP 就好，就忍痛割捨 HTTP 部分...全部拿掉！  
據我所知，ESPHome 有關 Framework Arduino 的部分是使用 [arduino-esp32 函式庫](https://github.com/espressif/arduino-esp32)，裡面雖有 libraries/WebServer，但在 ESPHome 使用時卻會出問題，似乎是仍有 bug 無法成功登錄在 PlatformIO Registry  裡 (註)，所以無法使用，只好期待未來有 Socket Programming 高手可以把它改成適用的函式庫了。

###### (註) 可能是我認知有誤，須待釐清

## 孵蛋

計劃已定，開始著手改造 Micro-RTSP :  
依 ESPHome 新裝置設定方法，先建立一個新的裝置 (見Part 1)。新裝置的描述檔使用YAML格式，我們的新程式標頭檔命名為 esp32cam-rtsp-server-single-client.h，要在 YAML 檔裡用 Include 連結。拿掉與 HTTP 相關的程式段，不只可瘦身程式，執行效能也會提高。  
把一大段程式封裝成一個類別，命名為 class Esp32camRtsp，並繼承 ESPHome 的 Component 元件。Aduino 程式中的兩個主要常式 setup() 與 loop() 要加上關鍵字 override 並放在 public: 段內。變數初值化依 class 方式為之，並優化各變數類型與執行效能。  
在 ESPHome 新裝置 YAML 檔內 [依此說明](https://esphome.io/custom/custom_component.html) 建立一個客製化的元件 custom_component:。  
YAML檔內也要加入psram元件，否則可能造成執行期間不斷重開機的無窮迴圈。

## 破殼

準備就緒後，按下 Install 會有四種方式編譯程式與上傳，可以選擇您適用的方法。完成後，就可以使用 VLC 做連線測試。  
我在 github 上也放了一個 example-rtsp-server.yaml 檔，可以作為設定參考。如果您的設定無誤，也編譯上傳執行了，要如何除錯呢？  
爬了許多文，最後終於頓悟：「衆裏尋他千百度。驀然回首，那人卻在，燈火闌珊處。」原來最好用的還是 Arduino IDE 裡內建的「序列埠監控窗」！在此監控窗裡，若在 VLC 連線時可以看到 "RTSP received OPTIONS", "... DESCRIBE", "... SETUP", "... PLAY", "... TEARDOWN" 等字眼之一，那就表示成功了！  
如要在程式中除錯，顯示訊息，可以使用下列任一種方式：

    printf("%s, %d", "String", 12345);
    Serial.print("String");
    ESP_LOGI("TAG String", "%s, %d", "String", 12345);

## 演示

配合 Arduino IDE 的「序列埠監控窗」，以 VLC 播放實測 RTSP 動作。影片中的紫色小方塊是 Part 1 裡提及的 AHT20+BMP280。ESP32-CAM 上所接為160度廣角鏡，可以在室內涵蓋較大的偵測區域。

https://github.com/user-attachments/assets/1ebd8fd2-a66c-4b38-97f8-3a7a210a94bc

## 未竟

目前的這個 RTSP Server 只能讓一個客戶端 (client) 連線，確實有些缺憾。~~只是力有未逮，目前尚在啃書試圖補上最後一塊拼圖... 只能說「革命尚未成功，同志仍須努力！」~~  
原以為還要花很長的時間才能克服多客戶同時連線的問題，沒想到靈光乍現，短短兩天時間即測試成功，確實運氣不錯。或許還有更好的寫法，可待日後再突破。

## 參考資料

1. Micro-RTSP: <https://github.com/geeksville/Micro-RTSP>
2. esp32cam-rtsp: <https://github.com/rzeldent/esp32cam-rtsp>
3. 夜巿小霸王 <https://youyouyou.pixnet.net/blog/post/120778494>
4. Micro-RTSP 程式碼解析1-3  
    <https://blog.csdn.net/katerdaisy/article/details/128318785>  
    <https://blog.csdn.net/katerdaisy/article/details/128345987>  
    <https://blog.csdn.net/katerdaisy/article/details/128483624>
5. Lewis Van Winkle (2019) Hands-On Network Programming with C.

## 後記

原以為 RTSP 的資料傳輸不好懂，仔細研究之後倒不是那麼困難，倒是較細節的實際 TCP/UDP 傳輸較需要耗費精神去理解。Socket Programming 是此程式改寫的瓶頸，尤其是要改寫成同時多客戶端連線的模式。  
上面參考資料中的 Hands-On Network Programming with C 網路程式設計專書，提綱挈領、淺顯易懂、貼合實務，是本值得推薦的好書。
