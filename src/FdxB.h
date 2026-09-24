// Credits: https://github.com/decrazyo/fdxb
#ifndef FDXB_H
#define FDXB_H

#include <Arduino.h>

// ISO 11784/11785 RFID tag carrier frequency 134.2kHz.
// 134.454kHz is as close as Arduino PWM can achieve.
#ifndef FDXB_CARRIER_HZ
#define FDXB_CARRIER_HZ 132400
#endif


#ifndef FDXB_BUFFER_SIZE
#define FDXB_BUFFER_SIZE 1024 // a tag sends 128 of data, can contain a few tag message
#endif

namespace FdxB {
#pragma pack(1)
typedef struct Tag {
  uint64_t id : 38;
  uint64_t country : 10;
  uint64_t flags : 16;  // 2 flag bits, 14 reserved bits
  uint16_t checksum;    // crc16
  uint8_t data[3];
} tag_t;

typedef enum Flag {
  DATA = (1 << 0),
  APPLICATION = (1 << 15),
} flag_t;

typedef struct RFData {
  bool valid;
  uint8_t data;
} rfdata_t;

class Fifo {
  // Simple first in, first out (FIFO) ring buffer.
public:
  Fifo(size_t size = FDXB_BUFFER_SIZE);
  ~Fifo();

  void push(rfdata_t value);
  bool pop(rfdata_t *value);
  bool peek(rfdata_t *value);
  size_t length();
  bool empty();

private:
	size_t mSize;
  rfdata_t *mBuffer;
  uint8_t mRead = 0;
  uint8_t mWrite = 0;

  inline void next(uint8_t *index);
};

class Decoder {
public:
  Decoder(uint32_t carrierHz = FDXB_CARRIER_HZ,
          size_t bufferSize = FDXB_BUFFER_SIZE);
  ~Decoder();

  inline void putStateChange() {
    putStateChange(micros());
  }
  void putStateChange(uint16_t timeUs);
  bool getBit(uint8_t *value);
  uint8_t length();

private:
  Fifo *mRFData;
  uint16_t mLastStateChangeUs;
  float mBaud;
  bool getDelta(uint8_t *value);
};

class Parser {
public:
  Parser(uint32_t carrierHz = FDXB_CARRIER_HZ,
         size_t bufferSize = FDXB_BUFFER_SIZE);
  ~Parser();

  inline void putStateChange() {
    putStateChange(micros());
  }
  void putStateChange(uint16_t time);
  bool getTag(tag_t *tag);

private:
  Decoder *mDecoder;

  bool findHeader();
  bool getByte(uint8_t *value);
  uint16_t crc16k(uint16_t crc, uint8_t *mem, uint8_t len);
  bool getData(uint8_t *data);
  size_t antennaIndex;
};
}

#endif
