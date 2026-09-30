#ifndef FIFO_H
#define FIFO_H
#include "./EM4095.h"
#include "util.h"
#include <stdint.h>

// C: capacity
template <typename T, size_t C>
class Fifo {
  // Simple first in, first out (FIFO) ring buffer.
public:
  Fifo()=default;
  ~Fifo()=default;

  void push(T data) {
    mBuffer[mWrite] = data;
    next(&mWrite);
    if (empty()) {
      next(&mRead);
    }
  }

  bool pop(T *data) {
    bool status = peek(data);
    if (status) {
      next(&mRead);
    }
    return status;
  }

  bool peek(T *value) {
    if (empty()) {
      return false;
    }
    *value = mBuffer[mRead];
    return true;
  }
  size_t length() {
    // Buffer grows downwards
    // mRead is inferior to mWrite, it means the write crossed the
    // end of the buffer and has been reset to mSize - 1.

    if (mRead < mWrite) {
      return mSize - mWrite + mRead;
    } else {
      return mRead - mWrite;
    }
  }
  bool empty() { return mRead == mWrite; }

private:
  size_t mSize = C;
  T mBuffer[C] = {};
  size_t mRead = 0;
  size_t mWrite = 0;

  inline void next(size_t *index) {
    if (*index) {
      (*index)--;
    } else {
      *index = mSize - 1;
    }
  }
};
#endif
