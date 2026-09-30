// Credits: https://github.com/decrazyo/fdxb
#ifndef EM4095_H
#define EM4095_H

#include "Arduino.h"
#include "Decoder.h"
#include "util.h"
#include <stdint.h>

class EM4095 {
public:
  EM4095() = default;
  ~EM4095() = default;

  inline void putStateChange() { putStateChange(micros()); }
  void putStateChange(uint32_t time);
  bool getTag(tag_t *tag);

private:
  Decoder mDecoder;

  bool findHeader();
  bool getByte(uint8_t *value);
  uint16_t crc16k(uint16_t crc, uint8_t *mem, uint8_t len);
  bool getData(uint8_t *data);
};

#endif
