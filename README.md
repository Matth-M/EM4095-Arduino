Fork of [fdxb](https://github.com/decrazyo/fdxb). Read RFID tags (like [Biomark APT12](https://www.merck-animal-health-usa.com/downloads/merck-2021-product-apt12-050521-pdf/)) with [EM4095](https://www.emmicroelectronic.com/index.php/product/rf-reader-ics/em4095) in non blocking mode (i.e. without staying in while loop until a complete tag is found).

# Usage
```cpp
#define EN_SHD 17   // Enable pin
#define DMOD 18     // Data from EM4095 reader
#define RDY_CLK 19  // Frequency signal from EM4095 reader
EM4095 parser;

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
```
