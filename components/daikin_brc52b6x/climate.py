import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.automation import maybe_simple_id
from esphome.components import climate_ir
from esphome.const import CONF_ID

AUTO_LOAD = ["climate_ir"]

daikin_ns = cg.esphome_ns.namespace("daikin_brc52b")
DaikinBRC52bClimate = daikin_ns.class_("DaikinBRC52bClimate", climate_ir.ClimateIR)

CONFIG_SCHEMA = climate_ir.CLIMATE_IR_WITH_RECEIVER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(DaikinBRC52bClimate),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await climate_ir.register_climate_ir(var, config)

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
