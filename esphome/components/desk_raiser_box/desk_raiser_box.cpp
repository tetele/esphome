#include "esphome/core/log.h"
#include "desk_raiser_box.h"

#include <string>
#include <vector>

namespace esphome {
namespace desk_raiser_box {

static const char *TAG = "desk_raiser_box.component";

char box_to_char(uint8_t c) {
  switch (c) {
    case 0x3f:
    case 0xbf:
      return '0';
    case 0x06:
    case 0x86:
      return '1';
    case 0x5b:
    case 0xdb:
      return '2';
    case 0x4f:
    case 0xcf:
      return '3';
    case 0x66:
    case 0xe6:
      return '4';
    case 0x6d:
    case 0xed:
      return '5';
    case 0x7d:
    case 0xfd:
      return '6';
    case 0x07:
    case 0x87:
      return '7';
    case 0x7f:
    case 0xff:
      return '8';
    case 0x6f:
    case 0xef:
      return '9';
    case 0x00:
    case 0x80:
      return ' ';
    case 0x40:
    case 0xC0:
      return '-';
    case 0x77:
    case 0xf7:
      return 'A';
    case 0x79:
    case 0xf9:
      return 'E';
    case 0x71:
    case 0xf1:
      return 'F';
    case 0x76:
    case 0xf6:
      return 'H';
    case 0x38:
    case 0xb8:
      return 'L';
    case 0x31:
    case 0xb1:
      return 'T';
    case 0x50:
    case 0xd0:
      return 'r';
    default:
      ESP_LOGW(TAG, "Cannot decode 7 segment representation: %x", c);
      return ' ';
  }
}

void DeskRaiserBox::setup() {
  // Code here should perform all component initialization,
  //  whether hardware, memory, or otherwise
}

void DeskRaiserBox::loop() {
  // Tasks here will be performed at every call of the main application loop.
  // Note: code here MUST NOT BLOCK (see below)

  this->handle_uart();

  uint64_t now = millis();

  if ((this->uart_state_ == UART_STATE_READY) &&
      ((now - this->last_request_timestamp_) >
       35)) {  // sparse status requests (>300ms apart) make the box think it can't communicate with the control panel
    this->get_status();
    this->last_request_timestamp_ = now;
  }

  // TODO: move if needed
}

void DeskRaiserBox::dump_config() {
  ESP_LOGCONFIG(TAG, "Desk raiser control box");
  // ESP_LOGCONFIG(TAG, "  foo = %s", TRUEFALSE(this->foo_));
  // ESP_LOGCONFIG(TAG, "  bar = %s", this->bar_.c_str());
  // ESP_LOGCONFIG(TAG, "  baz = %i", this->baz_);
}

void DeskRaiserBox::handle_uart() {
  uint64_t now = millis();

  while (this->available()) {
    uint8_t c;
    float value;
    this->read_byte(&c);

    switch (this->uart_state_) {
      case UART_STATE_SENDING:
        continue;
      case UART_STATE_READY:
        if (c == 0x5A) {
          // Response has begun
          this->uart_state_ = UART_STATE_RECEIVING;
        }
        break;
      case UART_STATE_RECEIVING:
        // Append a byte to response
        this->rx_data_.push_back(c);
        if (this->rx_data_.size() < 5)
          continue;  // read another byte

        uint8_t checksum = (this->rx_data_[0] + this->rx_data_[1] + this->rx_data_[2] + this->rx_data_[3]) & 0xFF;

        if (checksum != this->rx_data_[4]) {
          ESP_LOGW(TAG, "Invalid checksum from desk control box: %02x != %02x", checksum, this->rx_data_[4]);
          this->uart_state_ = UART_STATE_RECEIVED_INVALID;  // Response message from box invalid
        } else {
          this->uart_state_ = UART_STATE_RECEIVED_VALID;  // Response message from box is valid
        }
        break;
    }

    if (this->uart_state_ == UART_STATE_RECEIVED_VALID) {
      // TODO: Decode response
      // TODO: Check 4th byte
      // 0x00	Everything on the display is completely off
      // 0x01	Timer indicator is turned on
      // 0x10	7 segment display is on (3 characters + decimal point)
      // 0x11	7 segment display is on as well as the timer indicator
      std::string response("");
      for (int i = 0; i < 3; i++) {
        response += box_to_char(this->rx_data_[i]);
        if (this->rx_data_[i] & 0x80)
          response += '.';
      }

      if ((this->last_response_ != response) || ((now - this->last_response_timestamp_) > 5000)) {
        this->last_response_ = response;
        this->last_response_timestamp_ = now;
        ESP_LOGV(TAG, "New response received: %s", response.c_str());
      }

      // Restart reading message
      this->uart_state_ = UART_STATE_READY;
      this->rx_data_.clear();
    } else if (this->uart_state_ == UART_STATE_RECEIVED_INVALID) {
      // Restart reading message
      this->uart_state_ = UART_STATE_READY;
      this->rx_data_.clear();
    }
  }
}

void DeskRaiserBox::press_key() { this->key_pin_->digital_write(true); }

void DeskRaiserBox::release_key() { this->key_pin_->digital_write(false); }

void DeskRaiserBox::send_command(DeskRaiserCommand command) {
  std::vector<uint8_t> data = {};

  if (this->uart_state_ != UART_STATE_READY) {
    ESP_LOGW(TAG, "Cannot send command. Device state: %d", this->uart_state_);
    return;
  }

  this->uart_state_ = UART_STATE_SENDING;

  switch (command) {
    case COMMAND_STATUS:
      data.push_back(0x00);
      data.push_back(0x00);
      break;
    case COMMAND_UP:
      data.push_back(0x00);
      data.push_back(0x20);
      break;
    case COMMAND_DN:
      data.push_back(0x00);
      data.push_back(0x40);
      break;
    case COMMAND_UP_DN:
      data.push_back(0x00);
      data.push_back(0x60);
      break;
    case COMMAND_M:
      data.push_back(0x00);
      data.push_back(0x01);
      break;
    case COMMAND_MEM_1:
      data.push_back(0x00);
      data.push_back(0x02);
      break;
    case COMMAND_MEM_2:
      data.push_back(0x00);
      data.push_back(0x04);
      break;
    case COMMAND_MEM_3:
      data.push_back(0x00);
      data.push_back(0x08);
      break;
    case COMMAND_T:
      data.push_back(0x00);
      data.push_back(0x10);
      break;
    case COMMAND_M_T:
      data.push_back(0x00);
      data.push_back(0x11);
      break;
    default:
      ESP_LOGE(TAG, "Unknown command provided: %i", command);
      return;
  }
  data.push_back(0x01);

  uint8_t checksum = 0x00;
  for (auto i : data) {
    checksum = (checksum + i) & 0xFF;
  }

  data.push_back(checksum);
  data.insert(data.begin(), 0xA5);

  this->write_array(data);

  this->uart_state_ = UART_STATE_READY;
}

void DeskRaiserBox::get_status() { this->send_command(COMMAND_STATUS); }

}  // namespace desk_raiser_box
}  // namespace esphome
