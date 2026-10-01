// Credits: https://github.com/decrazyo/fdxb
#include "Arduino.h"
#include "EM4095.h"

Decoder::Decoder(uint32_t carrierHz) {
  // RFID tags use the carrier frequency as a clock signal.
  // That causes the data rate to be a function of the carrier frequency.
  // Determine the period of one clock cycle in microseconds.
  float carrierPeriod = 1 / (carrierHz / 1000000.0);
  _baud = carrierPeriod * 16;
  _shortBefore = false;
}

void Decoder::putStateChange(uint32_t timeUs) {
  // https://github.com/espressif/arduino-esp32/issues/3697#issuecomment-580715641
  // If ISR runs for more than 300us -> WDT trigger
  mTimestamps.push(timeUs);
}

void Decoder::parseTimestamps() {
  // For each pair of timestamps, check if it's a long or short period
  // https://www.priority1design.com.au/fdx-b_animal_identification_protocol.html
  // A long period corresponds to a logical 1, and 2 consecutive short period to
  // logical 0 In case of 00 (short -> short -> short -> short), we need to keep
  // track of the first time a short period of a logical 0

  size_t timestampsCount = mTimestamps.length();
  if (timestampsCount < 2) {
    // Need at least 2 timestamps to have a period
    return;
  }
  uint32_t beforeUs = 0, afterUs = 0;
  uint32_t delta;
  rfdata_t data;
  // Check until last timestamps, keep it for next call
  for (size_t i = 0; i < timestampsCount - 1; i++) {
    mTimestamps.pop(&beforeUs);
    mTimestamps.peek(&afterUs);
    delta = afterUs - beforeUs;

    uint8_t periodCount = (uint8_t)round(delta / _baud);
    switch (periodCount) {
    case 1:
      if (_shortBefore) {
        _shortBefore = false;
        data.data = 0;
        data.valid = true;
        mRFData.push(data);
        break;
      }
      if (!_shortBefore) {
        _shortBefore = true;
        break;
      }
    case 2:
      data.data = 1;
      data.valid = true;
      mRFData.push(data);
      break;
    default:
      _shortBefore = false;
      data.data = 0;
      data.valid = false;
      mRFData.push(data);
      break;
    }
  }
}

size_t Decoder::length() { return mRFData.length(); }

bool Decoder::getBit(uint8_t *value) {
  rfdata_t data;
  bool status = mRFData.pop(&data);
  if (!status || !data.valid) {
    return false;
  }
  *value = data.data;
  return true;
}
