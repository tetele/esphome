#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/uart/uart.h"

#include <string>
#include <vector>

namespace esphome {
namespace desk_raiser_box {

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

enum DeskRaiserCommand : uint8_t {
  COMMAND_NONE,
  COMMAND_GO_TO_MEM_1,
  COMMAND_GO_TO_MEM_2,
  COMMAND_GO_TO_MEM_3,
  COMMAND_PRESS_UP,
  COMMAND_RELEASE_UP,
  COMMAND_PRESS_DN,
  COMMAND_RELEASE_DN,
  COMMAND_UNLOCK_SCREEN,
};

enum DeskRaiserState : uint8_t {
  STATE_SCREEN_OFF,
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

class DeskRaiserBox : public uart::UARTDevice, public Component {
  SUB_BINARY_SENSOR(connected)
  SUB_BINARY_SENSOR(screen_locked)
public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_key_pin(InternalGPIOPin *pin) { this->key_pin_ = pin; }

  bool is_connected() {
    return (this->last_response_timestamp_ >= this->last_request_timestamp_) &&
           (this->last_response_timestamp_ - this->last_request_timestamp_ < 100);
  }

  void go_to_saved_position(uint8_t position);
  void unlock_screen();
  void press_up();
  void release_up();
  void press_dn();
  void release_dn();

 protected:
  InternalGPIOPin *key_pin_;
  std::vector<uint8_t> rx_data_{};
  std::string last_response_;
  uint64_t last_request_timestamp_{0};
  uint64_t last_response_timestamp_{0};
  uint64_t last_valid_response_timestamp_{0};

  bool screen_locked_{true};
  float current_height_;

  void handle_uart();

  void press_key();
  void release_key();
  void send_command(DeskRaiserInteraction command);

 private:
  DeskRaiserState state_{STATE_IDLE};
  DeskRaiserUARTState uart_state_{UART_STATE_READY};
  DeskRaiserCommand next_command_{COMMAND_NONE};
};

template<typename... Ts> class GoToSavedPositionAction : public Action<Ts...>, public Parented<DeskRaiserBox> {
 public:
  TEMPLATABLE_VALUE(uint8_t, position)

  void play(Ts... x) override {
    ESP_LOGW("desk_raiser_box.action", "Going to position %d", this->position_.value(x...));
    this->parent_->go_to_saved_position(this->position_.value(x...));
  }
};

template<typename... Ts> class UnlockScreenAction : public Action<Ts...>, public Parented<DeskRaiserBox> {
 public:
  void play(Ts... x) override { this->parent_->unlock_screen(); }
};

template<typename... Ts> class PressUpAction : public Action<Ts...>, public Parented<DeskRaiserBox> {
 public:
  void play(Ts... x) override { this->parent_->press_up(); }
};

template<typename... Ts> class ReleaseUpAction : public Action<Ts...>, public Parented<DeskRaiserBox> {
 public:
  void play(Ts... x) override { this->parent_->release_up(); }
};

template<typename... Ts> class PressDownAction : public Action<Ts...>, public Parented<DeskRaiserBox> {
 public:
  void play(Ts... x) override { this->parent_->press_dn(); }
};

template<typename... Ts> class ReleaseDownAction : public Action<Ts...>, public Parented<DeskRaiserBox> {
 public:
  void play(Ts... x) override { this->parent_->release_dn(); }
};

}  // namespace desk_raiser_box
}  // namespace esphome
