#include "esphome/core/log.h"
#include "desk_raiser_panel.h"

#ifdef USE_ESP_IDF
#include "esphome/components/uart/uart_component_esp_idf.h"
#endif  // USE_ESP_IDF

#include <string>
#include <vector>

namespace esphome {
namespace desk_raiser_panel {

static const char *TAG = "desk_raiser_panel.component";

uint8_t char_to_panel(char c) {
  switch (c) {
    case '0':
      return 0x3f;
    case '1':
      return 0x06;
    case '2':
      return 0x5b;
    case '3':
      return 0x4f;
    case '4':
      return 0x66;
    case '5':
      return 0x6d;
    case '6':
      return 0x7d;
    case '7':
      return 0x07;
    case '8':
      return 0x7f;
    case '9':
      return 0x6f;
    case ' ':
      return 0x00;
    case '-':
      return 0x40;
    case 'A':
    case 'a':
      return 0x77;
    case 'E':
    case 'e':
      return 0x79;
    case 'F':
    case 'f':
      return 0x71;
    case 'H':
    case 'h':
      return 0x76;
    case 'L':
    case 'l':
      return 0x38;
    case 'T':
    case 't':
      return 0x31;
    case 'R':
    case 'r':
      return 0x50;
    default:
      ESP_LOGW(TAG, "Cannot encode 7 segment representation: %c", c);
      return 0x00;
  }
}

void DeskRaiserPanel::setup() {
  // Code here should perform all component initialization,
  //  whether hardware, memory, or otherwise
  this->turn_on_screen();
#ifdef USE_ESP_IDF
  // esphome::uart::IDFUARTComponent *uart_parent;
  // uart_parent = dynamic_cast<esphome::uart::IDFUARTComponent*>(this->parent_);
  // uart_set_rx_full_threshold((uart_port_t)uart_parent->get_hw_serial_number(), 5);

  uart_set_rx_full_threshold(UART_NUM_1, 1);

  // uart_set_rx_timeout(UART_NUM_1, 1);
#endif
}

void DeskRaiserPanel::loop() {
  // Tasks here will be performed at every call of the main application loop.
  // Note: code here MUST NOT BLOCK (see below)

  uint64_t now = millis();
  static uint64_t last_loop_timestamp = 0;
  static int loop_number = 0;
  size_t UART_bytes_available;
  loop_number++;

  while (UART_bytes_available = this->available()) {
    uint8_t c;
    // ESP_LOGV(TAG, "(loop %i - %ims) %i bytes in RX buffer", loop_number, (int)(now-last_loop_timestamp), UART_bytes_available);

    switch (this->uart_state_) {
      case desk_raiser_box::UART_STATE_SENDING:
        continue;
      case desk_raiser_box::UART_STATE_READY:
        this->read_byte(&c);
        if (c == 0xA5) {
          // Request has begun
          this->uart_state_ = desk_raiser_box::UART_STATE_RECEIVING;
        }
        break;
      case desk_raiser_box::UART_STATE_RECEIVING:
        // Append a byte to response
        this->read_byte(&c);
        this->rx_data_.push_back(c);
        if (this->rx_data_.size() < 4)
          continue;  // read another byte

        uint8_t checksum = (this->rx_data_[0] + this->rx_data_[1] + this->rx_data_[2]) & 0xFF;

        if (checksum != this->rx_data_[3]) {
          ESP_LOGW(TAG, "Invalid checksum from desk control panel: %02x != %02x", checksum, this->rx_data_[3]);
          this->uart_state_ = desk_raiser_box::UART_STATE_RECEIVED_INVALID;  // Request message from panel invalid
        } else {
          this->uart_state_ = desk_raiser_box::UART_STATE_RECEIVED_VALID;  // Request message from panel is valid
        }
        this->last_request_timestamp_ = now;
        this->flush();
        break;
    }

    if (this->uart_state_ == desk_raiser_box::UART_STATE_RECEIVED_VALID) {
      // TODO: Decode request - parse command
      // 0xA5 0x00 0x00 0x01 0x01    Idle/Get current display status
      // 0xA5 0x00 0x20 0x01 0x21    Move up
      // 0xA5 0x00 0x40 0x01 0x41    Move down
      // 0xA5 0x00 0x60 0x01 0x61    UP and Down (used to reset)
      // 0xA5 0x00 0x01 0x01 0x02    M button
      // 0xA5 0x00 0x02 0x01 0x03    memory 1
      // 0xA5 0x00 0x04 0x01 0x05    memory 2
      // 0xA5 0x00 0x08 0x01 0x09    memory 3
      // 0xA5 0x00 0x10 0x01 0x11    T button
      // 0xA5 0x00 0x11 0x01 0x12    M+T (to get into settings)

      // Restart reading message
      this->uart_state_ = desk_raiser_box::UART_STATE_READY;
      this->rx_data_.clear();

      this->send_response("1.2.3");
      break; // while available
    } else if (this->uart_state_ == desk_raiser_box::UART_STATE_RECEIVED_INVALID) {
      // Restart reading message
      this->uart_state_ = desk_raiser_box::UART_STATE_READY;
      this->rx_data_.clear();
      break; // while available
    }
  }

  // if(UART_bytes_available || ((loop_number % 1000) == 0)) {
  //   ESP_LOGV(TAG, "(loop %i - %ims) UART bytes available: %i", loop_number, (int)(now-last_loop_timestamp), UART_bytes_available);
  // }

  last_loop_timestamp = now;
}

void DeskRaiserPanel::dump_config() {
  ESP_LOGCONFIG(TAG, "Desk raiser control panel");
  // ESP_LOGCONFIG(TAG, "  foo = %s", TRUEFALSE(this->foo_));
  // ESP_LOGCONFIG(TAG, "  bar = %s", this->bar_.c_str());
  // ESP_LOGCONFIG(TAG, "  baz = %i", this->baz_);
}

void DeskRaiserPanel::send_response(std::string response) {
  std::vector<uint8_t> data = {};

  if (this->uart_state_ != desk_raiser_box::UART_STATE_READY) {
    ESP_LOGW(TAG, "Cannot send response. Device state: %d", this->uart_state_);
    return;
  }

  bool last_char_complete{false};
  for (auto &ch: response) {
    if(last_char_complete && ch == '.') {
      uint8_t last_byte = data[data.size()-1];
      data.pop_back();
      data.push_back(last_byte | 0x80); // add dot
      last_char_complete = false;
    } else {
      if(data.size() > 3) {
        ESP_LOGW(TAG, "Response must be at most 3 characters");
        break; // prevent adding more than 3 chracters
      }
      data.push_back(char_to_panel(ch));
      last_char_complete = true;
    }
  }

  this->uart_state_ = desk_raiser_box::UART_STATE_SENDING;
  data.push_back((this->screen_on_ ? 0x10 : 0x00) | (this->timer_indicator_on_ ? 0x01 : 0x00));

  uint8_t checksum = 0x00;
  for (auto i : data) {
    checksum = (checksum + i) & 0xFF;
  }

  data.push_back(checksum);
  data.insert(data.begin(), 0x5A);

  // ESP_LOGV(TAG, "Sending %i bytes as response", data.size());

  this->write_array(data);
  this->flush();

  this->uart_state_ = desk_raiser_box::UART_STATE_READY;
  this->last_response_timestamp_ = millis();
}

}  // namespace desk_raiser_panel
}  // namespace esphome
