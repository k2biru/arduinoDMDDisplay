#include <math.h>

//############# Fungsi Linier ke Logaritmik ##########################
unsigned int fscale( unsigned int originalMin, unsigned int originalMax, unsigned int newBegin, unsigned int
                     newEnd, unsigned int inputValue, float curve) {

  float OriginalRange = 0, NewRange = 0, zeroRefCurVal = 0, normalizedCurVal = 0;
  unsigned int rangedValue = 0;
  boolean invFlag = 0;

  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;

  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function
  // Check for out of range inputValues
  if (inputValue < originalMin) inputValue = originalMin;
  if (inputValue > originalMax) inputValue = originalMax;
  // Zero Refference the values
  OriginalRange = originalMax - originalMin;
  if (newEnd > newBegin)  NewRange = newEnd - newBegin;
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float
  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax )  return 0;

  if (invFlag == 0)    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;
  else  rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);      // invert the ranges

  return rangedValue;
}
//####################################################################
//################# fungsi menentukan hari ###########################

/*
   ref : https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
   senin=1 ~ minggu=7
*/
byte cariHari(byte d, byte m, unsigned int y)
{
  static char t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  y -= m < 3;
  byte temp = ( y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
  temp++;
  if (temp == 0) temp = 7;                                  //minggu = 7
  return temp;
}

//####################################################################


char serialRead()
{
  for (byte i = 0; i < MAX_SERIAL; i++) MSG[i] = 0;
  Serial.readString().toCharArray(MSG, MAX_SERIAL);
  //Serial.print ("read :"); Serial.println(MSG);
  if (MSG[0] == '#')return MSG[1];
  else return 0;

}

void setBrighness()
{
  char X[4];
  unsigned int brighness;
  for (byte i = 0; i < 4; i++) X[i] = MSG[2 + i];
  brighness = atoi(X);
  sprintf(MSG, "Brighness :%d", brighness);
  staticText(5, 4, 1, MSG);
  //Serial.print("Set brigh :"); Serial.println(brighness);
  setKecerahan(brighness);
  staticText(5, 4, 1, MSG); 
}

void setMarq()
{
  char X[3];
  unsigned int marq;
  for (byte i = 0; i < 3; i++) X[i] = MSG[2 + i];
  marq = atoi(X);
  updateEEPROM (3, marq, BIT_PANJANG);
  sprintf(MSG, "Marquee :%d", marq);
  staticText(13, 4, 2, MSG);
  staticText(13, 4, 0, MSG); // 99 = no timer, no black after, black before
  sprintf(MSG, "%d", marq);
  dmd.selectFont(Comic_Sans_MS_Custom_13);
  tampilMarque(0,0);
  
}


void correctionTemp()
{
  char X[3];
  signed char tCorrect; //-128 || 127
  for (byte i = 0; i < 3; i++) X[i] = MSG[2 + i];
  tCorrect = atoi(X);
  updateEEPROM (2, tCorrect, BIT_PENDEK);
  staticText(10, 0, 98, MSG); // blank
  tampilSuhu(22, 9);
  sprintf(MSG, "T.Correc : %d", tCorrect);
  staticText(10,0, 95, MSG);

  //Serial.print("Temp Correction :"); Serial.println(tCorrect);
}

void staticTextConf()
{
  byte Z[2],i;
  byte x, y, sec;
  Z[0] = MSG[2];
  Z[1] = MSG[3];
  x = atoi(Z);
  Z[0] = MSG[4];
  Z[1] = MSG[5];
  y = atoi(Z);
  Z[0] = MSG[6];
  Z[1] = MSG[7];
  sec = atoi(Z);
  
    
   for( i=0;i< 40;i++)
  {
    MSG[i]=MSG[i+8];
  }
  //Serial.print("Static Txt :"); Serial.print(x);Serial.print(y); Serial.println(sec);
  //Serial.print("text :");Serial.println(MSG);
  staticText(x, y,sec, MSG);

}

void marqueText()
{
  byte X[6],i,x;
  unsigned int speed;
  for (i = 0; i < 2; i++) X[i] = MSG[3 + i];
  speed = atoi(X)*10;
  if (MSG[2]=='S')
  {
    dmd.selectFont(minimalis);
    x=4;
  }
  else if (MSG[2]=='B')
  {
    dmd.selectFont(Comic_Sans_MS_Custom_13);
    x=0;
  }
  else if (MSG[2]=='M')
  {
    dmd.selectFont(Comic_Sans_MS_Custom_13);
    x=0;
  }
  
   for( i=0;i< MAX_SERIAL-8;i++)
  {
    MSG[i]=MSG[i+5];
  }
  //Serial.print("Marque Txt Speed :"); Serial.println(speed);
  //Serial.print("text :");Serial.println(MSG);
  tampilMarque(x,speed);
}

