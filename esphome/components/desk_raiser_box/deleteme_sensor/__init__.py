import esphome.codegen as cg
from esphome.components import sensor, uart
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_DISTANCE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_RULER,
    STATE_CLASS_MEASUREMENT,
)

CONF_DESK_ID = "desk_id"

desk_raiser_box_ns = cg.esphome_ns.namespace("desk_raiser_box")
DeskHeightSensor = desk_raiser_box_ns.class_(
    "DeskHeightSensor", sensor.Sensor, cg.Component
)
DeskRaiserBox = desk_raiser_box_ns.class_(
    "DeskRaiserBox", cg.Component, uart.UARTDevice
)


CONFIG_SCHEMA = (
    sensor.sensor_schema(
        DeskHeightSensor,
        icon=ICON_RULER,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_DISTANCE,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        cv.Schema(
            {
                cv.GenerateID(CONF_DESK_ID): cv.use_id(DeskRaiserBox),
            }
        )
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    if desk_id_config := config.get(CONF_DESK_ID):
        desk_id = await cg.get_variable(desk_id_config)
        cg.add(var.set_desk(desk_id))
