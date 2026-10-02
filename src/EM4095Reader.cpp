// Credits: https://github.com/decrazyo/fdxb
#include "EM4095Reader.h"
#include "Arduino.h"
#include "util.h"
#include <stdio.h>

void EM4095Reader::begin(int enShdPin, int dmodPin, int clkPin) {
  _dmodPin = dmodPin;
  _enShdPin = enShdPin;
  _clkPin = clkPin;
  pinMode(_enShdPin, OUTPUT);
  pinMode(_dmodPin, INPUT);
  pinMode(_clkPin, INPUT);
  enableReader();
}

void EM4095Reader::putStateChange(uint32_t time) {
  _decoder.putStateChange(time);
}

bool EM4095Reader::getTag(tag_t *tag) {
  _decoder.parseTimestamps();
  return getData((uint8_t *)tag);
}

bool EM4095Reader::findHeader() {
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

bool EM4095Reader::getByte(uint8_t *value) {
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
uint16_t EM4095Reader::crc16k(uint16_t crc, uint8_t *mem, uint8_t len) {
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

bool EM4095Reader::getData(uint8_t *data) {
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

void EM4095Reader::dmod_change() { this->putStateChange(); }

void EM4095Reader::enableReader() { digitalWrite(_enShdPin, LOW); }

void EM4095Reader::disableReader() { digitalWrite(_enShdPin, HIGH); }

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

double EM4095Reader::antennaFrequency() {
  clk_first_period = true;
  clk_period_count = 0;
  attachInterrupt(digitalPinToInterrupt(_clkPin), rdy_clk_rising, RISING);
  delay(100);
  detachInterrupt(digitalPinToInterrupt(_clkPin));

  double elapsed_time_us = clk_now - clk_start_us;
  double frequency = 1000000.0 / (elapsed_time_us / clk_period_count);
  return frequency;
}

void EM4095Reader::printTag(tag_t *tag) {
  const size_t bufSize = 256;
  char buffer[bufSize];
  parseTag(tag, buffer, bufSize);
  Serial.println(buffer);
}

void EM4095Reader::parseTag(tag_t *tag, char buf[], size_t len) {
  // printf cannot format integers larger then 32-bits.
  // The id field is 38-bits so we need to do some additional processing to it.
  uint32_t idMsd = tag->id / 1000000000;
  uint32_t idLsd = tag->id % 1000000000;
  uint32_t data = tag->data[0] << 16 | tag->data[1] << 8 | tag->data[2];

  // This is the format that pet microchip lookup websites expect.
  snprintf(buf, len, "country: %03u\tid: %03u%09u\tdata: %u",
           (uint16_t)tag->country, idMsd, idLsd, data);
}
