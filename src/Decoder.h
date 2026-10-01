#ifndef DECODER_H
#define DECODER_H
#include "./EM4095.h"
#include "./Fifo.h"
#include "Arduino.h"
#include "util.h"
#include <stdint.h>

class Decoder {
public:
  Decoder(uint32_t carrierHz = FDXB_CARRIER_HZ);

  inline void putStateChange() { putStateChange(micros()); }
  void parseTimestamps();
  void putStateChange(uint32_t timeUs);
  bool getBit(uint8_t *value);
  size_t length();

private:
  Fifo<rfdata_t, FDXB_BUFFER_SIZE> mRFData;
  Fifo<uint32_t, TIMESTAMPS_BUF_SIZE> mTimestamps;
  float _baud;
  bool _shortBefore;
};
#endif
