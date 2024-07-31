# ESP32CAM RTSP Server三部曲- Part 1 淺談ESPHome與Home Assistant

## 楔子

現今的智慧裝置如雨後春筍般蓬勃發展，也為現在進行式的AI產業進入家庭成為現實而非只是空談。然而，控制智慧裝置的 App 亦是百家爭鳴，各彈各的調，不同廠商間常無法相互控制，為使用上的一大缺憾！隨著開放源代碼的自由軟體推波助瀾，一群個中高手創建了欲一統天下的 Home Assistant (HA, 家庭助理) 系統，將各家裝置控制整合到這個系統之下，用一致的控制方式控制各家裝置，也可加入自動化的控制流程，讓我們可以提早體驗機器人時代的方便性，是件很令人感到愜意的事！但，坐而言不如起而行...  
在個人的使用經驗中，在某些感知器狀態下去驅使另一個裝置做特定動作已是尋常不過的事，但若要偵測某個場地有人、或有幾個人，可能不是一般紅外線或雷達偵測器可以精準做到的，這時，攝影機便可派上用場了！巿面上攝影機琳琅滿目，要找一個不是問題，但單價都在近千元或上千元台幣，而我需要的並不需要解析度很高、畫質很好的機種，只要能分辨得出是一個人、一隻狗、一輛機車、或一輛汽車即可。用此目標尋找適合對象，以我目前的認知，則非 ESP32-CAM (或相容開發板) 莫屬了！

## 簡介

如前所述，要利用偵測到的影像來控制裝置，使用 HA 是再適合不過了。關於 HA，[在此](https://www.home-assistant.io/)有詳細介紹，不再贅述，僅就初學者較不易弄清的觀念略為介紹。  
關於HA的硬體，可以裝在他們主推的樹莓派 (Raspberry Pi) (最容易)，或裝到PC實體機或各種虛擬機上 (包括 Proxmox, Windows, macOS, Linux 等) (專家級)。  
關於 HA 的軟體，可以安裝 HAOS (較陽春的 Linux ) 或裝在虛擬機上成為一個客體作業系統(能達到功能各有不同)。HA有一些功能元件是內含(built-in)的；HA 未內含但可以是「選配」功能的，則由各熱心的高手群製作成一個 Docker 容器 (container)，統一由內含的Supervisor管理。  
這裡要介紹的 ESPHome 即是一個 Docker container，也是ESP32系列開發板可以在HA有一席之地的開發環境。它的編譯環境主要是由 PlatformIO 衍生而來。所使用的編譯架構 (Framework) 有二：一為我們熟悉的 Arduino，另一是 ESP32 母公司樂鑫公司所提供的原生程式庫 ESP-IDF，可以讓 ESP32 能力發揮得更淋漓盡致。

## 選擇

牽涉到影像處理，所需 MCU 處理能力對於 sensor (server端) 或 HA 主機 (client端) 都是個挑戰。前者手邊只能選 ESP32-CAM，後者則有多樣選擇。因要辨識影像內物體，會採用 Frigate container (AI 辨識核心可能是 YOLO7 )。據開發者建議，勿把 Frigate 建構在虛擬機上的容器裡，否則執行影像處理效能較差。因此，最後直接把 HAOS 裝在 PC (i7-xxxx) 的機器上，跑起來順很多。又，ESPHome 時常會更新，每更新一次，所有的 ESPHome 裝置程式都會重新編譯一次再經由 OTA (Over The Air) 上傳，有台效能高的電腦主機可節省不少時間。  
關於影像串流，一般監視器會提供 HTTP 與 RTSP 通訊格式，Frigate 則以 RTSP 為主，不同的格式會透過同在 Frigate docker 容器內的 go2rtc 介面來轉換 (仍會耗用不小 CPU 資源)。因此，如果 ESP32-CAM 能提供原生的 RTSP 串流就再好不過了。可惜的是，現有 ESPHome 元件 (component) 只提供 HTTP，想要 RTSP，只好自己造輪子。

## 探奧

使用 ESPHome 來開發開發板的好處，是可以很容易地把想拼湊在一起的元件像積木般加在同一開發板上，且不用寫任何程式，只要把需要的元件與設定寫在個別的 YAML 檔即可，之後的程式編譯與安裝都是自動或半自動的。  
例如，我開啟了一個以 "test" 為名的裝置，ESPHome 就自動產生了以下的 YAML 檔：

    /*** test.yaml ***
    esphome:
      name: test
      friendly_name: test
    esp32:
      board: esp32dev
      framework:
        type: arduino
    # Enable logging
    logger:
    # Enable Home Assistant API
    api:
      encryption:
        key: "hiKczJr7bQgxrPcY2+4cKOeLkYfnNMz2X6ZE09w79t8="
    ota:
      - platform: esphome
        password: "f8ab873e4377960b34ee2bffcfc6ad90"
    wifi:
      ssid: !secret wifi_ssid
      password: !secret wifi_password
      # Enable fallback hotspot (captive portal) in case wifi connection fails
      ap:
        ssid: "Test Fallback Hotspot"
        password: "cvZ397qiDTWa"
    captive_portal:

第一次 Install 時要先透過 USB 上傳編譯完成的程式碼，以後就可使用 OTA 方式經由 WiFi 連線傳輸程式碼。想加什麼元件，只要有人把它生出來，都可以在[這裡](https://www.esphome.io/)找得到。重要的是，這個地方是全球熱心人士無私提供的，您也可以是其中的一位貢獻者 (contributor)。  
我以手邊現有的溫溼度壓力感知器 AHT20+BMP280 這個單一元件為例說明 (可參閱 [ATH10/ATH20](https://esphome.io/components/sensor/aht10) 與 [BMP280](https://esphome.io/components/sensor/bmp280) 元件說明)，在上述 YAML 檔文末加入

    i2c:
      sda: GPIO14
      scl: GPIO15
      scan: True
    sensor:
      - platform: aht10
        variant: AHT20
        address: 0x38
        temperature:
          filters:
          - lambda: return x - 8.10;
          name: "Temperature"
        humidity:
          filters:
          - lambda: return x + 14.23;
          name: "Humidity"
        update_interval: 60s
      - platform: bmp280
        address: 0x77
        pressure:
          name: "Pressure"
        update_interval: 60s

該元件使用i2c介面，亦須另外加入 i2c 元件。裡面使用 lambda function 以校正實際數值。編譯上傳 (USB 或 OTA) 後，它就可以運作了，也可再加入HA面板顯示。然而，如果沒有現成元件可以使用，該怎麼做呢？ESPHome [在此](https://www.esphome.io/components/sensor/custom.html)也提供了做法。我們較好奇的是：所有使用到的元件是如何「兜」在一個程式裡的呢？ESPHome是如何把 YAML 翻譯成 C++ 程式？又如何讓各元件間可協同運作的？

## 直搗黃龍

既知我安裝的 ESPHome 是個 Docker container 版本，就可在它的終端機模式 (Terminal) 下使用相關命令一探究竟。  
終端機模式可藉安裝 Add-On "Advanced SSH & Web Terminal" (圖A) 獲得，或使用 PuTTY 連入。此處以前者演示。  
先下命令 "docker ps"，會列出已安裝的容器，找出esphome的容器識別碼 (不是固定不變的)，是 "b58141e0601a"。  
再下命令：**docker exec -it b58141e0601a bash**  
即可進入 ESPHome 的 Linux 環境 (圖B)，可用 "**find -name xxxx**" 尋找 xxxx 檔案存放的位置。在 **/data/build/test/src** 目錄下，找到了主檔 **main.cpp** (圖C)。由 YAML 翻譯成 C++ 的主檔名都固定為 main.cpp。  
列出 main.cpp 內容 (圖D)，可見其中每個元件 (包括匯流排驅動程式) 都自成一個類別 (class, HA 中稱為**platform**)，同一類別 (如sensor) 下各裝置 (class->instance) 再以 id_1, id_2,...區分，並各以 App.register_component() 讓 HA 使用。在類別定義裡，須有 **setup()** 與 **loop()** 兩個 public 常式 (override) 供 App 主程式呼叫。有興趣的話，可以注意一下 lambda function 是如何被翻譯成 C++ 程式的一部分的。  
原本由 ESPHome 提供的元件我們可以當黑盒子使用，不用知道它是如何產生的；但如果要自己從零開始打造一個新元件，就得知道它必須具備怎樣的架構。  
簡單地說，它必須宣告成一個類別 (class)，再在 YAML 檔裡透過 lambda function 將該類別實物化，變成 HA 實際可用的「實例」(instance)，並賦予它在 HA 中的對應 ID。

<img src="https://github.com/user-attachments/assets/82b85f49-f153-4f83-8b5b-a05ea935eb66" width="550">
<img src="https://github.com/user-attachments/assets/89fca6f2-ed16-404d-81e4-ef732b86a61a" width="450"><br>
<img src="https://github.com/user-attachments/assets/926c1e9a-3726-4b1f-8041-68a790182294" width="400">
<img src="https://github.com/user-attachments/assets/c032410c-70a6-4497-8760-58e7f97d50db" width="600"><br>
<img src="https://github.com/user-attachments/assets/adcda71d-076e-45f0-b51b-15c9240c15de" width="450">
<img src="https://github.com/user-attachments/assets/7b369de2-044e-48f6-aaa8-bcd1c5f0c9a9" width="550"><br>
<img src="https://github.com/user-attachments/assets/a5db09ad-e57e-4b4d-bd2e-dd540055f390" width="500">
<img src="https://github.com/user-attachments/assets/4d5f5907-643a-444f-8026-427fb15b7840" width="500"><br>
<img src="https://github.com/user-attachments/assets/1f2c03f3-efd2-4460-8ad5-fc7013db6fc4" width="500">
<img src="https://github.com/user-attachments/assets/2a23d434-dea9-486b-a411-4ed0ae702000" width="500"><br>

## 目標

將 Micro-RTSP 改造成 ESPHome 適用的元件！  
當然，如能把它變成 ESPHome 的一個元件，上傳供大家取用，是再好不過的了，各位愛好者加油吧！您也可以是ESPHome全球大家族裡的一個 contributor!
