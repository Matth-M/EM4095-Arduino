#include <EM4095Reader.h> // https://github.com/Matth-M/EM4095-Arduino

#define EN_SHD 17   // Enable pin
#define DMOD 18     // Data from EM4095 reader
#define RDY_CLK 19  // Frequency signal from EM4095 reader

EM4095Reader parser;

void dmodChangeISR() {
    parser.putStateChange();
}

void setup() {
    parser.begin(EN_SHD, DMOD, RDY_CLK);
    // Required to handle data from reader
    attachInterrupt(digitalPinToInterrupt(DMOD), dmodChangeISR, CHANGE);
}

void loop() {
    tag_t tag;
    if (parser.getTag(&tag)) {
      parser.printTag(&tag);
    }
}
