#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>

#define PACKET_SIZE 128 // bits
// ISO 11784/11785 RFID tag carrier frequency 134.2kHz.
#define FDXB_CARRIER_HZ 132400
#define FDXB_BUFFER_SIZE                                                       \
  (8 * PACKET_SIZE) // a tag sends 128 of data, can contain a few tag message
#define TIMESTAMPS_BUF_SIZE 1024

#pragma pack(1)
typedef struct Tag {
  uint64_t id : 38;
  uint64_t country : 10;
  uint64_t flags : 16; // 2 flag bits, 14 reserved bits
  uint16_t checksum;   // crc16
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

#endif
