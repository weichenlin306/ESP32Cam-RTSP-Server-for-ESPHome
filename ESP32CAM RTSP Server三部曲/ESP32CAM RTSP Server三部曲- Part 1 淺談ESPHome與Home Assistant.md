# ESP32CAM RTSP Server三部曲- Part 1 淺談ESPHome與Home Assistant

## 楔子

現今的智慧裝置如雨後春筍般蓬勃發展，也為現在進行式的AI產業進入家庭成為現實而非只是空談。然而，控制智慧裝置的App亦是百家爭鳴，各彈各的調，不同廠商間常無法相互控制，為使用上的一大缺憾！隨著開放源代碼的自由軟體推波助瀾，一群個中高手創建了欲一統天下的Home Assistant(HA,家庭助理)系統，將各家裝置控制整合到這個系統之下，用一致的控制方式控制各家裝置，也可加入自動化的控制流程，讓我們可以提早體驗機器人時代的方便性，是件很令人感到愜意的事！但，坐而言不如起而行...

在個人的使用經驗中，在某些感知器狀態下去驅使另一個裝置做特定動作已是尋常不過的事，但若要偵測某個場地有人、或有幾個人，可能不是一般紅外線或雷達偵測器可以精準做到的，這時，攝影機便可派上用場了！巿面上攝影機琳琅滿目，要找一個不是問題，但單價都在近千元或上千元台幣，而我需要的並不需要解析度很高、畫質很好的機種，只要能分辨得出是一個人、一隻狗、一輛機車、或一輛汽車即可。用此目標尋找適合對象，以我目前的認知，則非ESP32-CAM(或相容開發板)莫屬了！

## 簡介

如前所述，要利用偵測到的影像來控制裝置，使用HA是再適合不過了。關於HA，在此( <https://www.home-assistant.io/> )有詳細介紹，不再贅述，僅就初學者較不易弄清的觀念略為介紹。  
關於HA的硬體，可以裝在他們主推的樹莓派(Raspberry Pi)(最容易)，或裝到PC實體機或各種虛擬機上(包括Proxmox, Windows, macOS, Linux等)(專家級)。  
關於HA的軟體，可以安裝HAOS(較陽春的Linux)或裝在虛擬機上成為一個客體作業系統(能達到功能各有不同)。HA有一些功能元件是內含(built-in)的；HA未內含但可以是「選配」功能的，則由各熱心的高手群製作成一個Docker容器(container)，統一由內含的Supervisor管理。  
這裡要介紹的ESPHome即是一個Docker container，也是ESP32系列開發板可以在HA有一席之地的開發環境。它的編譯環境主要是由PlatformIO衍生而來。所使用的編譯架構(Framework)有二：一為我們熟悉的Arduino，另一是ESP32母公司樂鑫公司所提供的原生程式庫ESP-IDF，可以讓ESP32能力發揮得更淋漓盡致。

## 選擇

牽涉到影像處理，所需MCU處理能力對於sensor(server端)或HA主機(client端)都是個挑戰。前者手邊只能選ESP32-CAM，後者則有多樣選擇。因要辨識影像內物體，會採用Frigate container (AI辨識核心可能是YOLO7)。據開發者建議，勿把Frigate建構在虛擬機上的容器裡，否則執行影像處理效能較差。因此，最後直接把HAOS裝在PC (i7-xxxx)的機器上，跑起來順很多。又，ESPHome時常會更新，每更新一次，所有的ESPHome裝置程式都會重新編譯一次再經由OTA (Over The Air)上傳，有台效能高的電腦主機可節省不少時間。  
關於影像串流，一般監視器會提供HTTP與RTSP通訊格式，Frigate則以RTSP為主，不同的格式會透過同在Frigate docker容器內的go2rtc介面來轉換(仍會耗用不小CPU資源)。因此，如果ESP32-CAM能提供原生的RTSP串流就再好不過了。可惜的是，現有ESPHome元件(component)只提供HTTP，想要RTSP，只好自己造輪子。

## 探奧

使用ESPHome來開發開發板的好處，是可以很容易地把想拼湊在一起的元件像積木般加在同一開發板上，且不用寫任何程式，只要把需要的元件與設定寫在個別的YAML檔即可，之後的程式編譯與安裝都是自動或半自動的。  
例如，我開啟了一個以"test"為名的裝置，ESPHome就自動產生了以下的YAML檔：

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

第一次Install時要先透過USB上傳編譯完成的程式碼，以後就可使用OTA方式經由WiFi連線傳輸程式碼。想加什麼元件，只要有人把它生出來，都可以在這裡( <https://www.esphome.io/> )找得到。重要的是，這個地方是全球熱心人士無私提供的，您也可以是其中的一位貢獻者(contributor)。
我以手邊現有的溫溼度壓力感知器AHT20+BMP280這個單一元件為例說明(可參閱 <https://esphome.io/components/sensor/aht10> 與 <https://esphome.io/components/sensor/bmp280> )，在上述YAML檔文末加入

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

該元件使用i2c介面，亦須另外加入i2c元件。裡面使用lambda function以校正實際數值。編譯上傳(USB或OTA)後，它就可以運作了，也可再加入HA面板顯示。然而，如果沒有現成元件可以使用，該怎麼做呢？ESPHome也提供了做法( <https://www.esphome.io/components/sensor/custom.html> )。我們較好奇的是：所有使用到的元件是如何「兜」在一個程式裡的呢？ESPHome是如何把YAML翻譯成c++程式？又如何讓各元件間可協同運作的？

## 直搗黃龍

既知我安裝的ESPHome是個Docker container版本，就可在它的終端機模式(Terminal)下使用相關命令一探究竟。  
終端機模式可藉安裝Add-On "Advanced SSH & Web Terminal"(圖A)獲得，或使用PuTTY連入。此處以前者演示。  
先下命令"docker ps"，會列出已安裝的容器，找出esphome的容器識別碼(不是固定不變的)，是"b58141e0601a"。  
再下命令：docker exec -it b58141e0601a bash  
即可進入ESPHome的Linux環境(圖B)，可用"find -name xxxx"尋找xxxx檔案存放的位置。在/data/build/test/src目錄下，找到了主檔main.cpp(圖C)。由YAML翻譯成c++的主檔名都固定為main.cpp。  
列出main.cpp內容(圖D)，可見其中每個元件(包括匯流排驅動程式)都自成一個類別(class, HA中稱為platform)，同一類別(如sensor)下各裝置(class->instance)再以id_1, id_2,...區分，並各以App.register_component()讓HA使用。在類別定義裡，須有setup()與loop()兩個public常式(override)供App主程式呼叫。有興趣的話，可以注意一下lambda function是如何被翻譯成c++程式的一部分的。  
原本由ESPHome提供的元件我們可以當黑盒子使用，不用知道它是如何產生的；但如果要自己從零開始打造一個新元件，就得知道它必須具備怎樣的架構。  
簡單地說，它必須宣告成一個類別(class)，再在YAML檔裡透過lambda function將該類別實物化，變成HA實際可用的「實例」(instance)，並賦予它在HA中的對應ID。

## 目標

將Micro-RTSP改造成ESPHome適用的元件！  
當然，如能把它變成ESPHome的一個元件，上傳供大家取用，是再好不過的了，各位愛好者加油吧！您也可以是ESPHome全球大家族裡的一個contributor!
