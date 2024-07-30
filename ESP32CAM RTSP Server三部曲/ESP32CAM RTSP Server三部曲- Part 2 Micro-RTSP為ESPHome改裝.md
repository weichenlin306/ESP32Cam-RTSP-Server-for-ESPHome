# 【分享】ESP32CAM RTSP Server三部曲- Part 2 Micro-RTSP為ESPHome改裝

## 置頂

改裝後的Micro-RTSP放在這裡：<https://github.com/weichenlin306/ESP32Cam-RTSP-Server-for-ESPHome>

## 楔子

在寫稿的此時此刻，在ESPHome已發表的正式元件中，ESP32CAM只有以HTTP通訊為基礎的WebServer版，而我想使用的運行於Home Assistant (HA)具有強大功能的監視系統Frigate ( <https://github.com/blakeblackshear/frigate> )可直接使用RTSP串流不必耗用CPU再轉格式(若我認知有誤，請不吝指正)。是以，將Micro-RTSP改裝成ESPHome可用的元件是個不錯的目標。

## 見鬼

先前已成功讓Micro-RTSP在Arduino IDE上編譯成功，也上傳ESP32-CAM執行無誤，理論上把它改裝成class再在ESPHome重新編譯應該不是問題才對？但是，編譯過程中硬是出現許多MACRO重複定義錯誤，不然就是找不到標頭檔(.h)的錯誤。仔細尋找根源，是WebServer相關函式庫無法相容，也注定了原來與HTTP相關的程式部分無法使用。反正我只要純血RTSP就好，就忍痛割捨HTTP部分...全部拿掉！  
據我所知，ESPHome有關Framework Arduino的部分是使用arduino-esp32函式庫( <https://github.com/espressif/arduino-esp32> )，裡面雖有libraries/WebServer，但在ESPHome使用時卻會出問題，似乎是仍有bug無法成功登錄在PlatformIO Registry裡，所以無法使用，只好期待未來有Socket Programming高手可以把它改成適用的函式庫了。

## 孵蛋

計劃已定，開始著手改造Micro-RTSP:  
依ESPHome新裝置設定方法，先建立一個新的裝置(見Part 1)。新裝置的描述檔使用YAML格式，我們的新程式標頭檔命名為 esp32cam-rtsp-server-single-client.h，要在YAML檔裡用Include連結。拿掉與HTTP相關的程式段，不只可瘦身程式，執行效能也會提高。  
把一大段程式封裝成一個類別，命名為class Esp32camRtsp，並繼承ESPHome的Component元件。Aduino程式中的兩個主要常式setup()與loop()要加上關鍵字override並放在public:段內。  
變數初值化依class方式為之。優化各變數類型與執行效能。  
在ESPHome新裝置YAML檔內建立一個客製化的元件custom_component: (請參閱 <https://esphome.io/custom/custom_component.html> )。  
YAML檔內也要加入psram元件，否則可能造成執行期間不斷重開機的無窮迴圈。

## 破殼

準備就緒後，按下Install會有四種方式編譯程式與上傳，可以選擇您適用的方法。完成後，就可以使用VLC做連線測試。  
我在github上也放了一個example-rtsp-server.yaml檔，可以做設定參考。如果您的設定無誤，也編譯上傳執行了，要如何除錯呢？  
爬了許多文，最後終於頓悟：「衆裏尋他千百度。驀然回首，那人卻在，燈火闌珊處。」原來最好用的還是Arduino IDE裡的「序列埠監控窗」！在此監控窗裡，若在VLC連線時可以看到"RTSP received OPTIONS", "...DESCRIBE", "...SETUP", "...PLAY", "...TEARDOWN"等字眼之一，那就表示成功了！  
如要在程式中除錯，顯示訊息，可以使用下列任一種方式：

    printf("%s, %d", "String", 12345);
    Serial.print("String");
    ESP_LOGI("TAG String", "%s, %d", "String", 12345);

## 演示

配合Arduino IDE的「序列埠監控窗」，以VLC播放實測RTSP動作。影片中的紫色小方塊是Part 1裡提及的AHT20+BMP280。ESP32-CAM上所接為160度廣角鏡，可以在室內涵蓋較大的偵測區域。

## 未竟

目前的這個RTSP Server只能讓一個客戶端(client)連線，確實有些缺憾，只是力有未逮，目前尚在啃書試圖補上最後一塊拼圖...只能說「革命尚未成功，同志仍須努力！」

## 參考資料

1. Micro-RTSP: <https://github.com/geeksville/Micro-RTSP>
2. esp32cam-rtsp: <https://github.com/rzeldent/esp32cam-rtsp>
3. 夜巿小霸王 <https://youyouyou.pixnet.net/blog/post/120778494>
4. Micro-RTSP程式碼解析1-3  
    <https://blog.csdn.net/katerdaisy/article/details/128318785>  
    <https://blog.csdn.net/katerdaisy/article/details/128345987>  
    <https://blog.csdn.net/katerdaisy/article/details/128483624>
5. Lewis Van Winkle (2019) Hands-On Network Programming with C.

## 後記

原以為RTSP的資料傳輸不好懂，仔細研究之後倒不是那麼困難，倒是較細節的實際TCP/UDP傳輸較需要耗費精神去理解。Socket Programming是此程式改寫的瓶頸，尤其是要改寫成同時多客戶端連線的模式。  
上面參考資料中的Hands-On Network Programming with C確實是本不錯且務實的好書，淺顯易懂，是本值得推薦的好書。
