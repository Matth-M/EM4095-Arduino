#ifndef DECODER_H
#define DECODER_H
#include "./EM4095.h"
#include "./Fifo.h"
#include "util.h"
#include "Arduino.h"
#include <stdint.h>

class Decoder {
public:
  Decoder(uint32_t carrierHz = FDXB_CARRIER_HZ,
          size_t bufferSize = FDXB_BUFFER_SIZE);
  ~Decoder();

  inline void putStateChange() { putStateChange(micros()); }
  void putStateChange(uint16_t timeUs);
  bool getBit(uint8_t *value);
  uint8_t length();

private:
  Fifo *mRFData;
  uint16_t mLastStateChangeUs;
  float mBaud;
  bool getDelta(uint8_t *value);
};
#endif
