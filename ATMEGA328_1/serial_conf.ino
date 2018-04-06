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

String serialRead(char *ret)
{
  char temp[4][2], temp_[4], i;
  String data;
  if (Serial.available())
  {
    char read[MAX_SERIAL];
    Serial.readString().toCharArray(read,MAX_SERIAL);
    //read.toCharArray(temp[0], 2);
    Serial.println(read);
    //Serial.readString().toCharArray(MSG, MAX_SERIAL);   // baca serial dan dikonversi dari string ke array char
    if (read[0] == '#')             // mengecek apakah diproses ? (sesuai aturan baca:rule_serial_COM_v2.txt)
    {
      for (i = 0; i < 2; i++)
      {
        temp[1][i] = read[2 + i];
        temp[2][i] = read[4 + i];
        temp[3][i] = read[6 + i];
      }
      for (i = 0; i < 6; i++)
      {
        temp_[i] = read[2 + i];
      }
      Serial.println(temp[3]);
      *ret=read[1];

      //Serial.println(*ret);
      switch (read[1])
      {
        case 'C':

          hour = atoi(temp[1]);
          minute = atoi(temp[2]);
          second = 0;//atoi(temp[3]);
          Serial.println(hour);Serial.println(second);
          break;
        case 'D':

          dayOfMonth = atoi(temp[1]);
          month = atoi(temp[2]);
          year = atoi(temp[3]);
          dayOfWeek = cariHari(dayOfMonth, month, 2000 + year);

          break;

        case 'B':
          data = String(temp_);

          break;

        case 'T':
          data = String (temp[1]);
          break;

        case 'S':
          data = String(temp_);
          break;
          
        case 'm':
          //data = read.substring(2);
          break;
          
        case 'M':
          //data = read.substring(2);
          break;
          
        default:
          *ret = 0;
      }
    }
    else *ret = 0;
    return data;
  }
}

