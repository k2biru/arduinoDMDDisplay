#include <Wire.h>
//#include "Font3x5.h"
#include "minimalis.h"
#include "Comic_Sans_MS_Custom_13.h"       // Font sedang (Costom ASCII untuk menambahkan simbol derajat (" ` "= derajat)
#include <DMD.h>                    // Custom DMD library (ditambahkan setingan kecerahan)
#include <MsTimer2.h>

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
uint8_t DHTLembab, DHTSuhu;
byte counterDisplay, second, minute, hour, dayOfWeek, dayOfMonth, month, year;
byte menitAkhirAcak = rand() % 9, detikAcak = rand() % 59;
byte menitAkhirAcak2 = rand() % 9;                              // untuk menampilkan suhu
bool updateTime;
unsigned long last , counterOne = 0 ;
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, 1);
unsigned int count;

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
void tampilMarque(unsigned int);
void tampilkanHariTanggal();
byte decToBcd(byte );
byte bcdToDec(byte );
void setDateDS3231(byte, byte, byte, byte, byte, byte, byte);
void getDateDS3231(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year);
float getTemp3231Celcius();
char serialRead();
void setClock();
void setDate();
void DHTSampling();
void tampilKelembaban(byte x, byte y);
void setBrighness();
void setMarq();
void correctionTemp();
void staticTextConf();
void marqueText();
void weatherRead();
void updateRTC();

void timer2 ()
{
  updateTime =1;
   //getDateDS3231(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
   //Serial.print("s\n");
   //suhu = getTemp3231Celcius();              // sampling suhu
}



void setup() {
  randomSeed(analogRead(0));
  Serial.begin(2400);                  // serial ESP
  dmdInit();
  setKecerahan(2000);                         //Set kecerahan
  Wire.begin(); 
  dmd.clearScreen(0);
  getDateDS3231(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  digitalWrite(3, HIGH);
  staticText(4, 5, 92, "Yozora v2.1");
  //staticText(15, 7, 92, "Langit Malam");
  digitalWrite(3, LOW);
  //dmd.setBufferEdit(2);
  tampilkanHariTanggal();
  dmd.clearScreen(0 ); // 0 = Black
  //setDateDS3231(second, minute, hour, 6, dayOfMonth, month, year);
    /////
  MsTimer2::set(1000, timer2); // 500ms period
  MsTimer2::start();
  //

}

void loop() {

  while (Serial.available())
  {
    status = serialRead();
    if (status == 'C') setClock();
    else if (status == 'D') setDate();
    else if (status == 'B') setBrighness();
    else if (status == 'S') setMarq();
    else if (status == 'T') correctionTemp();
    else if (status == 'm') staticTextConf();
    else if (status == 'M') marqueText();
   if (status == 'C' || status == 'D' || status == 'M') dmd.clearScreen(0); // 0= black
   if (status != 'm')status = 0;
  }


  if (updateTime) updateRTC();

  
  if (minute == 0 && second == 0)digitalWrite(3, HIGH); else digitalWrite(3, LOW);
  //DHTSampling();
  unsigned long now = millis();
  if ((now - last) >= 999 && status == 0) {                 //timmer setiap mendekati 1000
    last = now;
    getDateDS3231(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

    if (menitAkhirAcak2 == (minute % 10) || menitAkhirAcak2 + 3 == (minute % 10))
    {
       if (second == 0) dmd.clearScreen(0);
      tampilSuhu(19, 1);                 // mencetak suhu (lokasi)
      tampilKelembaban(19, 7);
      if (second == 59)
      {
        menitAkhirAcak2 = rand() % 10;        //variabel acak dari suhu
        dmd.clearScreen(0);

      }
    }
    //if (counterDisplay%3==0)
    else jamAngka(19, 4);                       // mencetak jam angka (lokasi)

    if (hour >=4 && hour <=21)
    {
      runClockBox(0, 0);                          // mencetak jam kotak (lokasi)
      if (minute >= 59 && second >= 59) dmd.clearScreen(0 ); // 0 = Black
    }

  }
  if (menitAkhirAcak == (minute % 10) && detikAcak == second) tampilkanHariTanggal();
}
