import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID

CODEOWNERS = ["@weichenlin306"]
DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["psram"]
MULTI_CONF = True

CAMERAS = {
  "ESP32CAM": 0,            #  esp32cam_config
  "ESP32CAM_AITHINKER": 1,  # esp32cam_aithinker_config
  "ESP32CAM_TTGO_T": 2,     # esp32cam_ttgo_t_config
}

FRAME_SIZES = {
  "96X96": 0,     # 96x96
  "QQVGA": 1,     # 160x120
  "128X128": 2,   # 128x128
  "QCIF": 3,      # 176x144
  "HQVGA": 4,     # 240x176
  "240X240": 5,   # 240x240
  "QVGA": 6,      # 320x240
  "320X320": 7,   # 320x320
  "CIF": 8,       # 400x296
  "HVGA": 9,      # 480x320
  "VGA": 10,      # 640x480
  "SVGA": 11,     # 800x600
  "XGA": 12,      # 1024x768
  "HD": 13,       # 1280x720
  "SXGA": 14,     # 1280x1024
  "UXGA": 15,     # 1600x1200
  # 3MP Sensors
  "FHD": 16,      # 1920x1080
  "P_HD": 17,     #  720x1280
  "P_3MP": 18,    #  864x1536
  "QXGA": 19,     # 2048x1536
  # 5MP Sensors
  "QHD": 20,      # 2560x1440
  "FWQXGA": 21,   # 2560x1600
  "P_FHD": 22,    # 1080x1920
  "QSXGA": 23,    # 2560x1920
  "5MP": 24,      # 2592x1944
}

SPECIAL_EFFECTS = {
  "NONE": 0,        # No effect
  "NEGATIVE": 1,    # Negative
  "GRAYSCALE": 2,   # Grayscale
  "RED_TINT": 3,    # Red tint
  "GREEN_TINT": 4,  # Green tint
  "BLUE_TINT": 5,   # Blue tint
  "SEPIA": 6,       # Sepia
}

WHITE_BALANCES = {
  "AUTO": 0,
  "SUNNY": 1,
  "CLOUDY": 2,
  "OFFICE": 3,
  "HOME": 4,
}

GAIN_CEILINGS = {
  "2X": 0,
  "4X": 1,
  "8X": 2,
  "16X": 3,
  "32X": 4,
  "64X": 5,
  "128X": 6,
}

CONF_CAMERA = "camera"
CONF_XCLK_FREQ_HZ = "external_clock_frequency"
CONF_MAX_FRAMERATE = "max_framerate"
CONF_PORT = "port"
CONF_RESOLUTION = "resolution"

CONF_BRIGHTNESS = "brightness"
CONF_CONTRAST = "contrast"
CONF_SATURATION = "saturation"
CONF_SPECIAL_EFFECT = "special_effect"
CONF_WHITE_BALANCE = "white_balance"
CONF_AWB_GAIN = "awb_gain"
CONF_WB_MODE = "wb_mode"
CONF_EXPOSURE_CTRL = "exposure_control"
CONF_AEC2 = "aec2"
CONF_AE_LEVEL = "ae_level"
CONF_AEC_VALUE = "aec_value"
CONF_GAIN_CTRL = "gain_control"
CONF_AGC_GAIN = "agc_gain"
CONF_GAINCEILING = "gain_ceiling"
CONF_BPC = "bpc"
CONF_WPC = "wpc"
CONF_RAW_GMA = "raw_gma"
CONF_HMIRROR = "horizontal_mirror"
CONF_VFLIP = "vertical_flip"
CONF_LENC = "lenc"
CONF_DCW = "dcw"

# C++ namespace
esp32cam_rtsp_server_ns = cg.esphome_ns.namespace("esp32cam_rtsp_server")
Esp32camRtsp = esp32cam_rtsp_server_ns.class_("Esp32camRtsp", cg.Component)

CONFIG_SCHEMA = cv.Schema(
  {
    cv.GenerateID(): cv.declare_id(Esp32camRtsp),
    cv.Optional(CONF_CAMERA, default="esp32cam_aithinker"): cv.enum(CAMERAS, upper=True),
    cv.Optional(CONF_XCLK_FREQ_HZ, default=16000000): cv.int_range(min=10000000, max=20000000),
    cv.Optional(CONF_MAX_FRAMERATE, default="5 fps"): cv.All(cv.framerate, cv.Range(min=0, min_included=False, max=60)),
    cv.Optional(CONF_PORT, default=554): cv.port,
    cv.Required(CONF_RESOLUTION): cv.enum(FRAME_SIZES, upper=True),

    cv.Optional(CONF_BRIGHTNESS, default=0): cv.int_range(min=-2, max=2),
    cv.Optional(CONF_CONTRAST, default=0): cv.int_range(min=-2, max=2),
    cv.Optional(CONF_SATURATION, default=0): cv.int_range(min=-2, max=2),
    cv.Optional(CONF_SPECIAL_EFFECT, default="none"): cv.enum(SPECIAL_EFFECTS, upper=True),
    cv.Optional(CONF_WHITE_BALANCE, default=True): cv.boolean,
    cv.Optional(CONF_AWB_GAIN, default=True): cv.boolean,
    cv.Optional(CONF_WB_MODE, default="auto"): cv.enum(WHITE_BALANCES, upper=True),
    cv.Optional(CONF_EXPOSURE_CTRL, default=True): cv.boolean,
    cv.Optional(CONF_AEC2, default=True): cv.boolean,
    cv.Optional(CONF_AE_LEVEL, default=0): cv.int_range(min=-2, max=2),
    cv.Optional(CONF_AEC_VALUE, default=300): cv.int_range(min=0, max=1200),
    cv.Optional(CONF_GAIN_CTRL, default=False): cv.boolean,
    cv.Optional(CONF_AGC_GAIN, default=6): cv.int_range(min=0, max=30),
    cv.Optional(CONF_GAINCEILING, default="2x"): cv.enum(GAIN_CEILINGS, upper=True),
    cv.Optional(CONF_BPC, default=False): cv.boolean,
    cv.Optional(CONF_WPC, default=True): cv.boolean,
    cv.Optional(CONF_RAW_GMA, default=True): cv.boolean,
    cv.Optional(CONF_HMIRROR, default=False): cv.boolean,
    cv.Optional(CONF_VFLIP, default=False): cv.boolean,
    cv.Optional(CONF_LENC, default=True): cv.boolean,
    cv.Optional(CONF_DCW, default=True): cv.boolean,
  },
).extend(cv.COMPONENT_SCHEMA)

SETTERS = {
  CONF_CAMERA: "set_camera",
  CONF_XCLK_FREQ_HZ: "set_xclk_freq_hz",
  CONF_MAX_FRAMERATE: "set_max_framerate",
  CONF_PORT: "set_port",
  CONF_RESOLUTION: "set_resolution",
  CONF_VFLIP: "set_vertical_flip",
  CONF_HMIRROR: "set_horizontal_mirror",
  CONF_BRIGHTNESS: "set_brightness",
  CONF_CONTRAST: "set_contrast",
  CONF_SATURATION: "set_saturation",
  CONF_GAIN_CTRL: "set_gain_control",
  CONF_AGC_GAIN: "set_agc_gain",

  CONF_SPECIAL_EFFECT: "set_special_effect",
  CONF_WHITE_BALANCE: "set_white_balance",
  CONF_AWB_GAIN: "set_awb_gain",
  CONF_WB_MODE: "set_wb_mode",
  CONF_EXPOSURE_CTRL: "set_exposure_control",
  CONF_AEC2: "set_aec2",
  CONF_AE_LEVEL: "set_ae_level",
  CONF_AEC_VALUE: "set_aec_value",
  CONF_GAINCEILING: "set_gainceiling",
  CONF_BPC: "set_bpc",
  CONF_WPC: "set_wpc",
  CONF_RAW_GMA: "set_raw_gma",
  CONF_LENC: "set_lenc",
  CONF_DCW: "set_dcw",
}

async def to_code(config):
  server = cg.new_Pvariable(config[CONF_ID])

  for key,setter in SETTERS.items():
    if key in config:
      cg.add(getattr(server, setter)(config[key]))

  await cg.register_component(server, config)
