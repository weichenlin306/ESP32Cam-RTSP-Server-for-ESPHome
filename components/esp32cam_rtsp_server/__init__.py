import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID

CODEOWNERS = ["@weichenlin306"]
MULTI_CONF = True

CAMERAS = {
  "esp32cam": 0,            #  esp32cam_config
  "esp32cam_aithinker": 1,  # esp32cam_aithinker_config
  "ttgo_t": 2,              # esp32cam_ttgo_t_config
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

CONF_CAMERA = "camera"
CONF_XCLK_FREQ_HZ = "external_clock_frequency"
CONF_PORT = "port"
CONF_RESOLUTION = "resolution"

CONF_BRIGHTNESS = "brightness"
CONF_CONTRAST = "contrast"
CONF_SATURATION = "saturation"
CONF_SPECIAL_EFFECT = "special_effect"
CONF_WHITE_BALANCE_ENABLED = "white_balance_enabled"
CONF_AWB_GAIN_ENABLED = "awb_gain_enabled"
CONF_WB_MODE = "wb_mode"
CONF_EXPOSURE_CTRL_ENABLED = "exposure_control_enabled"
CONF_AEC2_ENABLED = "aec2_enabled"
CONF_AE_LEVEL = "ae_level"
CONF_AEC_VALUE = "aec_value"
CONF_GAIN_CTRL_ENABLED = "gain_control_enabled"
CONF_AGC_GAIN = "agc_gain"
CONF_GAINCEILING = "gainceiling"
CONF_BPC_ENABLED = "bpc_enabled"
CONF_WPC_ENABLED = "wpc_enabled"
CONF_RAW_GMA_ENABLED = "raw_gma_enabled"
CONF_HMIRROR_ENABLED = "horizontal_mirror_enabled"
CONF_VFLIP_ENABLED = "vertical_flip_enabled"
CONF_LENC_ENABLED = "lenc_enabled"
CONF_DCW_ENABLED = "dcw_enabled"

# C++ namespace
esp32cam_rtsp_server_ns = cg.esphome_ns.namespace("esp32cam_rtsp_server")
Esp32camRtsp = esp32cam_rtsp_server_ns.class_("Esp32camRtsp", cg.Component)

CONFIG_SCHEMA = cv.Schema(
  {
    cv.GenerateID(): cv.declare_id(Esp32camRtsp),
    cv.Optional(CONF_CAMERA, default="esp32cam_aithinker"): cv.enum(CAMERAS, lower=True),
    cv.Optional(CONF_XCLK_FREQ_HZ, default=16000000): cv.int_range(min=10000000, max=20000000),
    cv.Optional(CONF_PORT, default=554): cv.port,
    cv.Required(CONF_RESOLUTION): cv.enum(FRAME_SIZES, upper=True),

    cv.Optional(CONF_BRIGHTNESS, default=0): cv.int_range(min=-2, max=2),
    cv.Optional(CONF_CONTRAST, default=0): cv.int_range(min=-2, max=2),
    cv.Optional(CONF_SATURATION, default=0): cv.int_range(min=-2, max=2),
    cv.Optional(CONF_SPECIAL_EFFECT, default=0): cv.int_range(min=0, max=6),
    cv.Optional(CONF_WHITE_BALANCE_ENABLED, default=True): cv.boolean,
    cv.Optional(CONF_AWB_GAIN_ENABLED, default=True): cv.boolean,
    cv.Optional(CONF_WB_MODE, default=0): cv.int_range(min=0, max=4),
    cv.Optional(CONF_EXPOSURE_CTRL_ENABLED, default=True): cv.boolean,
    cv.Optional(CONF_AEC2_ENABLED, default=True): cv.boolean,
    cv.Optional(CONF_AE_LEVEL, default=0): cv.int_range(min=-2, max=2),
    cv.Optional(CONF_AEC_VALUE, default=300): cv.int_range(min=0, max=1200),
    cv.Optional(CONF_GAIN_CTRL_ENABLED, default=False): cv.boolean,
    cv.Optional(CONF_AGC_GAIN, default=6): cv.int_range(min=0, max=30),
    cv.Optional(CONF_GAINCEILING, default=0): cv.int_range(min=0, max=6),
    cv.Optional(CONF_BPC_ENABLED, default=False): cv.boolean,
    cv.Optional(CONF_WPC_ENABLED, default=True): cv.boolean,
    cv.Optional(CONF_RAW_GMA_ENABLED, default=True): cv.boolean,
    cv.Optional(CONF_HMIRROR_ENABLED, default=False): cv.boolean,
    cv.Optional(CONF_VFLIP_ENABLED, default=False): cv.boolean,
    cv.Optional(CONF_LENC_ENABLED, default=True): cv.boolean,
    cv.Optional(CONF_DCW_ENABLED, default=True): cv.boolean,
  },
).extend(cv.COMPONENT_SCHEMA)

SETTERS = {
  CONF_CAMERA: "set_camera",
  CONF_XCLK_FREQ_HZ: "set_xclk_freq_hz",
  CONF_PORT: "set_port",
  CONF_RESOLUTION: "set_resolution",
  CONF_VFLIP_ENABLED: "set_vertical_flip",
  CONF_HMIRROR_ENABLED: "set_horizontal_mirror",
  CONF_BRIGHTNESS: "set_brightness",
  CONF_CONTRAST: "set_contrast",
  CONF_SATURATION: "set_saturation",
  CONF_GAIN_CTRL_ENABLED: "set_gain_control",
  CONF_AGC_GAIN: "set_agc_gain",

  CONF_SPECIAL_EFFECT: "set_special_effect",
  CONF_WHITE_BALANCE_ENABLED: "set_white_balance",
  CONF_AWB_GAIN_ENABLED: "set_awb_gain",
  CONF_WB_MODE: "set_wb_mode",
  CONF_EXPOSURE_CTRL_ENABLED: "set_exposure_control",
  CONF_AEC2_ENABLED: "set_aec2",
  CONF_AE_LEVEL: "set_ae_level",
  CONF_AEC_VALUE: "set_aec_value",
  CONF_GAINCEILING: "set_gainceiling",
  CONF_BPC_ENABLED: "set_bpc",
  CONF_WPC_ENABLED: "set_wpc",
  CONF_RAW_GMA_ENABLED: "set_raw_gma",
  CONF_LENC_ENABLED: "set_lenc",
  CONF_DCW_ENABLED: "set_dcw",
}

async def to_code(config):
  server = cg.new_Pvariable(config[CONF_ID])

  for key,setter in SETTERS.items():
    if key in config:
      cg.add(getattr(server, setter)(config[key]))

  await cg.register_component(server, config)
