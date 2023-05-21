/*
 * ESPHome BRC52B6X Climate Component
 * Copyright (C) 2023 Justin Seyster
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

#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/optional.h"
#include "daikin_brc52b.h"

namespace esphome {
namespace daikin_brc52b {

template<typename... Ts> class ToggleCeilingLEDAction : public Action<Ts...> {
 public:
  explicit ToggleCeilingLEDAction(DaikinBRC52bClimate *daikin) : daikin_(daikin) {}

  void play(Ts... x) override { daikin_->toggle_ceiling_led(); }

 protected:
  DaikinBRC52bClimate *daikin_;
};

template<typename... Ts> class ToggleWallLEDAction : public Action<Ts...> {
 public:
  explicit ToggleWallLEDAction(DaikinBRC52bClimate *daikin) : daikin_(daikin) {}

  void play(Ts... x) override { daikin_->toggle_wall_led(); }

 protected:
  DaikinBRC52bClimate *daikin_;
};

template<typename... Ts> class UpdateOnOffStateAction : public Action<Ts...> {
 public:
  explicit UpdateOnOffStateAction(DaikinBRC52bClimate *daikin) : daikin_(daikin) {}

  void set_state(bool state) { state_ = state; };
  void play(Ts... x) override { daikin_->update_on_off_state(state_); }

 protected:
  DaikinBRC52bClimate *daikin_;
  bool state_;
};

}  // namespace daikin_brc52b
}  // namespace esphome
