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

}  // namespace daikin_brc52b
}  // namespace esphome
