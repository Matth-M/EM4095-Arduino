#include <EM4095Reader.h> // https://github.com/Matth-M/EM4095-Arduino

#define EN_SHD 17   // Enable pin
#define DMOD 18     // Data from EM4095 reader
#define RDY_CLK 19  // Frequency signal from EM4095 reader

EM4095Reader parser;
uint32_t lastFrequencyRead = 0;
const uint32_t frequencyReadDelayUs = 2000000; // 2 second

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

    // Read frequency every 2 second
    // Avoid using delay
    if(micros() - lastFrequencyRead > frequencyReadDelayUs) {
        lastFrequencyRead = micros();
        // This uses a delay, no tags can be detected during its execution.
        double frequency = parser.antennaFrequency();
        Serial.print("frequency: ");
        Serial.println(frequency);
    }
}
