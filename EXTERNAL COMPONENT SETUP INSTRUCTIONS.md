# External Component Transforming

Component name: `esp32cam_rtsp_server`

## Component Files

- \_\_init_\_\.py
- esp32cam_rtsp_server.cpp
- esp32cam_rtsp_server.h

## Include Files

- micro_rtsp (folder)

## Complete Example YAML file

- example_external_component_setup.yaml

## Prerequisites and Prework

- Copy "micro_rtsp" folder to config/esphome
- Create a new folder, say "components", under folder "config/esphome"
- Organize folder/file structure as follows:

```
esphome
├── example_external_component_setup.yaml
├── micro_rtsp
├── components
│   ├── esp32cam_rtsp_server
│   │   ├── __init__.py
│   │   ├── esp32cam_rtsp_server.cpp
│   │   └── esp32cam_rtsp_server.h
```

## Using the Include and Components in YAML

- Make sure to place the `micro_rtsp` folder in the include section.

    ```
    esphome:
      name: ...
      friendly_name: ...
      includes: micro_rtsp
    ```
- Make sure to use component `psram` and use the recommended settings as follows.

    ```
    psram:
      mode: octal
      speed: 80MHz
    ```
- Use component `esp32cam_rtsp_server` with settings.

    ```
    esp32cam_rtsp_server:
      resolution: UXGA
      vertical_flip: false
      horizontal_mirror: false
      .....
    ```
- Please see `example-rtsp-server.yaml` for an overview.

## Configuration Variables in YAML

- **camera** (Optional, string): The configuration name of the camera board.
    - Possible values: esp32cam, esp32cam__aithinker, esp32cam_ttgo_t.
- **external_clock_frequency** (Optional, int): Must be between 10000000 and 20000000. Default to `16000000`.
- **port** (Optional, int): RTSP port assignment. Default to `554`.
- **resolution** (Required, string): Display resolution assignment. Possible values:

    - #### 2MP Sensors
        | Value | Resolution |
        |-------|------------|
        | "96X96" | 96x96 |
        | "QQVGA" | 160x120 |
        | "128X128" | 128x128 |
        | "QCIF" | 176x144 |
        | "HQVGA" | 240x176 |
        | "240X240" | 240x240 |
        | "QVGA" | 320x240 |
        | "320X320" | 320x320 |
        | "CIF" | 400x296 |
        | "HVGA" | 480x320 |
        | "VGA" | 640x480 |
        | "SVGA" | 800x600 |
        | "XGA" | 1024x768 |
        | "HD" | 1280x720 |
        | "SXGA" | 1280x1024 |
        | "UXGA" | 1600x1200 |

    - #### 3MP Sensors

        | Value | Resolution |
        |-------|------------|
        | "FHD" | 1920x1080 |
        | "P_HD" |  720x1280 |
        | "P_3MP" |  864x1536 |
        | "QXGA" | 2048x1536 |

    - #### 5MP Sensors

        | Value | Resolution |
        |-------|------------|
        | "QHD" | 2560x1440 |
        | "FWQXGA" | 2560x1600 |
        | "P_FHD" | 1080x1920 |
        | "QSXGA" | 2560x1920 |
        | "5MP" | 2592x1944 |

- **brightness** (Optional, int): The brightness to apply to the picture, from -2 to 2. Defaults to `0`.
- **contrast** (Optional, int): The contrast to apply to the picture, from -2 to 2. Defaults to `0`.
- **saturation** (Optional, int): The saturation to apply to the picture, from -2 to 2. Defaults to `0`.
- **special_effect** (Optional, enum): The effect to apply to the picture. Defaults to `none`.
    - none
    - negative
    - grayscale
    - red_tint
    - green_tint
    - blue_tint
    - sepia
- **white_balance** (Optional, boolean): The white balance setting. Defaults to `True`.
- **awb_gain** (Optional, boolean): The auto white balance gain setting. Defaults to `True`.
- **exposure_control** (Optional, boolean): The exposure control setting. Defaults to `True`.
- **aec2** (Optional, boolean): The auto exposure control 2 setting. Defaults to `True`.
- **ae_level** (Optional, boolean): The auto exposure level setting, from -2 to 2. Defaults to `0`.
- **aec_value** (Optional, boolean): The auto exposure control value setting, from 0 to 1200. Defaults to `300`.
- **gain_control** (Optional, boolean): The gain control setting. Defaults to `False`.
- **agc_gain** (Optional, int): The agc gain setting, from 0 to 30. Defaults to `6`.
- **gain_ceiling** (OPTIONAL, enum): The maximum gain allowed. Defaults to `2x`.
    - 2x
    - 4x
    - 8x
    - 16x
    - 32x
    - 64x
    - 128x
- **bpc** (Optional, boolean): The bpc setting. Defaults to `False`.
- **wpc** (Optional, boolean): The wpc setting. Defaults to `True`.
- **raw_gma** (Optional, boolean): The raw gma setting. Defaults to `True`.
- **horizontal_mirror** (Optional, boolean): Whether to mirror the image horizontally. Defaults to `False`.
- **vertical_flip** (Optional, boolean): Whether to flip the image vertically. Defaults to `False`.
- **lenc** (Optional, boolean): The bpc setting. Defaults to `True`.
- **dcw** (Optional, boolean): The bpc setting. Defaults to `True`.
