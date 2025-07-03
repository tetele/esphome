from esphome import pins
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]

CONF_KEY_PIN = "key_pin"

desk_raiser_box_ns = cg.esphome_ns.namespace("desk_raiser_box")
DeskRaiserBox = desk_raiser_box_ns.class_(
    "DeskRaiserBox", cg.Component, uart.UARTDevice
)

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
