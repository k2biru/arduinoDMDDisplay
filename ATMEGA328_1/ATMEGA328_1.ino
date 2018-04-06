#include <Wire.h>
#include "Comic_Sans_MS_Custom_13.h"       // Font sedang (Costom ASCII untuk menambahkan simbol derajat (" ` "= derajat)
#include <DMD.h>                    // Custom DMD library (ditambahkan setingan kecerahan)

#define DEBUG false                    // debug untuk menampilkan serial (true untuk debug)
#define BIT_PANJANG true                  // panjang artinya 16 bit
#define BIT_PENDEK false                  // pendek artinya 8 bit
#define MAX_SERIAL 140                 // maksimal daya tampung serial
#define MAX_ST_TXT 10
#define DISPLAYS_ACROSS 2             // panjang  DMD
#define DISPLAYS_DOWN 1               // lebar DMD


//Global var
char status, MSG[MAX_SERIAL];
float suhu;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
byte menitAkhirAcak = rand() % 9, detikAcak = rand() % 59;
byte menitAkhirAcak2 = rand() % 9;                              // untuk menampilkan suhu

unsigned long last ;
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

//####################################################################


void setTanggal();
void cariHari();
void setjam();
unsigned int fscale( unsigned int, unsigned int , unsigned int , unsigned int , unsigned int , float );
void updateEEPROM (unsigned int , unsigned int, boolean );
unsigned int readEEPROM (unsigned int, boolean );
void dmdInit();
void ScanDMD();
void setKecerahan(unsigned int);
void runClockBox(byte, byte );
void jamAngka(byte , byte );
void tampilSuhu(byte , byte );
void tampilMarque();
void tampilkanHariTanggal();
//void menampilkanTeks();
byte decToBcd(byte );
byte bcdToDec(byte );
void setDateDS3231(byte, byte, byte, byte, byte, byte, byte);
void getDateDS3231(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year);
float getTemp3231Celcius();
String serialRead(char *ret);
void setClock();
void setDate();





void setup() {
  Serial.begin(2400);                  // serial ESP
  dmdInit();
  setKecerahan(2000);                         //Set kecerahan
  //Timer1.pwm(PIN_DMD_nOE, 1); //0~1024 [10 16 25 40 65 100 160 250 400 640 1024]
  Wire.begin();
  suhu = getTemp3231Celcius();              // sampling suhu
  //setDateDS3231(0, 44, 19, 2, 2, 1, 17);

  getDateDS3231(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  digitalWrite(3, HIGH);
  staticText(0,1,1,"Yozora 1");
  digitalWrite(3, HIGH);
  //tampilkanHariTanggal();
  //setDateDS3231(second, minute, hour, 7, dayOfMonth, month, year);

}

void loop() {

  String data;
  if (Serial.available()) { data =serialRead(&status);
    }
  while (!Serial.available())
  {
    if (minute == 0 && second == 0)digitalWrite(3, HIGH); else digitalWrite(3, LOW);
    if (status=='C') setClock();
    if (status=='D') setDate();

    
    unsigned long now = millis();
    if ((now - last) >= 999) {                   //timmer setiap mendekati 1000
      last = now;
      getDateDS3231(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
      if (menitAkhirAcak2 == (minute % 10) || menitAkhirAcak2 + 3 == (minute % 10)) tampilSuhu(17, 1);                 // mencetak suhu (lokasi)
      else jamAngka(17, 1);                       // mencetak jam angka (lokasi)
      runClockBox(0, 0);                          // mencetak jam kotak (lokasi)
    }
    if (menitAkhirAcak == (minute % 10) && detikAcak == second) tampilkanHariTanggal();
  }

}
