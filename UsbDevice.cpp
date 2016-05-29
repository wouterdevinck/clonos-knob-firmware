/*
  UsbDevice.cpp
  Created by Wouter Devinck, May 22, 2016.
*/
#include "UsbDevice.h"
#include "enumdata.h"

UsbDevice::UsbDevice(int ssPin, int intPin) {
  pinMode(ssPin, OUTPUT);
  _ssPin = ssPin;
  _intPin = intPin;
}

void UsbDevice::begin() {
  // EP3 inintially un-halted (no stall) (CH9 testing)
  _ep3stall = 0;      
  _msgidx = 0;
  _msglen = sizeof(theMessage);
  _inhibitSend = 0x01;
  _send3zeros = 1;
  _configval = 0;
  _suspended = 0;
  _rwuEnabled = 0;
  // Initialize SPI
  SPI.begin();
  // Always set the FDUPSPI bit in the PINCTL register FIRST if you are using the SPI port in 
  // full duplex mode. This configures the port properly for subsequent SPI accesses.
  // MAX3420: SPI=full-duplex, INT=neg level, GPX=SOF
  wreg(rPINCTL, (bmFDUPSPI + bmINTLEVEL + gpxSOF)); 
  // Reset the MAX3420E
  reset();
  // GPIOs off (Active HIGH)
  wreg(rGPIO, 0x00);
  // This is a self-powered design, so the host could turn off Vbus while we are powered.
  // Therefore set the VBGATE bit to have the MAX3420E automatically disconnect the D+
  // pullup resistor in the absense of Vbus. Note: the VBCOMP pin must be connected to Vbus
  // or pulled high for this code to work--a low on VBCOMP will prevent USB connection.
  // VBGATE=1 disconnects D+ pullup if host turns off VBUS
  wreg(rUSBCTL, (bmCONNECT + bmVBGATE));
  // Enable IRQS
  wreg(rEPIEN, (bmSUDAVIE + bmIN3BAVIE)); 
  wreg(rUSBIEN, (bmURESIE + bmURESDNIE));
  // Enable the INT pin
  wreg(rCPUCTL, bmIE);                 
}

void UsbDevice::reset() {
  byte dum;
  // Reset the MAX3420E
  wreg(rUSBCTL, bmCHIPRES);
  // Remove the reset
  wreg(rUSBCTL, 0x00); 
  // Chip reset stops the oscillator. Wait for it to stabilize.
  do {
    delay(1); // TODO Remove?
    dum = rreg(rUSBIRQ);
    dum &= bmOSCOKIRQ;
  } while (dum == 0);
}

void UsbDevice::loop() {
  // testSPI();
  // delay(2000);
  if(_suspended) {
    checkForResume();
  }
  if (digitalRead(_intPin) == HIGH) {
    serviceIRQs();
  }
  //_inhibitSend = 0x00; 
}

void UsbDevice::checkForResume() {
  if(rreg(rUSBIRQ) & bmBUSACTIRQ) {
    // The host resumed bus traffic - no longer suspended
    //Serial.println("[DEBUG] USB: No longer suspended");
    _suspended = 0;                    
  }
} 

void UsbDevice::serviceIRQs() {
  byte itest1, itest2;
  itest1 = rreg(rEPIRQ);  // Check the EPIRQ bits
  itest2 = rreg(rUSBIRQ); // Check the USBIRQ bits
  if(itest1 & bmSUDAVIRQ) {
    wreg(rEPIRQ, bmSUDAVIRQ); // Clear the SUDAV IRQ
    doSetup();
  }
  if(itest1 & bmIN3BAVIRQ) {
    // Was an EP3-IN packet just dispatched to the host?
    // Yes - load another keystroke and arm the endpoint
    doIN3();
  }                             
  // NOTE: don't clear the IN3BAVIRQ bit here - loading the EP3-IN byte
  // count register in the doIN3() function does it.
  if((_configval != 0) && (itest2 & bmSUSPIRQ)) {   
    // HOST suspended bus for 3 msec
    // Clear the IRQ and bus activity IRQ
    wreg(rUSBIRQ, (bmSUSPIRQ + bmBUSACTIRQ)); 
    //Serial.println("[DEBUG] USB: Suspended");
    _suspended = 1;
  }
  if(rreg(rUSBIRQ) & bmURESIRQ) {
    // Clear the IRQ
    wreg(rUSBIRQ, bmURESIRQ);      
  }
  if(rreg(rUSBIRQ) & bmURESDNIRQ) {
    // Clear the IRQ
    wreg(rUSBIRQ, bmURESDNIRQ);
    _suspended = 0;
    // Enable IRQS, because a bus reset clears the IE bits
    wreg(rEPIEN, (bmSUDAVIE + bmIN3BAVIE)); 
    wreg(rUSBIEN, (bmURESIE + bmURESDNIE));
  }
} 

void UsbDevice::doSetup() {
  // Got a SETUP packet. Read 8 SETUP bytes              
  readbytes(rSUDFIFO, 8, _sud);
  // Parse the SETUP packet. For request type, look only at b6 & b5          
  switch(_sud[bmRequestType] & 0x60) {
    case 0x00: stdRequest();    break;
    case 0x20: classRequest();  break; // Just a stub in this program
    case 0x40: vendorRequest(); break; // Just a stub in this program
    default: wreg(rEPSTALLS, 0x23);    // Unrecognized request type
  }
}

void UsbDevice::doIN3() {
  if (_inhibitSend == 0x01) {
    // Send the "keys up" code
    wreg(rEP3INFIFO, 0);
    wreg(rEP3INFIFO, 0);
    wreg(rEP3INFIFO, 0);
  } else if (_send3zeros == 0x01) {
    // Precede every keycode with the "keys up" code
    wreg(rEP3INFIFO, 0);     
    wreg(rEP3INFIFO, 0);
    wreg(rEP3INFIFO, 0);
    _send3zeros = 0;
  } else {
    _send3zeros = 1;
    // Load the next keystroke (3 bytes)
    wreg(rEP3INFIFO, theMessage[_msgidx++]); 
    wreg(rEP3INFIFO, theMessage[_msgidx++]);
    wreg(rEP3INFIFO, theMessage[_msgidx++]);
    if(_msgidx >= _msglen) {
      _msgidx = 0;
      _inhibitSend = 1;
    }
  }
  // Arm it
  //wreg(rEP3INFIFO, 0x01);
  wreg(rEP3INBC, 3);
  //Serial.println("Sending mute");
}

void UsbDevice::stdRequest() {
  switch(_sud[bRequest]) {
    case SR_GET_DESCRIPTOR:    sendDescriptor();   break;
    case SR_SET_FEATURE:       feature(1);         break;
    case SR_CLEAR_FEATURE:     feature(0);         break;
    case SR_GET_STATUS:        getStatus();        break;
    case SR_SET_INTERFACE:     setInterface();     break;
    case SR_GET_INTERFACE:     getInterface();     break;
    case SR_GET_CONFIGURATION: getConfiguration(); break;
    case SR_SET_CONFIGURATION: setConfiguration(); break;
    case SR_SET_ADDRESS:       rregAS(rFNADDR);    break; // Discard return value
    default: wreg(rEPSTALLS, 0x23);
  }
}

void UsbDevice::setConfiguration() {
  // Store the config value
  _configval = _sud[wValueL];     
  // If we are configured,       
  if(_configval != 0) {       
    // Start looking for SUSPEND interrupts   
    wreg(rUSBIEN, (rreg(rUSBIEN) | bmSUSPIE)); 
  }
  // Dummy read to set the ACKSTAT bit
  rregAS(rFNADDR);
}

void UsbDevice::getConfiguration() {
  // Send the config value
  wreg(rEP0FIFO, _configval);
  wregAS(rEP0BC, 1);
}

void UsbDevice::setInterface() {
  // All we accept are Interface=0 and AlternateSetting=0, otherwise send STALL
  if((_sud[wValueL] == 0) && (_sud[wIndexL] == 0)) {    
    // wValueL: AlternateSetting index
    // wIndexL: Interface index
    // Dummy read to set the ACKSTAT bit
    rregAS(rFNADDR); 
  } else {
    wreg(rEPSTALLS, 0x23);
  }
}

void UsbDevice::getInterface() {
  // Check for Interface=0, always report AlternateSetting=0
  if(_sud[wIndexL] == 0) {
    // wIndexL: Interface index
    wreg(rEP0FIFO, 0); // AS=0
    wregAS(rEP0BC, 1); // Send one byte, ACKSTAT
  } else {
    wreg(rEPSTALLS, 0x23);
  }
}

void UsbDevice::getStatus() {
  byte testbyte;
  testbyte = _sud[bmRequestType];
  switch(testbyte) {
    case 0x80: // Directed to DEVICE
      // First byte is 000000rs where r=enabled for RWU and s=self-powered.
      wreg(rEP0FIFO, (_rwuEnabled + 1));
      // Second byte is always 0
      wreg(rEP0FIFO, 0x00);
      // Load byte count, arm the IN transfer, ACK the status stage of the CTL transfer  
      wregAS(rEP0BC, 2);
      break;        
    case 0x81: // Directed to INTERFACE
      // This one is easy: two zero bytes
      wreg(rEP0FIFO, 0x00);
      wreg(rEP0FIFO, 0x00);
      // Load byte count, arm the IN transfer, ACK the status stage of the CTL transfer    
      wregAS(rEP0BC, 2);
      break;        
    case 0x82: // Directed to ENDPOINT
      if(_sud[wIndexL] == 0x83) {
        // We only reported ep3, so it's the only one the host can stall IN3=83
        // First byte is 0000000h where h is the halt (stall) bit
        wreg(rEP0FIFO, _ep3stall);  
        // Second byte is always 0
        wreg(rEP0FIFO, 0x00);
        // Load byte count, arm the IN transfer, ACK the status stage of the CTL transfer
        wregAS(rEP0BC, 2);
        break;
      } else {
        // Host tried to stall an invalid endpoint (not 3)
        wreg(rEPSTALLS, 0x23);        
      }
    default: 
      wreg(rEPSTALLS, 0x23); // Don't recognize the request
  }
}

void UsbDevice::feature(byte sc) {
  byte mask;
  if((_sud[bmRequestType] == 0x02) && (_sud[wValueL] == 0x00) && (_sud[wIndexL] == 0x83)) {
    // dir=h->p, recipient = ENDPOINT
    // wValueL is feature selector, 00 is EP Halt
    // wIndexL is endpoint number IN3=83
    // Read existing bits
    mask = rreg(rEPSTALLS); 
    if(sc==1) {
      // set_feature
      // Halt EP3IN
      mask += bmSTLEP3IN;
      _ep3stall = 1;
    } else {
      // clear_feature
      // UnHalt EP3IN
      mask &= ~bmSTLEP3IN;     
      _ep3stall = 0;
      // Clear the EP3 data toggle
      wreg(rCLRTOGS, bmCTGEP3IN);  
    }
    // Don't use wregAS for this: directly writing the ACKSTAT bit
    wreg(rEPSTALLS, (mask | bmACKSTAT)); 
  } else if ((_sud[bmRequestType]==0x00) && (_sud[wValueL]==0x01)) {
    // dir=h->p, recipient = DEVICE
    // wValueL is feature selector, 01 is Device_Remote_Wakeup
    // =2 for set, =0 for clear feature. The shift puts it in the get_status bit position. 
    _rwuEnabled = sc << 1; 
    // Dummy read to set ACKSTAT     
    rregAS(rFNADDR);
  } else {
    wreg(rEPSTALLS, 0x23);
  }
}

// NOTE This function assumes all descriptors are 64 or fewer bytes and can be sent in a single packet
void UsbDevice::sendDescriptor() {
  word reqlen, sendlen, desclen;
  // Pointer to ROM Descriptor data to send
  byte *pData;          
  // Check for zero as error condition (no case statements satisfied)
  desclen = 0;          
  reqlen = _sud[wLengthL] + 256 * _sud[wLengthH]; // 16-bit
  switch (_sud[wValueH]) { // wValueH is descriptor type
    case GD_DEVICE:
      desclen = DeviceDescriptor[0]; // Descriptor length
      pData = const_cast<byte*>(DeviceDescriptor);
      break;  
    case GD_CONFIGURATION:
      desclen = ConfigurationDescriptor[2]; // Config descriptor includes interface, HID, report and ep descriptors
      pData = const_cast<byte*>(ConfigurationDescriptor);
      break;
    case GD_STRING:
      desclen = StringDescriptors[_sud[wValueL]][0]; // wValueL=string index, array[0] is the length
      // Point to first array element
      pData = const_cast<byte*>(StringDescriptors[_sud[wValueL]]);  
      break;
    case GD_HID:
      desclen = ConfigurationDescriptor[18];
      pData = const_cast<byte*>(&ConfigurationDescriptor[18]);
      break;
    case GD_REPORT:
      desclen = ConfigurationDescriptor[25];
      pData = const_cast<byte*>(ReportDescriptor);
      break;
  } 
  if (desclen != 0) {                  
    // One of the case statements above filled in a value
    sendlen = (reqlen <= desclen) ? reqlen : desclen; 
    // Send the smaller of requested and avaiable
    writebytes(rEP0FIFO, sendlen, pData);
    // Load EP0BC to arm the EP0-IN transfer & ACKSTAT
    wregAS(rEP0BC, sendlen);
  }
  else wreg(rEPSTALLS, 0x23);  // none of the descriptor types match
}

void UsbDevice::classRequest() {
  wreg(rEPSTALLS, 0x23);
}                         

void UsbDevice::vendorRequest() {
  wreg(rEPSTALLS, 0x23);
}

void UsbDevice::testSPI() {
  byte j, wr, rd;
  Serial.println("SPI test");
  Serial.println();
  // Set SPI to full-duplex
  wreg(rPINCTL, bmFDUPSPI);
  reset();
  wr = 0x01;
  for(j=0; j<8; j++) {
    wreg(rUSBIEN, wr);
    rd = rreg(rUSBIEN);           
    wr <<= 1;       
    //  Values of 'rd' should be 01,02,04,08,10,20,40,80
    Serial.println(rd);
  }
  Serial.println();
}

void UsbDevice::wreg(byte atheDDress, byte value) {
  digitalWrite(_ssPin, LOW);
  SPI.transfer(atheDDress | B00000010);
  SPI.transfer(value);
  digitalWrite(_ssPin, HIGH);
}

void UsbDevice::wregAS(byte atheDDress, byte value) {
  digitalWrite(_ssPin, LOW);
  SPI.transfer(atheDDress | B00000011);
  SPI.transfer(value);
  digitalWrite(_ssPin, HIGH);
}

byte UsbDevice::rreg(byte atheDDress) {
  digitalWrite(_ssPin, LOW);
  SPI.transfer(atheDDress);
  byte value = SPI.transfer(0);
  digitalWrite(_ssPin, HIGH);
  return value;
}

byte UsbDevice::rregAS(byte atheDDress) {
  digitalWrite(_ssPin, LOW);
  SPI.transfer(atheDDress | B00000001);
  byte value = SPI.transfer(0);
  digitalWrite(_ssPin, HIGH);
  return value;
}

void UsbDevice::readbytes(byte atheDDress, byte count, byte *pointer) {
  byte i;
  digitalWrite(_ssPin, LOW);
  SPI.transfer(atheDDress);
  for (i = 0; i < count; i++) {
    *pointer = SPI.transfer(0);
    pointer++;
  }
  digitalWrite(_ssPin, HIGH);
}

void UsbDevice::writebytes(byte atheDDress, byte count, byte *pointer) {
  byte i;
  digitalWrite(_ssPin, LOW);
  SPI.transfer(atheDDress | B00000010);
  for (i = 0; i < count; i++) {
    SPI.transfer(*pointer);
    pointer++;
  }
  digitalWrite(_ssPin, HIGH);
}
