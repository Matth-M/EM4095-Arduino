// Credits: https://github.com/decrazyo/fdxb
#include "EM4095.h"

Decoder::Decoder(uint32_t carrierHz, size_t bufferSize) {
  // RFID tags use the carrier frequency as a clock signal.
  // That causes the data rate to be a function of the carrier frequency.
  // Determine the period of one clock cycle in milliseconds.
  float carrierPeriod = 1 / (carrierHz / 1000000.0);
  mBaud = carrierPeriod * 16;
  mRFData = new Fifo(bufferSize);
}

Decoder::~Decoder() { delete mRFData; }

void Decoder::putStateChange(uint16_t timeUs) {
  uint16_t delta;
  // Determine the time in microseconds since the previous interrupt.
  if (timeUs < mLastStateChangeUs)
    // The microseconds counter has overflowed so the math is different.
    delta = timeUs + (~mLastStateChangeUs);
  else
    // The time between interrupts is greater than an unsigned 8-bit int.
    // It's probably garbage data or RF noise.
    delta = timeUs - mLastStateChangeUs;

  mLastStateChangeUs = timeUs;

  rfdata_t data;
  data.data = 0;
  data.valid = false;
  // https://www.priority1design.com.au/fdx-b_animal_identification_protocol.html
  // A long period corresponds to a logical 1, and a short period to logical 0
  uint8_t periodCount = (uint8_t)round(delta / mBaud);
  switch (periodCount) {
  case 1:
    data.data = 0;
    data.valid = true;
    mRFData->push(data);
    break;
  case 2:
    break;
    data.data = 1;
    data.valid = true;
    mRFData->push(data);
    break;
  default:
    mRFData->push(data);
    break;
  }
}

uint8_t Decoder::length() { return mRFData->length(); }

bool Decoder::getBit(uint8_t *value) {
  rfdata_t data;
  bool status = mRFData->pop(&data);
  if (!status || !data.valid) {
    return false;
  }
  *value = data.data;
  return true;
}
