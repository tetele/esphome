#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#include <vector>
#include <string>

namespace esphome {
namespace desk_raiser {

enum DeskRaiserState : uint8_t {
  STATE_IDLE,
  STATE_EXECUTING_COMMAND,
};

enum DeskRaiserUARTState : uint8_t {
  UART_STATE_READY,
  UART_STATE_SENDING,
  UART_STATE_RECEIVING,
  UART_STATE_RECEIVED_VALID,
  UART_STATE_RECEIVED_INVALID,
  UART_STATE_NOT_READY,
};

enum DeskRaiserInteraction : uint8_t {
  INTERACTION_STATUS,
  INTERACTION_UP,
  INTERACTION_DN,
  INTERACTION_UP_DN,
  INTERACTION_M,
  INTERACTION_MEM_1,
  INTERACTION_MEM_2,
  INTERACTION_MEM_3,
  INTERACTION_T,
  INTERACTION_M_T,
};

class DeskRaiserUART {
 public:
  DeskRaiserUART(uart::UARTComponent *uart, size_t message_size, uint8_t signature_byte) {
    this->uart_ = uart;
    this->message_size_ = message_size;
    this->signature_byte_ = signature_byte;
  }

  void loop();

  DeskRaiserUARTState get_uart_state() { return this->uart_state_; }
  uint64_t get_last_request_timestamp() { return this->last_request_timestamp_; }
  uint64_t get_last_response_timestamp() { return this->last_response_timestamp_; }
  std::vector<uint8_t> get_response() { return this->rx_data_; }

  void restart();

  void send_message(std::vector<uint8_t> bytes);

 protected:
  uart::UARTComponent *uart_{nullptr};
  DeskRaiserUARTState uart_state_{UART_STATE_READY};
  std::vector<uint8_t> rx_data_{};

  uint64_t last_request_timestamp_{0};
  uint64_t last_response_timestamp_{0};

 private:
  size_t message_size_{0};
  uint8_t signature_byte_{0x00};
};

class DeskRaiserControllerUART : public DeskRaiserUART {
 public:
  DeskRaiserControllerUART(uart::UARTComponent *uart) : DeskRaiserUART(uart, 5, 0x5A){};

  void loop();

  std::string get_last_response() { return this->last_response_; }

  bool decode_valid_response();

 protected:
  std::string last_response_;
  uint64_t last_valid_response_timestamp_{0};

  DeskRaiserInteraction current_message_{INTERACTION_STATUS};
};

class DeskRaiserPanelUART : public DeskRaiserUART {
 public:
  DeskRaiserPanelUART(uart::UARTComponent *uart) : DeskRaiserUART(uart, 4, 0xA5){};

  bool decode_valid_response();

  void send_text(std::string text);
protected:
  DeskRaiserInteraction last_response_{INTERACTION_STATUS};
  uint64_t last_valid_response_timestamp_{0};
};

class DeskRaiser : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_controller_uart(uart::UARTComponent *controller_uart) {
    this->controller_ = new DeskRaiserControllerUART(controller_uart);
  }
  void set_panel_uart(uart::UARTComponent *panel_uart) { this->panel_ = new DeskRaiserPanelUART(panel_uart); }

 protected:
  DeskRaiserControllerUART *controller_{nullptr};
  DeskRaiserPanelUART *panel_{nullptr};

  std::string last_controller_response_;
  uint64_t current_loop_timestamp_{0};
  uint64_t last_valid_controller_response_timestamp_{0};

  float current_height_;
  DeskRaiserState state_{STATE_IDLE};
};

char box_to_char(uint8_t c);
uint8_t char_to_panel(char c);
std::vector<uint8_t> interaction_bytes(DeskRaiserInteraction interaction);

}  // namespace desk_raiser
}  // namespace esphome
