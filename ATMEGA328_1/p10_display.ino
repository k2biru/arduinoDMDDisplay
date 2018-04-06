#include <SPI.h>                    // SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <TimerOne.h>               // timer 0 untuk DMD


void dmdInit() {
  Timer1.initialize( 4500 );              //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  Timer1.attachInterrupt( ScanDMD );      //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()
  dmd.clearScreen( true );                //true is normal (all pixels off), false is negative (all pixels on)
  dmd.selectFont(Comic_Sans_MS_Custom_13);

}

void ScanDMD()
{
  dmd.scanDisplayBySPI();
}

//############# Fungsi Kecerahan DMD #################################

void setKecerahan(unsigned int brighness)
{
   if (brighness >=1025) brighness = readEEPROM (0, BIT_PANJANG);                                 // baca EEPROM di alamat 0 ,16bit data
  else
  {
   if (brighness != 0 || brighness >= 1020)brighness = fscale( 1, 1024, 1, 1024, brighness, -5);    // merubah skala dari linier ke logaritmik
  }
  //terang=100;
  updateEEPROM (0, brighness, BIT_PANJANG);                                                 // update EEPROM di alamat 0 ,16bit data
  Timer1.pwm(PIN_DMD_nOE, brighness); //0~1024 [10 16 25 40 65 100 160 250 400 640 1024]
}

//####################################################################
//############### Fungsi Menampilkan jam KOTAK #######################

void runClockBox(byte LOKASI_JAM_X, byte LOKASI_JAM_Y)
// draws the hour hand
{

  const byte x_secMin [] = {8, 9, 10, 11, 12, 13, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 13, 12, 11, 10, 9, 8,   7, 6, 5, 4, 3,      2, 1, 0, 0, 0,      0, 0, 0, 0, 0,    0, 0, 0, 0, 0,  0, 0, 0, 1, 2,  3, 4, 5, 6, 7};
  const byte y_secMin [] = {0, 0, 0, 0, 0,    0, 0, 0, 1, 2,      3, 4, 5, 6, 7,      8, 9, 10, 11, 12,   13, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3,  2, 1, 0, 0, 0,  0, 0, 0, 0, 0};
  byte x_hour [] = {0, 11, 11, 11, 11, 9, 7, 4, 4, 4, 4, 6, 7};
  byte y_hour [] = {0, 4, 6, 7, 11, 11, 11, 11, 9, 7, 4, 4, 4};
  byte x_hour_c, y_hour_c;
  byte temp;

  if (second == 0) dmd.drawFilledBox(LOKASI_JAM_X + 1, LOKASI_JAM_Y + 1, LOKASI_JAM_X + 14, LOKASI_JAM_Y + 14, 1); // kosongkan jarum menit dan jam
  temp = hour;
  if (temp == 0)temp = 12;
  if (temp > 12)
  {
    temp -= 12; // only 12 hours on analogue clocks
  }

  //==== menentukan titik tengah (pusat) jarum
  if (temp <= 5)x_hour_c = LOKASI_JAM_X + 8;
  else x_hour_c = LOKASI_JAM_X + 7;

  if (temp <= 3) y_hour_c = LOKASI_JAM_Y + 7;
  else if (temp <= 8) y_hour_c = LOKASI_JAM_Y + 8;
  else if (temp <= 12) y_hour_c = LOKASI_JAM_Y + 7;

  dmd.drawLine( x_hour_c, y_hour_c, LOKASI_JAM_X + x_hour[temp], LOKASI_JAM_Y + y_hour[temp], GRAPHICS_NORMAL ); // menggambar jarum jam
  // draw minute
  byte x_min_c, y_min_c;
  if (minute <= 29)x_min_c = LOKASI_JAM_X + 8;
  else x_min_c = LOKASI_JAM_X + 7;

  if (minute <= 14) y_min_c = LOKASI_JAM_Y + 7;
  else if (minute <= 44) y_min_c = LOKASI_JAM_Y + 8;
  else if (minute <= 59) y_min_c = LOKASI_JAM_Y + 7;

  dmd.drawLine( x_min_c, y_min_c, LOKASI_JAM_X + x_secMin[minute], LOKASI_JAM_Y + y_secMin[minute], GRAPHICS_NORMAL );

  // draw Second
  dmd.drawBox(LOKASI_JAM_X, LOKASI_JAM_Y, LOKASI_JAM_X + 15, LOKASI_JAM_Y + 15, GRAPHICS_NORMAL);
  dmd.writePixel( LOKASI_JAM_X + x_secMin[second], LOKASI_JAM_Y + y_secMin[second], GRAPHICS_NOR, 1);
}
//####################################################################
//################# Fungsi menampilkan jam tulisan ###################
void jamAngka(byte x, byte y)
{
  char JAM [2], MENIT [2], DETIK [2];
  dmd.selectFont(Comic_Sans_MS_Custom_13);

  if (hour == 0)hour = 24;
  if (hour < 10)sprintf(JAM, "0%d", hour); else sprintf(JAM, "%d", hour);
  if (minute < 10)sprintf(MENIT, "0%d", minute); else sprintf(MENIT, "%d", minute);
  if (second < 10)sprintf(DETIK, "0%d", second); else sprintf(DETIK, "%d", second);
  sprintf(MSG, "%s:%s:%s  " , JAM, MENIT, DETIK);
  dmd.drawString(  x, y, MSG , strlen(MSG) , GRAPHICS_NORMAL );
}
//####################################################################
//############# Fungsi menampilkan Suhu ##############################
void tampilSuhu(byte x, byte y)
{
  suhu = getTemp3231Celcius();
  byte riil = suhu;
  byte cacah = (suhu - riil) * 100;
  dmd.selectFont(Comic_Sans_MS_Custom_13);
  sprintf(MSG, "%d,%d`C       ", riil, cacah);
  dmd.drawString(  x, y, MSG , strlen(MSG) , GRAPHICS_NORMAL );
  if (second == 59)
  {
    menitAkhirAcak2 = rand() % 10;        //variabel acak dari suhu
    dmd.clearScreen(true);

  }
}
//####################################################################
//############# Fungsi menampilkan Scrooling #########################
void tampilMarque()
{
  dmd.drawMarquee(MSG, strlen(MSG), (32 * DISPLAYS_ACROSS) - 1, 1);
  long start = millis();
  long timer = start;
  boolean ret = false;
  while (!ret) {
    if ((timer + 60) < millis()) {
      ret = dmd.stepMarquee(-1, 0);
      timer = millis();
    }
  }
}
//####################################################################
//############# Fungsi menampilkan hari tanggal ######################
void tampilkanHariTanggal()
{
  char hari[][7] = {"", "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};
  char bulanTeks[][9] = {"", "Januari", "Febluari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};

  sprintf(MSG, " %s, %d %s 20%d", hari[dayOfWeek], dayOfMonth, bulanTeks[month], year);

  tampilMarque();
  menitAkhirAcak = rand() % 9;
  detikAcak = rand() % 59;
}
//####################################################################
//##################### Menampilkan static text ######################
void staticText(byte x, byte y, byte sec, char text[MAX_ST_TXT])
{
  dmd.clearScreen(true);
  dmd.drawString(  x,  y, text, strlen(text), GRAPHICS_NORMAL );
  delay (sec*990);  
  dmd.clearScreen(true);
}
//####################################################################
//##################### Menampilkan Text #############################
/*void menampilkanTeks()
{
  char temp [strlen(serial) - 3];
  for (int i = 0; i <= (strlen(serialData)); i++) temp[i] = serialData[i + 3];
  strcpy(MSG, "   ");
  strcat(MSG, temp);
  tampilMarque();
  status = 0;
}
*/
//####################################################################

