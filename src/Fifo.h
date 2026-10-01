#ifndef FIFO_H
#define FIFO_H
#include "./EM4095.h"
#include "util.h"
#include <stdint.h>

// C: capacity
template <typename T, size_t C> class Fifo {
  // Simple first in, first out (FIFO) ring buffer.
public:
  Fifo() = default;
  ~Fifo() = default;

  void push(T data) {
    mBuffer[_write] = data;
    next(&_write);
    if (empty()) {
      next(&_read);
    }
  }

  bool pop(T *data) {
    bool status = peek(data);
    if (status) {
      next(&_read);
    }
    return status;
  }

  bool peek(T *value) {
    if (empty()) {
      return false;
    }
    *value = mBuffer[_read];
    return true;
  }
  size_t length() {
    // Buffer grows downwards
    // _read is inferior to _write, it means the write crossed the
    // end of the buffer and has been reset to _size - 1.

    if (_read < _write) {
      return _size - _write + _read;
    } else {
      return _read - _write;
    }
  }
  bool empty() { return _read == _write; }

private:
  size_t _size = C;
  T mBuffer[C] = {};
  size_t _read = 0;
  size_t _write = 0;

  inline void next(size_t *index) {
    if (*index) {
      (*index)--;
    } else {
      *index = _size - 1;
    }
  }
};
#endif
