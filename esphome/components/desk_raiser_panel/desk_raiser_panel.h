#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/desk_raiser_box/desk_raiser_box.h"
#include "esphome/components/uart/uart.h"

#include <string>
#include <vector>

namespace esphome {
namespace desk_raiser_panel {

class DeskRaiserPanel : public uart::UARTDevice, public Component, public Parented<desk_raiser_box::DeskRaiserBox> {
public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_key_pin(InternalGPIOPin *pin) { this->key_pin_ = pin; }

 protected:
  InternalGPIOPin *key_pin_;
  std::vector<uint8_t> rx_data_{};
  uint64_t last_request_timestamp_{0};
  uint64_t last_response_timestamp_{0};
  bool screen_on_{false};
  bool timer_indicator_on_{false};

  float current_height_;

  void turn_on_screen() { this->screen_on_ = true; }
  void turn_off_screen() { this->screen_on_ = false; }
  void turn_on_timer_indicator() { this->timer_indicator_on_ = true; }
  void turn_off_timer_indicator() { this->timer_indicator_on_ = false; }

  void send_response(std::string response);

 private:
  desk_raiser_box::DeskRaiserUARTState uart_state_{desk_raiser_box::UART_STATE_READY};
};

}  // namespace desk_raiser_panel
}  // namespace esphome
