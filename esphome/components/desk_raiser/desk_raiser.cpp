#include "esphome/core/log.h"
#include "desk_raiser.h"

namespace esphome {
namespace desk_raiser {

static const char *TAG = "desk_raiser.component";

void DeskRaiserUART::loop() {
  if (this->uart_state_ == UART_STATE_NOT_READY)
    return;

  if (this->uart_state_ == UART_STATE_SENDING)
    return;

  while (this->uart_->available()) {
    uint8_t c;
    this->uart_->read_byte(&c);

    switch (this->uart_state_) {
      case UART_STATE_READY:
        if (c == this->signature_byte_) {
          // Response has begun
          this->uart_state_ = UART_STATE_RECEIVING;
        }
        break;
      case UART_STATE_RECEIVING:
        // Append a byte to response
        this->rx_data_.push_back(c);
        if (this->rx_data_.size() < this->message_size_)
          continue;  // read another byte

        uint8_t sum;
        for (int i = 0; i < this->message_size_ - 1; i++) {
          sum += this->rx_data_[i];
        }
        sum &= 0xFF;

        this->last_response_timestamp_ = millis();
        if (sum == this->rx_data_[this->message_size_ - 1]) {
          this->uart_state_ = UART_STATE_RECEIVED_VALID;
        } else {
          ESP_LOGW(TAG, "Invalid checksum: %02x != %02x", sum, this->rx_data_[this->message_size_ - 1]);
          this->uart_state_ = UART_STATE_RECEIVED_INVALID;
        }
        break;
    }
  }
}

void DeskRaiserUART::restart() {
  this->uart_state_ = UART_STATE_READY;
  this->rx_data_.clear();
}

void DeskRaiserUART::send_message(std::vector<uint8_t> bytes) {
  if (this->uart_state_ != UART_STATE_READY) {
    ESP_LOGE(TAG, "Unable to send message via UART. Current UART state: %i", this->uart_state_);
    return;
  }

  this->uart_state_ = UART_STATE_SENDING;

  std::vector<uint8_t> message{};

  message.push_back(this->signature_byte_);
  uint8_t csum = 0x00;
  for (int i = 0; i < bytes.size(); i++) {
    csum += bytes[i];
    message.push_back(bytes[i]);
  }

  message.push_back(csum & 0xFF);

  // this->uart_->write_array(message);
  // this->uart_->flush();
  this->last_request_timestamp_ = millis();
  this->uart_state_ = UART_STATE_READY;
}

void DeskRaiserControllerUART::loop() {
  uint64_t now = millis();

  if ((this->uart_state_ == UART_STATE_READY) && ((now - this->last_request_timestamp_) > 14)) {  // at most every 14ms
    // Time to send a message to the controller
    this->send_message(interaction_bytes(INTERACTION_STATUS));
  }

  // DeskRaiserUART::loop();
}

bool DeskRaiserControllerUART::decode_valid_response() {
  if (this->uart_state_ == UART_STATE_RECEIVED_VALID) {
    // TODO: Decode response
    // TODO: Check 4th byte
    // 0x00	Everything on the display is completely off
    // 0x01	Timer indicator is turned on
    // 0x10	7 segment display is on (3 characters + decimal point)
    // 0x11	7 segment display is on as well as the timer indicator

    // ESP_LOGV(TAG, "Received response: %X %X %X %X %X", this->rx_data_[0], this->rx_data_[1], this->rx_data_[2],
    // this->rx_data_[3], this->rx_data_[4]);

    if (this->rx_data_[3] & 0x10) {
      std::string response("");
      for (int i = 0; i < 3; i++) {
        response += box_to_char(this->rx_data_[i]);
        if (this->rx_data_[i] & 0x80)
          response += '.';
      }

      uint64_t now = millis();

      if ((this->last_response_ != response) || ((now - this->last_valid_response_timestamp_) > 5000)) {
        this->last_response_ = response;
        this->last_valid_response_timestamp_ = now;
        ESP_LOGV(TAG, "New response received: %s (screen %x)", response.c_str(), this->rx_data_[3]);

        this->restart();
        return true;
      }
    }

    // Restart reading message
    this->restart();
  }

  return false;
}

bool DeskRaiserPanelUART::decode_valid_response() {
  if (this->uart_state_ == UART_STATE_RECEIVED_VALID) {
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

    if ((this->rx_data_[0] == 0x00) && (this->rx_data_[2] == 0x01)) {
      DeskRaiserInteraction response = INTERACTION_STATUS;
      switch(this->rx_data_[1]) {
        case 0x00:
          response = INTERACTION_STATUS;
          break;
        case 0x20:
          response = INTERACTION_UP;
          break;
        case 0x40:
          response = INTERACTION_DN;
          break;
        case 0x60:
          response = INTERACTION_UP_DN;
          break;
        case 0x01:
          response = INTERACTION_M;
          break;
        case 0x02:
          response = INTERACTION_MEM_1;
          break;
        case 0x04:
          response = INTERACTION_MEM_2;
          break;
        case 0x08:
          response = INTERACTION_MEM_3;
          break;
        case 0x10:
          response = INTERACTION_T;
          break;
        case 0x11:
          response = INTERACTION_M_T;
          break;
      }

      uint64_t now = millis();

      if ((this->last_response_ != response) || ((now - this->last_valid_response_timestamp_) > 5000)) {
        this->last_response_ = response;
        this->last_valid_response_timestamp_ = now;
        ESP_LOGV(TAG, "New response received: %i", response);

        this->restart();
        return true;
      }
    }

    // Restart reading message
    this->restart();
  }

  return false;
}

void DeskRaiserPanelUART::send_text(std::string text) {
  std::vector<uint8_t> data = {};

  bool last_char_complete{false};
  for (auto &ch: text) {
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

  data.push_back(0x10); // screen on

  this->send_message(data);
}

void DeskRaiser::setup() {
  // Code here should perform all component initialization,
  //  whether hardware, memory, or otherwise
}

void DeskRaiser::loop() {
  // Tasks here will be performed at every call of the main application loop.
  // Note: code here MUST NOT BLOCK (see below)

  this->current_loop_timestamp_ = millis();

  this->controller_->loop();

  // switch (this->controller_->get_uart_state()) {
  //   case UART_STATE_RECEIVED_INVALID:
  //     this->controller_->restart();
  //     break;

  //   case UART_STATE_RECEIVED_VALID:
  //     if (this->controller_->decode_valid_response()) {
  //       this->current_height_ = std::stof(this->controller_->get_last_response());
  //     }
  //     break;
  // }

  // if (this->panel_ == nullptr)
  //   return;

  // this->panel_->loop();

  // switch (this->panel_->get_uart_state()) {
  //   case UART_STATE_RECEIVED_INVALID:
  //     this->panel_->restart();
  //     break;

  //   case UART_STATE_RECEIVED_VALID:
  //     if (this->panel_->decode_valid_response()) {
  //       this->panel_->send_text("---");
  //     }
  //     break;
  // }
}

void DeskRaiser::dump_config() { ESP_LOGCONFIG(TAG, "Desk raiser control box"); }

char box_to_char(uint8_t c) {
  // Converts 7-segment symbol to a char

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

std::vector<uint8_t> interaction_bytes(DeskRaiserInteraction interaction) {
  std::vector<uint8_t> data = {};

  switch (interaction) {
    case INTERACTION_STATUS:
      data.push_back(0x00);
      data.push_back(0x00);
      break;
    case INTERACTION_UP:
      data.push_back(0x00);
      data.push_back(0x20);
      break;
    case INTERACTION_DN:
      data.push_back(0x00);
      data.push_back(0x40);
      break;
    case INTERACTION_UP_DN:
      data.push_back(0x00);
      data.push_back(0x60);
      break;
    case INTERACTION_M:
      data.push_back(0x00);
      data.push_back(0x01);
      break;
    case INTERACTION_MEM_1:
      data.push_back(0x00);
      data.push_back(0x02);
      break;
    case INTERACTION_MEM_2:
      data.push_back(0x00);
      data.push_back(0x04);
      break;
    case INTERACTION_MEM_3:
      data.push_back(0x00);
      data.push_back(0x08);
      break;
    case INTERACTION_T:
      data.push_back(0x00);
      data.push_back(0x10);
      break;
    case INTERACTION_M_T:
      data.push_back(0x00);
      data.push_back(0x11);
      break;
    default:
      ESP_LOGE(TAG, "Unknown interaction provided: %i", interaction);
      return std::vector<uint8_t>{};
  }
  data.push_back(0x01);

  return data;
}

}  // namespace desk_raiser
}  // namespace esphome
