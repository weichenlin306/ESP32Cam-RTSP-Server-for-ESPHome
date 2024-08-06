# ESP32CAM RTSP Server三部曲- Part 3 多客戶連線最終章

## 置頂
多客戶端程式esp32cam-rtsp-server.h放在這裡：<https://github.com/weichenlin306/ESP32Cam-RTSP-Server-for-ESPHome>

## 楔子

承上 Part 2 之 Micro-RTSP 改裝，已成功在 ESPHome 上實現了類別實作，可以做單一客戶端的連線。然而，實務上只能連線一個客戶端，似乎不太實用。但若要再修改成多客戶端連線，該怎麼做呢？又，開發板效能不如電腦，經得起多個客戶端連線的負擔嗎？

## 功課

書上說：在 Linux/macOS 系統上，要實現多客戶端連線有兩大類的方法，一是使用 fork()，另一是使用 select()。前者須配合多線程 (multithreading) 設計，但問題很多，有一大籮筐的問題需要處理，作者並不建議。捨棄後只剩下 select() 方法，也沒得選了。秉持「第一胎照書養」的信念，依樣畫葫蘆，第一步得找出 listening socket 的 sockfd。程式中建立連線用的函式庫是 WiFi，server用的是 WiFiServer.h 與 WiFiServer.cpp 這兩個函式定義與程式檔。但是 void begin() 的初始化函式裡的 sockfd 是私用 (private) 變數，無法由外部程式取用，如果要使用這個變數，得修改這個原始函式檔才行！查了 ESPHome 的 libraries 引用方式 (YAML檔中)，有一個方式是要先把 WiFi 函式庫另外開個 GitHub Repo 來儲存，再以 `<name`>=`<source`> 的方式 ([詳見說明](https://esphome.io/components/esphome.html#libraries)) 連結使用。  
很高興地在 <https://github.com/espressif/arduino-esp32> 網站下載了函式庫，並在 WiFiServer.h/.cpp 類別裡加了一個公用函式 int getSockFd()，編譯過程中卻出現 getSockFd() 未定義的錯誤...。發生了什麼事？明明已加上去了，為什麼找不到？百思不得其解。

## 頓悟

找了半天，終於發現所改的程式完全沒被引用入編譯環境中，而且又出現了其他不少的錯誤 (如 EPSmDNS，我根本沒動到這個元件)，編譯器到底是找到哪個版本的WiFi來編譯的？鐵定不是我加工過的這個！又再研究了 PlatformIO 有關 lib_deps 的設定，總算找到了一點蛛絲馬跡。原來 platform-espressif32 還在持續開發中，目前最新版本 (截至2024/7/30) 為 6.7.0，但是在ESPHome的編譯使用的是版本 5.4.0 ( espressif32@5.4.0，如圖)，配合它的 Framework arduino 是 v2.0.6，須至 [樂鑫網站](https://components.espressif.com/components/espressif/arduino-esp32/versions/2.0.6?language=en) 下載這個版本來修改才行。

<img src="https://github.com/user-attachments/assets/6a080e76-1d2e-48fd-984b-577b4c1a6832" width="900">

## 轉念

費了好大工夫，總算修改了 WiFiServer.h 與 WiFiServer.cpp ，加了 int getSockFd() 也如願可以呼叫得到了。然後呢？總覺得書上的多客戶端程式寫作方式與 Arduino 的寫作風格格格不入，有可能不用 select() 方法就能達到多客戶端的連線嗎？再檢視一下相關 .h/.cpp 檔，果然在 WiFiClient.cpp 裡看到了 select() 函式的使用，原來它被巧妙地包裝在 WiFiServer::accept() 裡！既是如此，是否可以只使用 Arduino 的寫作方法，就可以達到多客戶端連線的要求呢？
回過頭再思考一下單客戶端的連線，拆解它的關鍵步驟如下：在 loop() 迴圈裡

1. 測試是否已有連線的 session？若有，則在超過設定的幀間時間時送出一張影像。
2. 若未產生 session，則用 WiFiServer::accept() 看看是否有客戶端請求連線，若無則略過。
3. 若有客戶端連線請求，則建立一條新的 session。
4. Server端持續送出影像，若客戶端過久 (timeout) 等不到影像，會送出 TEARDOWN 命令結束連線，此連線又再回到等待連線狀態。

從整個行為來看，這組連線由無而有、由有而無，自成一個封閉運作空間！我們或可利用這個特性，先建立各組運作空間，再依序循環切換各組連線，就可達到多客戶端連線的目的？

## 實作

很快地利用 vector 設定一個動態陣列，以儲存各組客戶端連線空間，包含各組需用到的物件與變數。先測試一個客戶端，確定沒問題。再把 port 改變，並加大可連客戶端數，至少測試到四個客戶端是沒問題的。  
由於每個影像輸出須在幀間時間 (MSEC_PER_FRAME) 內完成，亦即，每個單一連線的一次完整迴圈所佔用時間為最小耗用時間，在幀間時間裡可完成幾個最小耗用時間，決定了最大連線數。

## 演示

在YAML檔中，lambda function設為

    auto Esp32CamRtspServer = new Esp32camRtsp(8554,4);
    return {Esp32CamRtspServer};

連線網址設為 <font color="yellow">rtsp://<ESP32CAM_RTSP_SERVER_IP>:8554/jmpeg/1</font>，同時開啟四個 VLC 視窗 (影片)，可正常運作。成功！

https://github.com/user-attachments/assets/839536a1-5a1d-4920-89dc-d03e6936c3bc

## 當 ESP32-CAM 遇到 Frigate 與 Home Assistant

ESPHome 由於全世界熱心人士的頁獻，可以讓毫無程式寫作經驗的人在極短時間內就創造出一片獨特的感應器 (sensor)。在此，我們除了可以用 ESP32-CAM 來串流影像外，只要可使用的 pin 腳足夠且 MCU 還夠力，就能再把想加入的感應器元件加進去。  
如 Part 1 所述，我們再把 ATH20+BMP280 加進來使用。因又增加了元件，相對也耗用了 MCU 的資源，可同時連線數必須減少 (建議只留一個連線) (註)。開發板功能雖不及一般電腦，但因價廉，可多量使用，放在屋內多個角落，用以控制智慧裝置。  
Frigate 為近年很熱門的智慧監視系統，截至目前 (2024/8/4) 的最新版本為 0.13.2。RTSP 串流可以不透過它內含的 go2rtc 就可直接使用，設定方便且效能要好許多。Frigate 也是以 docker 容器的形式存在，可以經由 Home Assistant (HA) 透過 HACS 安裝，屬於 HA 系統中的一個 add-on 元件。以下為 frigate.yml 中有關 camera 設定的參考片段:

    garage-esp32-cam:
      ffmpeg:
        inputs:
        - path: rtsp://<YOUR_RTSP_SERVER_IP>/mjpeg/1
          input_args: preset-rtsp-restream
          roles:
          - detect
      detect:
        enabled: true
        width: 1024
        height: 768
        fps: 5
        stationary:
          interval: 50
          threshold: 50
      objects:
        track:
        - car
        - bicycle
        filters:
          car:
            mask:
            - 520,0,0,0,0,768,74,768
          bicycle:
            mask:
            - 404,173,221,494,0,374,0,768,1024,768,1024,0,0,0,0,328,242,61
      snapshots:
        enabled: true
        bounding_box: true
        retain:
          default: 3

<img src="https://github.com/user-attachments/assets/9553df44-9e3a-4a6a-90dc-f20234a58d43" width="400"> <img src="https://github.com/user-attachments/assets/a2538def-922a-40d1-a92b-a7c829e6f2a4" width="400">

在 HA 的主面板裡，可以把 ESP32-CAM 另加的溫溼度壓力元件的輸出數值顯示於此，同時把由串流到 Frigate 再分流出來的影像顯示在此面板中，顯示速度為 0.1 fps。Frigate 還可做許多方面的監測 (事件觸發) 與記錄 (recording / snapshot)，可依需求調整設定。

##### (註) 若遇到 OV2640 鏡頭品質不優、排線接頭容易發熱時，可嘗試修改 esp32cam-rtsp-server.h 如下：
1. <font color="yellow">#define MSEC_PER_FRAME 200</font>
2. 設定 <font color="yellow">config.xclk_freq_hz = 10000000;</font>

## 後記

《ESP32CAM RTSP Server三部曲》可算是我個人的學習筆記，在摸索的過程中確實獲益良多。希望在此藉由我跌跌撞撞得到的經驗，分享予諸位剛入坑的新手們，可以讓大家少走許多冤枉路，是我最大的期望！
