#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#include <string>
#include <vector>

namespace esphome {
namespace desk_raiser_box {

enum DeskRaiserCommand : uint8_t {
  COMMAND_STATUS,
  COMMAND_UP,
  COMMAND_DN,
  COMMAND_UP_DN,
  COMMAND_M,
  COMMAND_MEM_1,
  COMMAND_MEM_2,
  COMMAND_MEM_3,
  COMMAND_T,
  COMMAND_M_T,
};

enum DeskRaiserState : uint8_t {
  STATE_SCREEN_OFF,
  STATE_IDLE,
  STATE_RAISING,
  STATE_LOWERING,
};

enum DeskRaiserUARTState : uint8_t {
  UART_STATE_READY,
  UART_STATE_SENDING,
  UART_STATE_RECEIVING,
  UART_STATE_RECEIVED_VALID,
  UART_STATE_RECEIVED_INVALID,
  UART_STATE_NOT_READY,
};

class DeskRaiserBox : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_key_pin(InternalGPIOPin *pin) { this->key_pin_ = pin; }

 protected:
  InternalGPIOPin *key_pin_;
  std::vector<uint8_t> rx_data_{};
  std::string last_response_;
  uint64_t last_request_timestamp_{0};
  uint64_t last_response_timestamp_{0};

  void press_key();
  void release_key();
  void send_command(DeskRaiserCommand command);

  void get_status();

 private:
  DeskRaiserState state_{STATE_IDLE};
  DeskRaiserUARTState uart_state_{UART_STATE_READY};
};

}  // namespace desk_raiser_box
}  // namespace esphome
