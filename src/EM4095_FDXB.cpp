// Credits: https://github.com/decrazyo/fdxb
#include "EM4095_FDXB.h"
#include "Arduino.h"
#include "util.h"


void EM4095_FDXB::begin(int enShdPin, int dmodPin, int clkPin) {
  _dmodPin = dmodPin;
  _enShdPin = enShdPin;
  _clkPin = clkPin;
  pinMode(_enShdPin, OUTPUT);
  pinMode(_dmodPin, INPUT);
  pinMode(_clkPin, INPUT);
	enableReader();
}

void EM4095_FDXB::putStateChange(uint32_t time) { _decoder.putStateChange(time); }

bool EM4095_FDXB::getTag(tag_t *tag) {
  _decoder.parseTimestamps();
  return getData((uint8_t *)tag);
}

bool EM4095_FDXB::findHeader() {
  uint8_t bit;
  uint8_t count = 0;
  if (_decoder.length() < PACKET_SIZE) {
    return false;
  }

  // The header starts with 10 zeros and ends with a one.
	// If no header is found, break
  while (true) {
    if (!_decoder.getBit(&bit)) {
      return false;
      // bit read failed
      count = 0;
    } else if (bit) { // Found 1
      if (count >= 10) {
        // header identified
        break;
      } else {
        // expected another 0, got 1
        count = 0;
        return false;
      }
    } else if (count < 10) { // Found 0
      count++;
    }
  }
  return true;
}

bool EM4095_FDXB::getByte(uint8_t *value) {
  uint8_t bit;

  *value = 0;

  for (uint8_t i = 0; i < 8; i++) {
    if (!_decoder.getBit(&bit)) {
      // failed bit read
      return false;
    }
    *value |= bit << i;
  }

  if (!_decoder.getBit(&bit)) {
    // failed control bit read
    return false;
  } else if (bit) {
    // valid control bit
    return true;
  } else {
    // invalid control bit
    return false;
  }
}

// https://stackoverflow.com/questions/57893261
uint16_t EM4095_FDXB::crc16k(uint16_t crc, uint8_t *mem, uint8_t len) {
  uint8_t *data = mem;

  if (data == NULL) {
    return 0;
  }

  while (len--) {
    crc ^= *data++;
    for (uint8_t k = 0; k < 8; k++)
      crc = crc & 1 ? (crc >> 1) ^ 0x8408 : crc >> 1;
  }

  return crc;
}


bool EM4095_FDXB::getData(uint8_t *data) {
  // Block until we identify a valid tag header.
  if (!findHeader()) {
    return false;
  }

  tag_t *tag = (tag_t *)data;

  // Get the first 10 byte which should always exist in the tag.
  for (uint8_t i = 0; i < 10; i++) {
    if (!getByte(&data[i])) {
      // read bad data
      return false;
    }
  }

  // Get an extra 3 bytes if the tag says they exist.
  if (tag->flags & DATA) {
    for (uint8_t i = 10; i < 13; i++) {
      if (!getByte(&data[i])) {
        return false;
      }
    }
  }

  // verify checksum
  return crc16k(0, data, 8) == tag->checksum;
}

void EM4095_FDXB::dmod_change() { this->putStateChange(); }

void EM4095_FDXB::enableReader() {
  // EM4095_FDXB pin is enabled by SHD pin. When 5V is applied to this pin, the
  // reader is disabled, and when 0V(GND) is applied, the reader is enabled. The
  // ESP32 is working with 3V3 and cannot output 5V. On gate v6, a NMOS is used
  // to connect SHD to GND when EN_SHD signal is HIGH (in the MCU POV, so 3V3)
  // which enables the reader and a pull-up resistor to 5V to disable the reader
  // when EN_SHD is LOW.
  digitalWrite(_enShdPin, HIGH);
}

void EM4095_FDXB::disableReader() { digitalWrite(_enShdPin, LOW); }

// To measure frequency, count the number of period (rising interrupts)
// in a given time. We need to know when is the first period
// to get the correct duration.
volatile bool clk_first_period = true;
volatile uint32_t clk_now = 0;
volatile uint32_t clk_start_us = 0;
volatile uint32_t clk_period_count = 0;

void rdy_clk_rising() {
  if (clk_first_period) {
    clk_first_period = false;
    clk_start_us = micros();
    return;
  }
  clk_period_count++;
  clk_now = micros();
}

double EM4095_FDXB::antennaFrequency() {
  clk_first_period = true;
  clk_period_count = 0;
  attachInterrupt(digitalPinToInterrupt(_clkPin), rdy_clk_rising, RISING);
  delay(100);
  detachInterrupt(digitalPinToInterrupt(_clkPin));

  double elapsed_time_us = clk_now - clk_start_us;
  double frequency = 1000000.0 / (elapsed_time_us / clk_period_count);
  return frequency;
}

void EM4095_FDXB::printTag(tag_t* tag) {
  char buffer[256];

  // printf cannot format integers larger then 32-bits.
  // The id field is 38-bits so we need to do some additional processing to it.
  uint32_t idMsd = tag->id / 1000000000;
  uint32_t idLsd = tag->id % 1000000000;
  uint32_t data = tag->data[0] << 16 | tag->data[1] << 8 | tag->data[2];

  // This is the format that pet microchip lookup websites expect.
  sprintf(buffer, "country: %03u\tid: %03u%09u\tdata: %ld", (uint16_t)tag->country, idMsd, idLsd, data);
  Serial.println(buffer);
}
