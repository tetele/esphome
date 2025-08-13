from esphome import automation, pins
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor"]

CONF_DESK_ID = "desk_id"
CONF_KEY_PIN = "key_pin"
CONF_POSITION = "position"

desk_raiser_box_ns = cg.esphome_ns.namespace("desk_raiser_box")
DeskRaiserBox = desk_raiser_box_ns.class_(
    "DeskRaiserBox", cg.Component, uart.UARTDevice
)
GoToSavedPositionAction = desk_raiser_box_ns.class_(
    "GoToSavedPositionAction", automation.Action
)
UnlockScreenAction = desk_raiser_box_ns.class_("UnlockScreenAction", automation.Action)
PressUpAction = desk_raiser_box_ns.class_("PressUpAction", automation.Action)
ReleaseUpAction = desk_raiser_box_ns.class_("ReleaseUpAction", automation.Action)
PressDownAction = desk_raiser_box_ns.class_("PressDownAction", automation.Action)
ReleaseDownAction = desk_raiser_box_ns.class_("ReleaseDownAction", automation.Action)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DeskRaiserBox),
        cv.Required(CONF_KEY_PIN): pins.internal_gpio_output_pin_schema,
    }
).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    key_pin = await cg.gpio_pin_expression(config[CONF_KEY_PIN])

    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_key_pin(key_pin))


@automation.register_action(
    "desk_raiser_box.go_to_saved_position",
    GoToSavedPositionAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(DeskRaiserBox),
            cv.Required(CONF_POSITION): cv.Range(min=1, max=3),
        }
    ),
)
async def desk_raiser_box_go_to_saved_position_to_code(
    config, action_id, template_arg, args
):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "desk_raiser_box.unlock_screen",
    UnlockScreenAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(DeskRaiserBox),
        }
    ),
)
async def desk_raiser_box_unlock_screen_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "desk_raiser_box.press_up",
    PressUpAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(DeskRaiserBox),
        }
    ),
)
async def desk_raiser_box_press_up_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "desk_raiser_box.release_up",
    ReleaseUpAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(DeskRaiserBox),
        }
    ),
)
async def desk_raiser_box_release_up_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "desk_raiser_box.press_down",
    PressDownAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(DeskRaiserBox),
        }
    ),
)
async def desk_raiser_box_press_down_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "desk_raiser_box.release_down",
    ReleaseDownAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(DeskRaiserBox),
        }
    ),
)
async def desk_raiser_box_release_down_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var
