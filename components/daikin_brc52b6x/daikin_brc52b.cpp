/*
 * ESPHome BRC52B6X Climate Component
 * Copyright (C) 2021-2023  ESPHome, Justin Seyster
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iomanip>
#include <sstream>

#include "daikin_brc52b.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/core/optional.h"

namespace esphome {
namespace daikin_brc52b {

static const char *const TAG = "daikin_brc52b6x.climate";

namespace {
std::string daikin_frame_to_string(uint8_t frame[DAIKIN_STATE_FRAME_SIZE]) {
  std::ostringstream stream;
  std::hex(stream);
  for (int i = 0; i < DAIKIN_STATE_FRAME_SIZE; i++) {
    stream << (i == 0 ? "" : " ") << "0x" << std::setfill('0') << std::setw(2) << static_cast<int>(frame[i]);
  }
  return stream.str();
}

void encode_byte(remote_base::RemoteTransmitData &data, uint8_t byte) {
  // Note: bytes are transmitted from least to most significant.
  for (int i = 0; i < 8; i++) {
    data.item(DAIKIN_BIT_MARK, (byte & 0x1) ? DAIKIN_ONE_SPACE : DAIKIN_ZERO_SPACE);
    byte >>= 1;
  }
}

// Convert a number in the range [00-99] to binary-coded decimal, with the tens digit in the upper four bits and the
// ones digit in the lower four bits.
uint8_t encode_bcd(uint8_t value) { return ((value / 10) << 4) | ((value % 10) & 0xF); }

// Inverse of encode_bcd().
uint8_t decode_bcd(uint8_t value_bcd) { return 10 * (value_bcd >> 4) + (value_bcd & 0xF); }
}  // namespace

void DaikinBRC52bClimate::transmit_state_with_led_commands(bool ceiling_led_command, bool wall_led_command) {
  if (this->mode != climate::CLIMATE_MODE_OFF) {
    this->saved_mode_ = this->mode;
  }

  uint8_t minute = 0;
  uint8_t hour = 0;
  if (this->time_source_) {
    auto now = this->time_source_->now();
    minute = encode_bcd(now.minute);
    hour = encode_bcd(now.hour);
  }

  uint8_t state_frame[DAIKIN_STATE_FRAME_SIZE];
  state_frame[0] = DAIKIN_FRAME_1_HEADER;
  state_frame[1] = this->encode_mode() | this->encode_fan_speed();
  state_frame[2] = minute;
  state_frame[3] = hour;
  state_frame[4] = 0x13;  // TODO
  state_frame[5] = 0x04;  // TODO
  state_frame[6] = this->encode_temperature();
  state_frame[7] = this->encode_power_toggle() | 0x4 | this->encode_fan_swing();

  uint8_t state_checksum = 0;
  for (int i = 0; i < DAIKIN_STATE_FRAME_SIZE; i++) {
    uint8_t byte = state_frame[i];
    state_checksum += (byte >> 4) + (byte & 0xF);
  }
  state_frame[7] |= (state_checksum & 0xF) << 4;

  uint8_t control_frame[DAIKIN_STATE_FRAME_SIZE] = {0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  if (ceiling_led_command) {
    control_frame[1] |= DAIKIN_TOGGLE_CEILING_LED;
  }
  if (wall_led_command) {
    control_frame[1] |= DAIKIN_TOGGLE_WALL_LED;
  }

  uint8_t control_checksum = 0;
  for (int i = 0; i < DAIKIN_STATE_FRAME_SIZE; i++) {
    uint8_t byte = control_frame[i];
    control_checksum += (byte >> 4) + (byte & 0xF);
  }
  control_frame[7] = control_checksum;

  ESP_LOGV(TAG, "Transmitting state frame:   %s", daikin_frame_to_string(state_frame).c_str());
  ESP_LOGVV(TAG, "Transmitting control frame: %s", daikin_frame_to_string(control_frame).c_str());

  auto transmit = this->transmitter_->transmit();
  auto *data = transmit.get_data();
  data->set_carrier_frequency(DAIKIN_IR_FREQUENCY);

  // Hack: Send a "negative header" so that _this_ receiver can ignore its own IR commands. These extra pulses make the
  // message unrecognizable to our receiver but not to the unit being controlled.
  //
  // We want to avoid mistaking our own power toggle command for a power toggle command from the remote, which would
  // make it difficult to properly track whether the unit is powered on.
  data->item(250, 250);
  data->item(250, 250);

  data->item(DAIKIN_HEADER1_MARK, DAIKIN_HEADER1_SPACE);
  data->item(DAIKIN_HEADER1_MARK, DAIKIN_HEADER1_SPACE);
  data->item(DAIKIN_HEADER2_MARK, DAIKIN_HEADER2_SPACE);

  for (int i = 0; i < DAIKIN_STATE_FRAME_SIZE; i++) {
    encode_byte(*data, state_frame[i]);
  }

  data->item(DAIKIN_BIT_MARK, DAIKIN_FRAME_GAP);

  for (int i = 0; i < DAIKIN_STATE_FRAME_SIZE; i++) {
    encode_byte(*data, control_frame[i]);
  }

  encode_byte(*data, 0);
  data->item(DAIKIN_HEADER2_MARK, DAIKIN_HEADER2_SPACE);

  transmit.perform();
}

uint8_t DaikinBRC52bClimate::encode_mode() const {
  switch (this->saved_mode_) {
    case climate::CLIMATE_MODE_COOL:
      return DAIKIN_MODE_COOL;
    case climate::CLIMATE_MODE_DRY:
      return DAIKIN_MODE_DRY;
    case climate::CLIMATE_MODE_HEAT:
      return DAIKIN_MODE_HEAT;
    case climate::CLIMATE_MODE_HEAT_COOL:
      return DAIKIN_MODE_AUTO;
    case climate::CLIMATE_MODE_FAN_ONLY:
    case climate::CLIMATE_MODE_OFF:
      // The saved mode should never be OFF.
    default:
      return DAIKIN_MODE_FAN;
  }
}

climate::ClimateMode DaikinBRC52bClimate::decode_mode(uint8_t modeAndFanData) const {
  switch (modeAndFanData & 0xF) {
    case DAIKIN_MODE_COOL:
      return climate::CLIMATE_MODE_COOL;
    case DAIKIN_MODE_DRY:
      return climate::CLIMATE_MODE_DRY;
    case DAIKIN_MODE_HEAT:
      return climate::CLIMATE_MODE_HEAT;
    case DAIKIN_MODE_AUTO:
      return climate::CLIMATE_MODE_HEAT_COOL;
    default:
    case DAIKIN_MODE_FAN:
      return climate::CLIMATE_MODE_FAN_ONLY;
  }
}

uint8_t DaikinBRC52bClimate::encode_fan_speed() const {
  switch (this->fan_mode.value()) {
    case climate::CLIMATE_FAN_LOW:
      return DAIKIN_FAN_1;
    case climate::CLIMATE_FAN_MEDIUM:
      return DAIKIN_FAN_2;
    case climate::CLIMATE_FAN_HIGH:
      return DAIKIN_FAN_3;
    case climate::CLIMATE_FAN_QUIET:
      return DAIKIN_FAN_QUIET;
    case climate::CLIMATE_FAN_AUTO:
    default:
      return DAIKIN_FAN_AUTO;
  }
}

climate::ClimateFanMode DaikinBRC52bClimate::decode_fan_speed(uint8_t modeAndFanData) const {
  switch (modeAndFanData & 0xF0) {
    case DAIKIN_FAN_1:
      return climate::CLIMATE_FAN_LOW;
    case DAIKIN_FAN_2:
      return climate::CLIMATE_FAN_MEDIUM;
    case DAIKIN_FAN_3:
    case DAIKIN_FAN_STRONG:
      return climate::CLIMATE_FAN_HIGH;
    case DAIKIN_FAN_QUIET:
      return climate::CLIMATE_FAN_QUIET;
    case DAIKIN_FAN_AUTO:
    default:
      return climate::CLIMATE_FAN_AUTO;
  }
}

uint8_t DaikinBRC52bClimate::encode_fan_swing() const {
  return (this->swing_mode == climate::CLIMATE_SWING_OFF) ? 0x0 : DAIKIN_FAN_SWING_VERTICAL;
}

climate::ClimateSwingMode DaikinBRC52bClimate::decode_fan_swing(uint8_t fandAndPowerToggleByte) const {
  return (fandAndPowerToggleByte & DAIKIN_FAN_SWING_VERTICAL) ? climate::CLIMATE_SWING_VERTICAL
                                                              : climate::CLIMATE_SWING_OFF;
}

uint8_t DaikinBRC52bClimate::encode_temperature() const {
  auto target_temperature = roundf(clamp<float>(this->target_temperature, DAIKIN_TEMP_MIN, DAIKIN_TEMP_MAX));
  return encode_bcd(static_cast<uint8_t>(target_temperature));
}

uint8_t DaikinBRC52bClimate::encode_power_toggle() {
  if (this->mode == climate::CLIMATE_MODE_OFF ? this->power_on_ : !this->power_on_) {
    this->power_on_ = !this->power_on_;
    return DAIKIN_TOGGLE_POWER;
  } else {
    return 0x0;
  }
}

bool DaikinBRC52bClimate::parse_state_frame(const uint8_t frame[]) {
  if (frame[7] & DAIKIN_TOGGLE_POWER) {
    this->power_on_ = !this->power_on_;
  }

  this->saved_mode_ = this->decode_mode(frame[1] & 0xF);
  this->mode = this->power_on_ ? this->saved_mode_ : climate::CLIMATE_MODE_OFF;

  this->target_temperature = decode_bcd(frame[6]);
  this->fan_mode = this->decode_fan_speed(frame[1]);
  this->swing_mode = this->decode_fan_swing(frame[7]);

  this->publish_state();
  return true;
}

namespace {
optional<uint8_t> expect_byte(remote_base::RemoteReceiveData &data) {
  uint8_t result = 0;

  // Note: bytes are received from least to most significant.
  for (int i = 0; i < 8; i++) {
    if (data.expect_item(DAIKIN_BIT_MARK, DAIKIN_ONE_SPACE)) {
      result = (result >> 1) | 0x80;
    } else if (data.expect_item(DAIKIN_BIT_MARK, DAIKIN_ZERO_SPACE)) {
      result >>= 1;
    } else {
      return {};
    }
  }

  return result;
}
}  // namespace

bool DaikinBRC52bClimate::on_receive(remote_base::RemoteReceiveData data) {
  bool gotHeader = data.expect_item(DAIKIN_HEADER1_MARK, DAIKIN_HEADER1_SPACE) &&
                   data.expect_item(DAIKIN_HEADER1_MARK, DAIKIN_HEADER1_SPACE) &&
                   data.expect_item(DAIKIN_HEADER2_MARK, DAIKIN_HEADER2_SPACE);
  if (!gotHeader) {
    return false;
  }

  int checksum = 0;
  uint8_t state_frame[DAIKIN_STATE_FRAME_SIZE] = {};
  for (int i = 0; i < DAIKIN_STATE_FRAME_SIZE; i++) {
    auto byte = expect_byte(data);
    if (!byte || (i == 0 && *byte != 0x16)) {
      return false;
    }

    if (i != 7) {
      checksum += (*byte >> 4) + (*byte & 0xF);
    } else {
      // Do not include the received checksum in the checksum calculation.
      checksum += *byte & 0xF;
    }

    state_frame[i] = *byte;
  }

  if ((checksum & 0xF) != (state_frame[7] >> 4)) {
    ESP_LOGW(TAG, "Received IR data with invalid checksum");
    return false;
  }

  ESP_LOGV(TAG, "Received state frame:   %s", daikin_frame_to_string(state_frame).c_str());

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERY_VERBOSE
  // There is almost no useful information in the control frame, but we capture and log it at the highest verbosity
  // setting.
  uint8_t control_frame[DAIKIN_STATE_FRAME_SIZE] = {};
  data.expect_item(DAIKIN_BIT_MARK, DAIKIN_FRAME_GAP);
  for (int i = 0; i < DAIKIN_STATE_FRAME_SIZE; i++) {
    auto byte = expect_byte(data);
    control_frame[i] = byte ? *byte : 0;
  }
  ESP_LOGVV(TAG, "Received control frame: %s", daikin_frame_to_string(control_frame).c_str());
#endif

  return this->parse_state_frame(state_frame);
}

}  // namespace daikin_brc52b
}  // namespace esphome
