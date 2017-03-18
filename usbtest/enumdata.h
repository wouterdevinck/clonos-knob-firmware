// http://www.beyondlogic.org/usbnutshell/usb5.shtml
// http://www.usb.org/developers/hidpage
// http://kentie.net/article/usbvolume/


// Report descriptor 
const byte ReportDescriptor[] = {
  // 0x05,0x01, // Usage Page (generic desktop)
  // 0x09,0x06, // Usage (keyboard)
  // 0xA1,0x01, // Collection (application)
  // 0x05,0x07, //   Usage Page 7 (keyboard/keypad)
  // 0x19,0xE0, //   Usage Minimum = 224
  // 0x29,0xE7, //   Usage Maximum = 231
  // 0x15,0x00, //   Logical Minimum = 0
  // 0x25,0x01, //   Logical Maximum = 1
  // 0x75,0x01, //   Report Size = 1
  // 0x95,0x08, //   Report Count = 8
  // 0x81,0x02, //  Input(Data,Variable,Absolute)
  // 0x95,0x01, //   Report Count = 1
  // 0x75,0x08, //   Report Size = 8
  // 0x81,0x01, //  Input(Constant)
  // 0x19,0x00, //   Usage Minimum = 0
  // 0x29,0x65, //   Usage Maximum = 101
  // 0x15,0x00, //   Logical Minimum = 0,
  // 0x25,0x65, //   Logical Maximum = 101
  // 0x75,0x08, //   Report Size = 8
  // 0x95,0x01, //   Report Count = 1
  // 0x81,0x00, //  Input(Data,Variable,Array)
  // 0xC0       // End Collection
  
  // 0x05, 0x0C, // USAGE_PAGE (Consumer Devices)
  // 0x09, 0x01, // USAGE (Consumer Control)
  // 0xa1, 0x01, // COLLECTION (Application)
  // 0x85, 0x01, // REPORT_ID (1)
  // 0x15, 0x00, // LOGICAL_MINIMUM (0)
  // 0x25, 0x01, // LOGICAL_MAXIMUM (1)
  // 0x75, 0x01, // REPORT_SIZE (1)
  // 0x95, 0x03, // REPORT_COUNT (3)
  // 0x09, 0xe2, // USAGE (Mute) 0x01
  // 0x09, 0xe9, // USAGE (Volume Up) 0x02
  // 0x09, 0xea, // USAGE (Volume Down) 0x03
  
  // 0x09, 0xcd, // USAGE (Play/Pause) 0x04
  // 0x09, 0xb7, // USAGE (Stop) 0x05
  // 0x09, 0xb6, // USAGE (Scan Previous Track) 0x06
  // 0x09, 0xb5, // USAGE (Scan Next Track) 0x07
  // 0x0a, 0x8a, 0x01, // USAGE (Mail) 0x08
  // 0x0a, 0x92, 0x01, // USAGE (Calculator) 0x09
  // 0x0a, 0x21, 0x02, // USAGE (www search) 0x0a
  // 0x0a, 0x23, 0x02, // USAGE (www home) 0x0b
  // 0x0a, 0x2a, 0x02, // USAGE (www favorites) 0x0c
  // 0x0a, 0x27, 0x02, // USAGE (www refresh) 0x0d
  // 0x0a, 0x26, 0x02, // USAGE (www stop) 0x0e
  // 0x0a, 0x25, 0x02, // USAGE (www forward) 0x0f
  // 0x0a, 0x24, 0x02, // USAGE (www back) 0x10
  // 0x81, 0x62, // INPUT (Data,Var,Abs,NPrf,Null)
  
  // 0xC0
  
  0x05, 0x0c,                    // USAGE_PAGE (Consumer Devices)
  0x09, 0x01,                    // USAGE (Consumer Control)
  0xa1, 0x01,                    // COLLECTION (Application)
  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
  0x09, 0xe9,                    //   USAGE (Volume Up)
  0x09, 0xea,                    //   USAGE (Volume Down)
  0x75, 0x01,                    //   REPORT_SIZE (1)
  0x95, 0x02,                    //   REPORT_COUNT (2)
  0x81, 0x06,                    //   INPUT (Data,Var,Rel)
  0x09, 0xe2,                    //   USAGE (Mute)
  0x95, 0x01,                    //   REPORT_COUNT (1)
  0x81, 0x06,                    //   INPUT (Data,Var,Rel)
  0x95, 0x05,                    //   REPORT_COUNT (5)
  0x81, 0x07,                    //   INPUT (Cnst,Var,Rel)
  0xc0                           // END_COLLECTION
}; 

// DEVICE Descriptor
const byte DeviceDescriptor[] = {
  0x12,             // bLength = 18d
  0x01,             // bDescriptorType = Device (1)
  0x00, 0x01,       // bcdUSB(L/H) USB spec rev (BCD)
  0x00, 0x00, 0x00, // bDeviceClass, bDeviceSubClass, bDeviceProtocol
  0x40,             // bMaxPacketSize0 EP0 is 64 bytes
  0x6A, 0x0B,       // idVendor(L/H)--Maxim is 0B6A               TODO
  0x46, 0x53,       // idProduct(L/H)--5346                       TODO
  0x01, 0x00,       // bcdDevice "Device Release Number": 0001
  1, 2, 3,          // iManufacturer, iProduct, iSerialNumber
  1                 // bNumConfigurations
};     

// CONFIGURATION Descriptor
const byte ConfigurationDescriptor[] = {
  0x09,       // bLength
  0x02,       // bDescriptorType = Config
  0x22 ,0x00, // wTotalLength(L/H) = 34 bytes
  0x01,       // bNumInterfaces
  0x01,       // bConfigValue
  0x00,       // iConfiguration
  0xE0,       // bmAttributes. b7=1 b6=self-powered b5=RWU supported    TODO Implement RWU when moving knob?
  0x01,       // MaxPower is 2 ma
// INTERFACE Descriptor
  0x09,       // length = 9
  0x04,       // type = IF
  0x00,       // IF #0
  0x00,       // bAlternate Setting
  0x01,       // bNum Endpoints
  0x03,       // bInterfaceClass = HID
  0x00, 0x00, // bInterfaceSubClass, bInterfaceProtocol (boot mode not supported)
  0x00,       // iInterface
// HID Descriptor
  0x09,       // bLength
  0x21,       // bDescriptorType = HID
  0x10, 0x01, // bcdHID(L/H) Rev 1.1
  0x00,       // bCountryCode (none)
  0x01,       // bNumDescriptors (one report descriptor)
  0x22,       // bDescriptorType  (report)
  sizeof(ReportDescriptor), 0,      // wDescriptorLength(L/H) (report descriptor size is 43 bytes)
// Endpoint Descriptor
  0x07,       // bLength
  0x05,       // bDescriptorType (Endpoint)
  0x83,       // bEndpointAddress (EP3-IN)    
  0x03,       // bmAttributes (interrupt)
  64, 0,      // wMaxPacketSize (64)
  10          // bInterval (poll every 10 msec)
};      

// STRING descriptors. An array of string arrays
const byte StringDescriptors[][64] = {
  { // STRING descriptor 0: Language string
    0x04,     // bLength
    0x03,     // bDescriptorType = string
    0x09,0x04 // wLANGID(L/H) = English-United Sates
  }, { // STRING descriptor 1: Manufacturer ID
    14,   // bLength
    0x03, // bDescriptorType = string
    'C',0,'l',0,'o',0,'n',0,'o',0,'s',0
  }, { // STRING descriptor 2: Product ID
    24,   // bLength
    0x03, // bDescriptorType = string
    'C',0,'l',0,'o',0,'n',0,'o',0,'s',0,' ',0,'k',0,'n',0,'o',0,'b',0
  }, { // STRING descriptor 3: Serial Number ID     
    20,   // bLength
    0x03, // bDescriptorType = string
    'S',0,'/',0,'N',0,' ',0,'0',0,'0',0,'0',0,'0',0,'1',0
  }
};

// Each letter is 3 bytes: shiftcode, 00, HID keycode
const byte theMessage[]={ 
  0x01, 0xE9, 0x00
  //0x00 ,0x00, 0x28, // (cr)
  //0x02, 0x00, 0x17, // T (02 is shift)
  //0x00, 0x00, 0x0B, // h
  //0x00, 0x00, 0x08, // e
  //0x00, 0x00, 0x2C, // (sp)
  //0x02, 0x00, 0x10, // M
  //0x02, 0x00, 0x04, // A
  //0x02, 0x00, 0x1B, // X
  //0x00, 0x00, 0x20, // 3
  //0x00, 0x00, 0x21, // 4
  //0x00, 0x00, 0x1F, // 2
  //0x00, 0x00, 0x27, // 0
  //0x02, 0x00, 0x08, // E
  //0x00, 0x00, 0x2C, // (sp)
  //0x00, 0x00, 0x07, // d
  //0x00, 0x00, 0x12, // o
  //0x00, 0x00, 0x08, // e
  //0x00, 0x00, 0x16, // s
  //0x00, 0x00, 0x2C, // (sp)
  //0x02, 0x00, 0x18, // U
  //0x02, 0x00, 0x16, // S
  //0x02, 0x00, 0x05, // B
  //0x02, 0x00, 0x1E, // !
  //0x00, 0x00, 0x7F, // mute
  //0x00, 0x00, 0x80, // up
  //0x00, 0x00, 0x80, // up
  //0x00, 0x00, 0x28  // (cr)
};
