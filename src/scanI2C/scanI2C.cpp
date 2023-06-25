#include "scanI2C.h"

void scanI2C(HardwareSerial Serial, TwoWire Wire)
{
    int nDevices = 0;

  for( byte address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    byte res = Wire.endTransmission();

    
    if(res == 0)
    {
      nDevices++;
      Serial.print("Device found at address: 0x");
      Serial.println(address, HEX);

    }
  }

  Serial.print("Found ");
  Serial.print(nDevices);
  if(nDevices == 1) // 1 = NACK, read up on NACK in Wire.h
  {
    Serial.println(" device.");
  }
  else
  {
    Serial.println(" devices.");
  }
  
  Serial.println();
}