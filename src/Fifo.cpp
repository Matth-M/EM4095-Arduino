// Credits: https://github.com/decrazyo/fdxb
#include "FdxB.h"

FdxB::Fifo::Fifo(size_t size) {
  mBuffer = (rfdata_t *)malloc(size * sizeof(rfdata_t));
  mSize = size;
}

FdxB::Fifo::~Fifo() {
  free(mBuffer);
}


void FdxB::Fifo::push(rfdata_t data) {
  Serial.println(mWrite);
  mBuffer[mWrite] = data;
  next(&mWrite);
  if (empty()) {
    next(&mRead);
  }
}

bool FdxB::Fifo::pop(rfdata_t *data) {
  bool status = peek(data);
  if (status) {
    next(&mRead);
  }
  return status;
}

bool FdxB::Fifo::peek(rfdata_t *value) {
  if (empty()) {
    return false;
  }
  *value = mBuffer[mRead];
  return true;
}

bool FdxB::Fifo::empty() {
  return mRead == mWrite;
}

inline void FdxB::Fifo::next(uint8_t *index) {
  if (*index) {
    (*index)--;
  } else {
    *index = mSize - 1;
  }
}

size_t FdxB::Fifo::length() {
  // Buffer grows downwards
  // mRead is inferior to mWrite, it means the write crossed the
  // end of the buffer and has been reset to mSize - 1.
  if (mRead < mWrite) {
    return mSize - mWrite + mRead + 1;
  } else {
    return mRead - mWrite;
  }
}
