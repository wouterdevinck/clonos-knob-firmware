#include <SPI.h>
#include "UsbDevice.h"

const int ssPin = 53;
const int intPin = 49;
UsbDevice usb(ssPin, intPin);

void setup() {
  Serial.begin(9600);
  usb.begin();
}

void loop() {
  //usb.testSPI();
  //delay(2000);
  usb.loop();
}



