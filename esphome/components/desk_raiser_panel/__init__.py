from esphome import automation, pins
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["uart", "desk_raiser_box"]

CONF_DESK_ID = "desk_id"
CONF_KEY_PIN = "key_pin"
CONF_POSITION = "position"

desk_raiser_panel_ns = cg.esphome_ns.namespace("desk_raiser_panel")
desk_raiser_box_ns = cg.esphome_ns.namespace("desk_raiser_box")
DeskRaiserPanel = desk_raiser_panel_ns.class_(
    "DeskRaiserPanel", cg.Component, uart.UARTDevice
)
DeskRaiserBox = desk_raiser_box_ns.class_(
    "DeskRaiserBox", cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DeskRaiserPanel),
        cv.GenerateID(CONF_DESK_ID): cv.use_id(DeskRaiserBox),
        cv.Required(CONF_KEY_PIN): pins.internal_gpio_output_pin_schema,
    }
).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    key_pin = await cg.gpio_pin_expression(config[CONF_KEY_PIN])

    await cg.register_parented(var, config[CONF_DESK_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_key_pin(key_pin))
