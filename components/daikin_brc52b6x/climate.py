# Copyright (c) 2019-2023 ESPHome, Justin Seyster
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.automation import maybe_simple_id
from esphome.components import climate_ir, time
from esphome.const import CONF_ASSUMED_STATE, CONF_ID, CONF_TIME_ID

AUTO_LOAD = ["climate_ir"]

daikin_ns = cg.esphome_ns.namespace("daikin_brc52b")
DaikinBRC52bClimate = daikin_ns.class_("DaikinBRC52bClimate", climate_ir.ClimateIR)

CONFIG_SCHEMA = climate_ir.CLIMATE_IR_WITH_RECEIVER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(DaikinBRC52bClimate),
        cv.Optional(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await climate_ir.register_climate_ir(var, config)
    if CONF_TIME_ID in config:
        time = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time_source(time))

# Toggle the ceiling cassette unit's status light.
ToggleCeilingLEDAction = daikin_ns.class_("ToggleCeilingLEDAction", automation.Action)
@automation.register_action(
    "daikin_brc52b6x.toggle_ceiling_led",
    ToggleCeilingLEDAction,
    maybe_simple_id(
        {
            cv.Required(CONF_ID): cv.use_id(DaikinBRC52bClimate),
        }
    )
)
async def daikin_brc52b6x_toggle_ceiling_led_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    return var

# Toggle the wall-mounted air handler's status light.
ToggleWallLEDAction = daikin_ns.class_("ToggleWallLEDAction", automation.Action)
@automation.register_action(
    "daikin_brc52b6x.toggle_wall_led",
    ToggleWallLEDAction,
    maybe_simple_id(
        {
            cv.Required(CONF_ID): cv.use_id(DaikinBRC52bClimate),
        }
    )
)
async def daikin_brc52b6x_toggle_wall_led_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    return var

# Update the component's internal state to indicate whether it is on or off.
UpdateOnOffStateAction = daikin_ns.class_("UpdateOnOffStateAction", automation.Action)
@automation.register_action(
    "daikin_brc52b6x.update_on_off_state",
    UpdateOnOffStateAction,
    maybe_simple_id(
        {
            cv.Required(CONF_ID): cv.use_id(DaikinBRC52bClimate),
            cv.Required(CONF_ASSUMED_STATE): cv.templatable(cv.boolean),
        }
    )
)
async def daikin_brc52b6x_update_on_off_state(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    state_template_ = await cg.templatable(config[CONF_ASSUMED_STATE], args, bool)
    cg.add(var.set_state(state_template_))

    return var
