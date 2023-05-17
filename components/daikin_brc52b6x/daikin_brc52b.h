#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome {
namespace daikin_brc52b {

// Temperature
const uint8_t DAIKIN_TEMP_MIN = 16;  // Celsius
const uint8_t DAIKIN_TEMP_MAX = 30;  // Celsius

// Modes
const uint8_t DAIKIN_MODE_AUTO = 0xA;
const uint8_t DAIKIN_MODE_COOL = 0x2;
const uint8_t DAIKIN_MODE_HEAT = 0x8;
const uint8_t DAIKIN_MODE_DRY = 0x1;
const uint8_t DAIKIN_MODE_FAN = 0x4;

// Fan Speed
const uint8_t DAIKIN_FAN_AUTO = 0x10;
const uint8_t DAIKIN_FAN_SILENT = 0x90;
const uint8_t DAIKIN_FAN_1 = 0x80;
const uint8_t DAIKIN_FAN_2 = 0x40;
const uint8_t DAIKIN_FAN_3 = 0x20;
const uint8_t DAIKIN_FAN_4 = 0x30;

// IR Transmission
const uint32_t DAIKIN_IR_FREQUENCY = 38000;
const uint32_t DAIKIN_HEADER1_MARK = 9850;
const uint32_t DAIKIN_HEADER1_SPACE = 9770;
const uint32_t DAIKIN_HEADER2_MARK = 4650;
const uint32_t DAIKIN_HEADER2_SPACE = 2470;
const uint32_t DAIKIN_BIT_MARK = 390;
const uint32_t DAIKIN_ONE_SPACE = 350;
const uint32_t DAIKIN_ZERO_SPACE = 910;
const uint32_t DAIKIN_MESSAGE_SPACE = 20270;

// State Frame size
const uint8_t DAIKIN_STATE_FRAME_SIZE = 8;

class DaikinBRC52bClimate : public climate_ir::ClimateIR {
 public:
  DaikinBRC52bClimate()
      : climate_ir::ClimateIR(DAIKIN_TEMP_MIN, DAIKIN_TEMP_MAX, 1.0f, true, true,
                              {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
                               climate::CLIMATE_FAN_HIGH, climate::CLIMATE_FAN_QUIET},
                              {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL}) {}

 protected:
  // Transmit via IR the state of this climate controller.
  void transmit_state() override;
  uint8_t operation_mode_();
  uint16_t fan_speed_();
  uint8_t temperature_();
  // Handle received IR Buffer
  bool on_receive(remote_base::RemoteReceiveData data) override;
  bool parse_state_frame_(const uint8_t frame[]);
};

}  // namespace daikin_brc52b
}  // namespace esphome
