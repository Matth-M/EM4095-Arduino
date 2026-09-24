#ifndef FIFO_H
#define FIFO_H
#include "./EM4095.h"
#include "util.h"
#include <stdint.h>

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
#endif
