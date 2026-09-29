// Credits: https://github.com/decrazyo/fdxb
#include "Arduino.h"
#include "EM4095.h"

Decoder::Decoder(uint32_t carrierHz) {
  // RFID tags use the carrier frequency as a clock signal.
  // That causes the data rate to be a function of the carrier frequency.
  // Determine the period of one clock cycle in microseconds.
  float carrierPeriod = 1 / (carrierHz / 1000000.0);
  mBaud = carrierPeriod * 16;
  mShortBefore = false;
	mLastStateChangeUs = 0;
}

void Decoder::putStateChange(uint32_t timeUs) {
	// https://github.com/espressif/arduino-esp32/issues/3697#issuecomment-580715641
	// If ISR runs for more than 300us -> WDT trigger

	mTimestamps.push(timeUs);
}

void Decoder::parseTimestamps(){
	uint32_t timeUs;
	for(int i =0; i< mTimestamps.length(); i++) {
		mTimestamps.pop(&timeUs);

	}
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
  // https://www.priority1design.com.au/fdx-b_animal_identification_protocol.html
  // A long period corresponds to a logical 1, and a short period to logical 0
  uint8_t periodCount = (uint8_t)round(delta / mBaud);

  // data.data = 0;
  // data.valid = true;
  // mRFData.push(data);
  // return;

  // uint8_t periodCount = 2;
  switch (periodCount) {
  case 1:
    if (mShortBefore) {
      mShortBefore = false;
      data.data = 0;
      data.valid = true;
      mRFData.push(data);
      break;
    }
    if (!mShortBefore) {
      mShortBefore = true;
      break;
    }
  case 2:
    break;
    data.data = 1;
    data.valid = true;
    mRFData.push(data);
    break;
  default:
		mShortBefore = false;
    data.data = 0;
    data.valid = false;
    mRFData.push(data);
    break;
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
