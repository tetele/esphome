#pragma once

#include "esphome/core/defines.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/component.h"

namespace esphome {
namespace desk_raiser_box {

class DeskHeightSensor : public sensor::Sensor, public Component {
 public:
  void setup() override;
  void dump_config() override;

  void set_desk(DeskRaiserBox *desk) { this->desk_ = desk; }

 protected:
  DeskRaiserBox *desk_;
};

}  // namespace desk_raiser_box
}  // namespace esphome

