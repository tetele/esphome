from esphome import core, pins
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["uart"]

CONF_DESK_ID = "desk_id"
CONF_CONTROLLER_UART_ID = "controller_uart_id"
CONF_CONTROLLER_TX_PIN = "tx_to_controller_pin"
CONF_CONTROLLER_RX_PIN = "rx_from_controller_pin"
CONF_CONTROLLER_KEYPRESS_PIN = "controller_keypress_pin"
CONF_PANEL_UART_ID = "panel_uart_id"
CONF_PANEL_KEYPRESS_PIN = "panel_keypress_pin"
CONF_PANEL_TX_PIN = "tx_to_panel_pin"
CONF_PANEL_RX_PIN = "rx_from_panel_pin"
CONF_POSITION = "position"

desk_raiser_ns = cg.esphome_ns.namespace("desk_raiser")
DeskRaiser = desk_raiser_ns.class_("DeskRaiser", cg.Component)


def uart_component_type(value):
    if core.CORE.is_esp8266:
        return cv.declare_id(uart.ESP8266UartComponent)(value)
    if core.CORE.is_esp32:
        if core.CORE.using_arduino:
            return cv.declare_id(uart.ESP32ArduinoUARTComponent)(value)
        if core.CORE.using_esp_idf:
            return cv.declare_id(uart.IDFUARTComponent)(value)
    if core.CORE.is_rp2040:
        return cv.declare_id(uart.RP2040UartComponent)(value)
    if core.CORE.is_libretiny:
        return cv.declare_id(uart.LibreTinyUARTComponent)(value)
    if core.CORE.is_host:
        return cv.declare_id(uart.HostUartComponent)(value)
    raise NotImplementedError


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DeskRaiser),
        cv.GenerateID(CONF_CONTROLLER_UART_ID): uart_component_type,
        cv.Required(CONF_CONTROLLER_TX_PIN): pins.internal_gpio_output_pin_schema,
        cv.Required(CONF_CONTROLLER_RX_PIN): pins.internal_gpio_input_pin_schema,
        cv.Optional(CONF_CONTROLLER_KEYPRESS_PIN): pins.internal_gpio_output_pin_schema,
        cv.GenerateID(CONF_PANEL_UART_ID): uart_component_type,
        cv.Optional(CONF_PANEL_TX_PIN): pins.internal_gpio_output_pin_schema,
        cv.Optional(CONF_PANEL_RX_PIN): pins.internal_gpio_input_pin_schema,
        cv.Optional(CONF_PANEL_KEYPRESS_PIN): pins.internal_gpio_input_pin_schema,
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    controller_uart_config = {
        CONF_ID: config[CONF_CONTROLLER_UART_ID],
        uart.CONF_BAUD_RATE: 9600,
        uart.CONF_TX_PIN: config[CONF_CONTROLLER_TX_PIN],
        uart.CONF_RX_PIN: config[CONF_CONTROLLER_RX_PIN],
        # uart.CONF_PARITY: uart.UARTParityOptions.UART_PARITY_OPTIONS_NONE,
        # uart.CONF_DATA_BITS: 8,
        # uart.CONF_STOP_BITS: 1,
    }

    controller_uart = cg.new_Pvariable(config[CONF_CONTROLLER_UART_ID])
    await cg.register_component(controller_uart, controller_uart_config)
    cg.add(var.set_controller_uart(controller_uart))

    if CONF_PANEL_TX_PIN in config and CONF_PANEL_RX_PIN in config:
        panel_uart_config = {
            CONF_ID: config[CONF_PANEL_UART_ID],
            uart.CONF_BAUD_RATE: 9600,
            uart.CONF_TX_PIN: config[CONF_PANEL_TX_PIN],
            uart.CONF_RX_PIN: config[CONF_PANEL_RX_PIN],
            # uart.CONF_PARITY: uart.UARTParityOptions.UART_PARITY_OPTIONS_NONE,
            # uart.CONF_DATA_BITS: 8,
            # uart.CONF_STOP_BITS: 1,
        }

        panel_uart = cg.new_Pvariable(config[CONF_PANEL_UART_ID])
        await cg.register_component(panel_uart, panel_uart_config)
        cg.add(var.set_panel_uart(panel_uart))
