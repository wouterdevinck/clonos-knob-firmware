/*
  UsbDevice.h
  Created by Wouter Devinck, May 22, 2016.
*/
#ifndef UsbDevice_h
#define UsbDevice_h

#include "Arduino.h"
#include "SPI.h"
#include "constants.h"

class UsbDevice {
  public:
    UsbDevice(int ssPin, int intPin);
    void testSPI();
    void begin();
    void loop();
  private:
    int _ssPin;
    int _intPin;
    byte _suspended;
    byte _rwuEnabled;
    byte _configval;
    byte _sud[8];
    byte _inhibitSend;
    byte _send3zeros;
    byte _msgidx;
    byte _msglen;
    byte _ep3stall;
    void reset();
    void checkForResume();
    void serviceIRQs();
    void doSetup();
    void doIN3();
    void stdRequest();
    void getStatus();
    void setInterface();
    void getInterface();
    void getConfiguration();
    void setConfiguration();
    void feature(byte sc);
    void sendDescriptor();
    void classRequest();
    void vendorRequest();
    void wreg(byte address, byte value);
    void wregAS(byte address, byte value);
    byte rreg(byte address);
    byte rregAS(byte address);
    void readbytes(byte address, byte count, byte *pointer);
    void writebytes(byte address, byte count, byte *pointer);
};

#endif
