import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_LOCK,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

from . import CONF_DESK_ID, DeskRaiserBox

CONF_CONNECTED = "connected"
CONF_SCREEN_LOCKED = "screen_locked"


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_DESK_ID): cv.use_id(DeskRaiserBox),
        cv.Optional(CONF_SCREEN_LOCKED): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_LOCK, entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    desk = await cg.get_variable(config[CONF_DESK_ID])

    # if connected_sensor_config := config.get(CONF_CONNECTED):
    #     sens = await binary_sensor.new_binary_sensor(connected_sensor_config)
    #     cg.add(desk.set_connected_binary_sensor(sens))

    # if screen_locked_sensor_config := config.get(CONF_SCREEN_LOCKED):
    #     sens = await binary_sensor.new_binary_sensor(screen_locked_sensor_config)
    #     cg.add(desk.set_screen_locked_binary_sensor(sens))
