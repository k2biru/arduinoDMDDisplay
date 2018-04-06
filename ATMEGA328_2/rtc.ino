#define DS3231_I2C_ADDRESS 0x68       // alamat DS3231
#define DS3231_TEMP_MSB  0x11         // alamat suhu


//############### Fungsi dasar komunikasi dengan RTC #################

byte decToBcd(byte val)                 //Merubah BEC ke BCD
{
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val)                 //Merubah BCD ke BEC
{
  return ( (val / 16 * 10) + (val % 16) );
}

void setDateDS3231                      //Fungsi SetDate
(byte second,                           // 0-59
 byte minute,                           // 0-59
 byte hour,                             // 1-23
 byte dayOfWeek,                        // 1-7
 byte dayOfMonth,                       // 1-28/29/30/31
 byte month,                            // 1-12
 byte year)                             // 0-99
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second));         // 0 to bit 7 starts the clock
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.write(00010000);                 // sends 0x10 (hex) 00010000 (binary) to control register - turns on square wave
  Wire.endTransmission();
}

void getDateDS3231                      // Fungsi baca jam dan tanggal dari RTC
(byte *second,
 byte *minute,
 byte *hour,
 byte *dayOfWeek,
 byte *dayOfMonth,
 byte *month,
 byte *year)
{
  // Reset the register pointer
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // A few of these need masks because certain bits are control bits
  *second     = bcdToDec(Wire.read() & 0x7f);
  *minute     = bcdToDec(Wire.read());
  *hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
  *dayOfWeek  = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month      = bcdToDec(Wire.read());
  *year       = bcdToDec(Wire.read());
}

float getTemp3231Celcius()
{

  Wire.beginTransmission(DS3231_I2C_ADDRESS);          // Membuka jalan ke RTC
  Wire.write(DS3231_TEMP_MSB);                         // kirim alamat yang diminta
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 2);             // meminta jawaban
  uint8_t riil  = Wire.read();                         // jawaban pertama berupa suhu riil [celcius]
  uint8_t cacah = (Wire.read() >> 6) * 25;             // jawaban kedua berupa 1/4 fraksi suhu(1/4 ; 2/4 ; 3/4 ; 4/4)
  float derajat = (float)riil;
  derajat += (float)cacah / ((derajat < 0) ? -100.0f : 100.0f) ; // bila derajat<0 maka dibagi -100.0f bila tidak dibagi 100.0f (baca datasheet ds3231 #bagian suhu negatif)
  return derajat;
}


//####################################################################

void setClock()
{
  byte X[2];
  //byte hour, minute, second;
  X[0] = MSG[2];
  X[1] = MSG[3];
  hour = atoi(X);
  X[0] = MSG[4];
  X[1] = MSG[5];
  minute = atoi(X);
  X[0] = MSG[6];
  X[1] = MSG[7];
  second = atoi(X);
  //Serial.print("Clock :"); Serial.print(hour); Serial.println(minute);
  setDateDS3231(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
}

void setDate()
{
  char X[2];
  X[0] = MSG[2];
  X[1] = MSG[3];
  dayOfMonth = atoi(X);
  X[0] = MSG[4];
  X[1] = MSG[5];
  month = atoi(X);
  X[0] = MSG[6];
  X[1] = MSG[7];
  year = atoi (X);
  dayOfWeek = cariHari(dayOfMonth, month, 2000 + year);
  setDateDS3231(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
  //Serial.print("Date :"); Serial.print(dayOfMonth); Serial.print(month); Serial.println(year);
  tampilkanHariTanggal();
}
void updateRTC()
{
 suhu = getTemp3231Celcius();
 getDateDS3231(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
 count++;
 Serial.print ("RTC Update ");Serial.print (count);Serial.print ("\n");
 updateTime=0;
}

