#pragma once

#include "esphome/components/climate_ir/climate_ir.h"
#include "esphome/components/time/real_time_clock.h"

namespace esphome {
namespace daikin_brc52b {

const uint8_t DAIKIN_FRAME_1_HEADER = 0x16;
const uint8_t DAIKIN_FRAME_2_HEADER = 0xA1;

const uint8_t DAIKIN_TOGGLE_POWER = 0x8;

const uint8_t DAIKIN_TOGGLE_CEILING_LED = 0x1;
const uint8_t DAIKIN_TOGGLE_WALL_LED = 0x8;

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
const uint8_t DAIKIN_FAN_QUIET = 0x90;
const uint8_t DAIKIN_FAN_1 = 0x80;
const uint8_t DAIKIN_FAN_2 = 0x40;
const uint8_t DAIKIN_FAN_3 = 0x20;
const uint8_t DAIKIN_FAN_STRONG = 0x30;

const uint8_t DAIKIN_FAN_SWING_VERTICAL = 0x1;

// IR Transmission
const uint32_t DAIKIN_IR_FREQUENCY = 38000;
const uint32_t DAIKIN_HEADER1_MARK = 9850;
const uint32_t DAIKIN_HEADER1_SPACE = 9770;
const uint32_t DAIKIN_HEADER2_MARK = 4650;
const uint32_t DAIKIN_HEADER2_SPACE = 2470;
const uint32_t DAIKIN_BIT_MARK = 390;
const uint32_t DAIKIN_ONE_SPACE = 910;
const uint32_t DAIKIN_ZERO_SPACE = 350;
const uint32_t DAIKIN_FRAME_GAP = 20270;

// State Frame size
const uint8_t DAIKIN_STATE_FRAME_SIZE = 8;

class DaikinBRC52bClimate : public climate_ir::ClimateIR {
 public:
  DaikinBRC52bClimate()
      : climate_ir::ClimateIR(DAIKIN_TEMP_MIN, DAIKIN_TEMP_MAX, 1.0f, true, true,
                              {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
                               climate::CLIMATE_FAN_HIGH, climate::CLIMATE_FAN_QUIET},
                              {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL}) {}

  void set_time_source(time::RealTimeClock *time_source) { this->time_source_ = time_source; }

  void toggle_ceiling_led() { this->transmit_state_with_led_commands(true, false); };
  void toggle_wall_led() { this->transmit_state_with_led_commands(false, true); }

 protected:
  // Transmit via IR the state of this climate controller.
  void transmit_state() override { transmit_state_with_led_commands(false, false); };
  void transmit_state_with_led_commands(bool ceiling_led_command, bool wall_led_command);

  uint8_t encode_mode() const;
  climate::ClimateMode decode_mode(uint8_t modeAndFanData) const;

  uint8_t encode_fan_speed() const;
  climate::ClimateFanMode decode_fan_speed(uint8_t modeAndFanData) const;

  uint8_t encode_fan_swing() const;
  climate::ClimateSwingMode decode_fan_swing(uint8_t fandAndPowerToggleByte) const;

  uint8_t encode_temperature() const;

  uint8_t encode_power_toggle();

  // Handle received IR Buffer
  bool on_receive(remote_base::RemoteReceiveData data) override;
  bool parse_state_frame(const uint8_t frame[]);

  time::RealTimeClock *time_source_{nullptr};

  bool power_on_{false};

  // It is possible to change the unit's operation mode while it is off. We store that state here so that we know what
  // state to use when powering the unit back on.
  climate::ClimateMode saved_mode_{climate::CLIMATE_MODE_FAN_ONLY};
};

}  // namespace daikin_brc52b
}  // namespace esphome
