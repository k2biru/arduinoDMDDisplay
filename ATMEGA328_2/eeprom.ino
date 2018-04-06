#include <EEPROM.h>  

//############# Fungsi memulis data ke EEPROM ########################
void updateEEPROM (unsigned int alamat, unsigned int data, boolean panjang)
{
  byte hi, low;
  low = data & 0x00FF;
  EEPROM.update(alamat, low);
  if (panjang)
  {
    hi = data >> 8;
    alamat ++;
    EEPROM.update(alamat, hi);
  }
}

unsigned int readEEPROM (unsigned int alamat, boolean panjang)
{
  unsigned int Data = EEPROM.read(alamat);
  if (panjang)
  {
    alamat++;
    Data = Data + (EEPROM.read(alamat) << 8);
  }
  return Data;
}

//####################################################################


