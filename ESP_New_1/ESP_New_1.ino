/*
  ESP_1 membuat server dan mengirim data dari _GET Request ke serial diawali dengan "#"
        aturan serial baca: rule_serial_COM.txt OK
  ESP_2 membuat akses point dan server sendiri dikirim ke DMD dengan baudrate "SANGAT RENDAH"
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <NTPtimeESP.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <EasyTransfer.h>


extern "C" {
#include "user_interface.h"
}

#define DEBUG false
#define led 2                                     // LED Indikator proses
#define MAX_BUFFER 100

NTPtime NTPid("0.id.pool.ntp.org");         //For NTC online time/
strDateTime dateTime;                     //For NTC online time/
MDNSResponder mdns;
EasyTransfer ET;

struct weatherDat
{
  byte date;
  byte month;
  float minTemp;
  float maxTemp;
  byte humidity;
  float windSpeed;
  unsigned int windBearing;
  float pressure;
  byte cloud;
  char icon[6];
  char summary[30];
};

struct byteSet
{
  union {
    char all;
    struct {
      boolean onlineTime: 1;
      boolean debugUpdate: 1;
      boolean wifiAutoConnect: 1;
      boolean noDataMsg: 1;
      boolean weatherEnable: 1;
      boolean f: 1;
      boolean r: 1;
      boolean weatherReady: 1;
    };
  } ;
};

//konek ke
char APSSID [20];
char APPass[20];
char ssid[20] ;
char pass[20] ;
String MSG[3] ;
byte APCh;
unsigned long last ;
unsigned int x;
boolean getOnce = true;
byteSet setting;
weatherDat weather[2];


ESP8266WebServer server ( 80 );                   // port Internet yang dipakai


void handleNotFound() {
  digitalWrite ( led, 1 );
  String message = "404 :: No body here but checken !\n\n";
  message += "\n";
  server.send ( 404, "text/plain", message );     // mengirim data 404 (halaman tidak ada) ke pengirim [fungsi debug]
  digitalWrite ( led, 0 );
}
void handleRestart()
{
  String message = " Restart the ESP in 5 Second\n\n";
  server.send ( 200, "text/plain", message );
  restartESP(5000);
}
void handleSentData() {
  digitalWrite ( led, 1 );
  String message = "SENT DATA\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    if (setting.debugUpdate)message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 200, "text/plain", message );         // mengirim data 200 (OK data diterima)ke pengirim  [fungsi debug]
  digitalWrite ( led, 0 );
  if (server.arg("newAPSSID") != "")
  {
    server.arg("newAPSSID").toCharArray(APSSID, 20);
    if (server.arg("newAPSSID") != "") server.arg("newAPPass").toCharArray(APPass, 20);
    else sprintf(APPass, "");
    if (server.arg("newAPCh") != "")APCh = server.arg("newAPCh").toInt();
    writeSettings(setting.all);
    restartESP(5000);
  }
  if (server.arg("newSSID") != "")
  {
    server.arg("newSSID").toCharArray(ssid, 20);
    if (server.arg("newPass") != "")server.arg("newPass").toCharArray(pass, 20);
    else sprintf(pass, "");
    writeSettings(setting.all);
    restartESP(5000);
  }
  if (server.arg("newLink") != "")
  {
    char temp[100] ;
    server.arg("index").toCharArray(temp, 2);
    int index = atoi(temp);
    if (index > 0 && index <= 3)
    {
      server.arg("newLink").toCharArray(temp, 100);
      index = (100 * index) + 1 ;
      writeEEPROMChar(temp, index, 100);
    }
  }
  if (server.arg("newByteSet") != "" || server.arg("setOnlineTime") == "1" || server.arg("setArgEnable") == "1" || server.arg("setAutoConnect") == "1")
  {
    byte sets = server.arg("newByteSet").toInt();
    setting.onlineTime = server.arg("setOnlineTime").toInt();
    setting.debugUpdate = server.arg("setArgEnable").toInt();
    setting.wifiAutoConnect = server.arg("setAutoConnect").toInt();
    writeSettings(setting.all);
    restartESP(5000);
  }
}
void handleHome()
{
  String text;
  char temp[20];
  byte n = WiFi.scanNetworks();
  WiFi.hostname().toCharArray(temp, 20);
  text = "<html><head><title>Home Of "; text += temp;
  text += "</title><style>body{background-color:#cccccc;font-family:Arial,Helvetica,Sans-Serif;Color:#000088;}";
  text += "</style></head><body><h1>Home Of "; text += temp; text += "</h1>";
  text += "<p>This is the main page of "; text += temp;
  text += "</p><table><tr><th>=========</th><th>=========</th><th>=========</th></tr>";
  text += "<tr><th><a href='/debug'>[DEBUG]</a></th><th><a href='/restart'>[RESTART]</a></th><th><a href='/conf'>[CONFIG]</a></th></tr>";
  text += "<tr><th>=========</th><th>=========</th><th>=========</th></tr></table><br>";
  text += "<p>Setting Wifi</p><form method='POST' action='/update' target='_blak'>";
  text += "SSID:<input type='text' name='newSSID' value='"; text += ssid; text += "'><br>Pass:<input type='password' name='newPass' value='"; //text += pass;
  text += "'> [Opsional]<br>";
  text += "<input type='submit' value='Save'></form><br>";
  text += "<a href='/'>[REFRESH]</a>";
  text += "<p>==|Networks|==</p>";
  if (n == 0) text += "No Network Found<br>";
  else {
    text += "======== Network Available ========<br><table><tr><th>SSID</th><th>RSSI</th><th>Encryption</th>";
    for (byte i = 0; i < n; i++)
    {
      text += "<tr><th>"; text += WiFi.SSID(i); text += "</th><th>"; text += WiFi.RSSI(i); text += "</th><th>"; text += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*" ;
      text += "</th></tr>";
      delay(10);
    }
    text += "</table><br>===============================<br>";

  }

  text += "Basic GUI by <a href='mailto:fahrizal.hari@gmail.com'>Fahrizal Hari Utama</a> v1 ><a href='/raw'>@512KB</a><br>(c)Malang ID 2017<br>";
  text += "</body></html>";
  server.send ( 200, "text/html", text );
}

void handleConf()
{
  char temp[20];
  int i = 1;
  WiFi.hostname().toCharArray(temp, 20);
  String text ;
  text = "<html><head><title>Configuration ";
  text += temp;
  text += "</title><style>body{background-color:#cccccc;font-family:Arial,Helvetica,Sans-Serif;Color:#000088;}</style>";
  text += "</style></head><body><h1>CONFIGURATION</h1>";
  text += "===========<br><a href='..'>[HOME]</a><br>===========<br>";
  text += "<p>Set Wifi AP</p>Setting Acces Point<br><form method='POST' action='/update' target='_blak'>";
  text += "APSSID:<input type='text' name='newAPSSID' value='"; text += APSSID; text += "'><br>";
  text += "APPass:<input type='password' name='newAPPass' value='";// text+= APPass;
  text += "'> [Opsional]<br>";
  text += "Channel:<select name='newAPCh'>";
  for (i = 1; i <= 14; i++)
  {
    text += "<option";
    text += " value='"; text += String(i); text += "'"; if (APCh == i)text += " selected";
    text += ">Channel "; text += String(i);
    text += "</option>";
  }
  text += "</select>";
  text += "<input type='submit' value='Save'></form>";
  text += "<p>Set Link</p><form method='POST' action='/update' target='_blak'>";
  text += "Link :<input type='text' name='newLink'><select name='index'>";
  for (i = 1; i <= 3; i++)
  {
    text += "<option";
    text += " value='"; text += String(i); text += "'>"; text += String(i);
    text += "</option>";
  }
  text += "</select>";
  text += "<input type='submit' value='Save'></form> ";
  text += "<p>Advance mode</p><form method='POST' action='/update' target='_blak'>";
  //text += "ByteSetting [8bit]:<input type='text' name='newByteSet'>[0~255]<br>";
  text += "<input type='checkbox' name='setOnlineTime' value=1 "; if (setting.onlineTime)text += "checked";
  text += ">NTP Server [Automatic Time Update]<br>";
  text += "<input type='checkbox' name='setArgEnable' value=1 "; if (setting.debugUpdate)text += "checked";
  text += ">Argument [/update] Enable<br>";
  text += "<input type='checkbox' name='setAutoConnect' value=1 "; if (setting.wifiAutoConnect)text += "checked";
  text += ">[WiFi] Auto Connect <br>";
  text += "<input type='submit' value='Save'></form>";
  text += "Basic GUI by <a href='mailto:fahrizal.hari@gmail.com'>Fahrizal Hari Utama</a> v1 @512KB<br>(c)Malang ID 2017</body></html>";
  server.send ( 200, "text/html", text );
}

void handleRawData()
{
  String message = "404 :: Nobody here but chicken ! Re.LieF\n\n";
  char temp[100], temp_[100];
  message += "\n";
  server.send ( 200, "text/plain", message );     // mengirim data 404 (halaman tidak ada) ke pengirim [fungsi debug]
  if (server.arg("raw") != "")
  {
    server.arg("raw").toCharArray(temp, 100);
    Serial.println (temp);
  }else   if (server.arg("raw_") != "")
  {
    temp_[0]='#';
    server.arg("raw_").toCharArray(temp, 100);
    strcat(temp_,temp);
    Serial.println (temp_);
  }
}

void handleDebug()
{
  char link[3][100];
  for (byte i = 0; i < 3; i++)  readEEPROMChar(link[i], (100 * (1 + i)) + 1, 100);
  String message = "Debug \n\nSaved Link \n Link 1 [Anime]:";

  message += link[0];
  message += "\n Link 2 [xxxxx]:";
  message += link[1];
  message += "\n Link 3 [xxxxx]:";
  message += link[2];

  message += "\n\nData link\n [Data [1]] :";
  message += MSG[0];
  message += "\n [Data [2]] :";
  message += MSG[1];
  message += "\n [Data [3]] :";
  message += MSG[2];


  message += "\n\nAP and Wifi\n [AP ID] :";
  message += APSSID;
  message += "\n [pass]  :";
  message += APPass;
  message += "\n [APCh]  :";
  message += APCh;
  message += "\n\n [Wifi]  :";
  message += ssid;
  message += "\n [pass]  :";
  message += pass;
  message += "\n\n [AP IP]        :";
  message += WiFi.softAPIP().toString();
  message += "\n [Local IP]     :";
  message += WiFi.localIP().toString();
  message += "\n [Station Name] :";
  message += WiFi.hostname();
  //message += wifi_station_get_hostname();
  message += "\n\nbyteSetting :";
  message += (int)setting.all;


  message += "\n\n OK";
  server.send ( 200, "text/plain", message );
}


void setup ( void ) {
  delay(8000);
  pinMode ( led, OUTPUT );
  digitalWrite ( led, 0 );                                // set GPIO sebagai indikator output
  Serial.begin ( 2400);
  delay(1000);
  if (DEBUG)Serial.println ( " ===========ESP8266==========" );
  setting.all = readSettings();
  //setting.all= 0xFF;
  WiFi.hostname(APSSID);
  //wifi_station_set_hostname(APSSID);
  //wifi_station_get_hostname();
  //wifi_station_dhcpc_start();

  wifi_station_set_auto_connect(setting.wifiAutoConnect);
  if (DEBUG)
  {
    Serial.print(" byteSet :");

    Serial.println(setting.all, HEX);
    Serial.print(" APSSID :");
    Serial.println(APSSID);
    Serial.print(" Pass :");
    Serial.println(APPass);
    Serial.print(" APCh :");
    Serial.println(APCh);
    WiFi.softAP(APSSID, APPass, APCh);
  }
  IPAddress myIP = WiFi.softAPIP();
  //Serial.print("#m070400AP OK\r\n");
  if (DEBUG)

  {
    Serial.print(" AP IP : "); Serial.println(myIP);

    Serial.print(" SSID :");
    Serial.println(ssid);
    Serial.print(" Pass :");
    Serial.println(pass);
  }
  byte timeOut = 0;

  WiFi.begin(ssid, pass);
  Serial.print("#m070400"); Serial.println(ssid);
  while ( WiFi.status() != WL_CONNECTED && timeOut < 5 ) {
    delay ( 1000 );
    timeOut++;
  }
  if (timeOut >= 5) Serial.print ("#m200498No Wifi\n");
  else
  {
    if (DEBUG) {
      Serial.print ( "#m100998Connected\n" );
    }
  }
  timeOut = 0;
  server.on ("/", handleHome);
  server.on ("/conf", handleConf);
  server.on ("/update", handleSentData);// bila berada di index server
  server.on ("/debug", handleDebug);                        // bila berada di index server
  server.on ("/restart", handleRestart);
  server.on ("/raw", handleRawData);
  server.onNotFound ( handleNotFound );                   // bila tiddak di index server
  server.begin();
  randomSeed(42);
  delay(2000);
  Serial.println ( "#m350402I'M OK" );
 delay(2000);
}

void loop ( void ) {
  server.handleClient();
  if (WiFi.status() == WL_CONNECTED && getOnce == true) getOnce = firstConnectSSID();
  unsigned long now = millis();
  if (now - last >= 120000) { // 2 minute
    last = now;
    if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.reconnect();
      getOnce = true;
    }
    else
    {
      String data = "#MB00";
      data += randomMSG();
      Serial.println(data);
      x++;
      if (x >= 5)
      {
        getHTTP();
        getTimeOnline();
        x = 0;
      }
    }
  }
}





/////////////////////////////////////////////////////
boolean firstConnectSSID()
{
  char name[10] = "esp01";
  Serial.print("#m100902Connected \n");
  //if(DEBUG) 
  delay(2000);
  Serial.print("#m050105");Serial.println(WiFi.localIP());
  if (mdns.begin(name))
    if(DEBUG)Serial.println("MDNS OK");

  MDNS.addService("http", "tcp", 80);
  delay(2000);
  getHTTP();
  getTimeOnline();
  //weatherGet();
  //if(DEBUG)cekWeather();
  yield();
  //weatherCom();
  int s = (millis() - (randomMSG().length() * 2)) % 50;
  randomSeed(s);
  String data = "#MB00";
  data += randomMSG();
  Serial.println(data);
  return false;
}
String randomMSG()
{
  byte timeOut = 0;
  byte randn;
  String temp ;
  do
  {
    timeOut++;
    randn = rand() % 3;
    temp = MSG [randn];
    //Serial.print ("random :");Serial.println(randn);
    if (timeOut > 10) return "No Data";
  } while (temp == "");
  return temp ;
}

void getHTTP ()
{

  HTTPClient http;
  char link[3][100];

  for (byte i = 0; i < 3; i++)
  {
    readEEPROMChar(link[i], (100 * (1 + i)) + 1, 100);
    //Link 1 anime
    if (DEBUG) Serial.print(" [HTTP] begin...\n");
    http.begin(link[i]); //HTTP
    if (DEBUG) Serial.print(" [HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    if (DEBUG) Serial.printf(" [HTTP] GET... code: %d\n", httpCode);
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        payload.replace("Subtitle Indonesia", " ## ");
        payload.replace("Episode", "EP");
        MSG[i] = payload;
      }
    } else {
      Serial.printf(" [HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();

  }
}


void writeEEPROMChar(char buff [MAX_BUFFER], unsigned int from , unsigned int length)
{
  EEPROM.begin(512);
  unsigned int i;
  for ( i = 0; i < length ; i++)
  {
    EEPROM.write(i + from , buff[i]);
  }
  EEPROM.end();
}

void readEEPROMChar( char *buff, unsigned int from , unsigned int length)
{
  EEPROM.begin(512);
  unsigned int i;
  for ( i = 0; i < length ; i++)
  {
    buff[i] = EEPROM.read(i + from);
  }
  EEPROM.end();
}
byte readSettings()
{
  byte setting;
  /// read AP
  readEEPROMChar(APSSID, 11, 20);
  readEEPROMChar(APPass, 31, 20);
  EEPROM.begin(512);
  APCh = EEPROM.read(52);
  setting = EEPROM.read(53);
  //if (APCh<1||APCh>14)APCh=6;
  EEPROM.end();
  /// read WiFi
  readEEPROMChar(ssid, 56, 20);
  readEEPROMChar(pass, 77, 20);
  return setting;
}

void writeSettings( byte byteSet)
{
  char buff[20];
  byte temp;
  readEEPROMChar(buff, 11, 20);
  if (buff != APSSID)writeEEPROMChar(APSSID, 11, 20);
  readEEPROMChar(buff, 31, 20);
  if (buff != APPass)writeEEPROMChar(APPass, 31, 20);
  readEEPROMChar(buff, 56, 20);
  if (buff != ssid)writeEEPROMChar(ssid, 56, 20);
  readEEPROMChar(buff, 77, 20);
  if (buff != pass)writeEEPROMChar(pass, 77, 20);
  EEPROM.begin(512);
  temp = EEPROM.read(52);
  if (temp != APCh)EEPROM.write(52, APCh);
  temp = EEPROM.read(53);
  if (temp != byteSet)EEPROM.write(53, byteSet);
  EEPROM.end();
}

void restartESP(unsigned int wait)
{
  Serial.println("#m030303REBOOT");
  delay(wait);
  ESP.restart();
}
void getTimeOnline()
{
  if (!setting.onlineTime)return;

  // UTC+7 indonesia(Jakarta)
  dateTime = NTPid.getNTPtime(7, 0); //UTC+7 non DaylighS
  //NTPid.printDateTime(dateTime);
  char temp[11];
  char tp[2];
  if (!dateTime.valid) return ;
  strcpy(temp, "#C");
  if (dateTime.hour < 10)sprintf(tp, "0%d", dateTime.hour); else sprintf(tp, "%d", dateTime.hour);
  strcat(temp, tp);
  if (dateTime.minute < 10)sprintf(tp, "0%d", dateTime.minute); else sprintf(tp, "%d", dateTime.minute);
  strcat(temp, tp);
  if (dateTime.second < 10)sprintf(tp, "0%d", dateTime.second); else sprintf(tp, "%d", dateTime.second);
  strcat(temp, tp);
  Serial.println(temp);
  delay(2000);
  strcpy(temp, "#D");
  if (dateTime.day < 10)sprintf(tp, "0%d", dateTime.day); else sprintf(tp, "%d", dateTime.day);
  strcat(temp, tp);
  if (dateTime.month < 10)sprintf(tp, "0%d", dateTime.month); else sprintf(tp, "%d", dateTime.month);
  strcat(temp, tp);
  sprintf(tp, "%d", dateTime.year - 2000);
  strcat(temp, tp);
  //sprintf(temp,"#1D%s%s%s\r\n", dy,mh,yr);
  Serial.println(temp);
  /////////////////
  //Serial.println(dateTime.dayofWeek);
  delay(3000);
}

void weatherGet()
{
  HTTPClient http;
  String payload;
  char link[100];
  readEEPROMChar(link, 400, 100);
  strcpy(link, "http://bit.ly/2ltXHoa");
  if (DEBUG) Serial.print(" [HTTP] begin...\n");
  http.begin(link); //HTTP
  if (DEBUG) Serial.print(" [HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  if (DEBUG) Serial.printf(" [HTTP] GET... code: %d\n", httpCode);
  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
    }
  } else {
    Serial.printf(" [HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  if (payload != "")
  {
    DynamicJsonBuffer jsonBuf;
    JsonObject &root = jsonBuf.parseObject(payload);
    if (!root.success())
    {
      Serial.println("parseObject() failed");
      return;
    }
    /*
      byte date;
      byte month;
      float minTemp;
      float maxTemp;
      byte humidity;
      float windSpeed;
      unsigned int windBearing;
      float pressure;
      byte cloud;
      char[6] icon;
      char[30] summary;
    */
    String temp;
    for (byte i = 0; i < 2; i++)
    {

      strcpy(weather[i].icon, root["daily"]["data"][i + 1]["icon"]);
      strcpy(weather[i].summary, root["daily"]["data"][i + 1]["summary"]);
      //weather[i].summary="";
      //weather[i].icon="";
      //temp = root["daily"]["data"][i]["icon"];
      //weather[i].icon = temp;
      //temp = root["daily"]["data"][i]["summary"];
      //weather[i].summary=temp;
      weather[i].minTemp = root["daily"]["data"][i + 1]["temperatureMin"];
      weather[i].maxTemp = root["daily"]["data"][i + 1]["temperatureMax"];
      weather[i].humidity = (float)(root["daily"]["data"][i + 1]["humidity"]) * 100;
      weather[i].cloud = (float)(root["daily"]["data"][i + 1]["cloudCover"]) * 100;
      weather[i].pressure = root["daily"]["data"][i + 1]["pressure"];
      weather[i].windSpeed = root["daily"]["data"][i + 1]["windSpeed"];
      weather[i].windBearing = root["daily"]["data"][i + 1]["windBearing"];
    }
  }

}
void cekWeather()
{
  for ( byte i = 0; i < 2; i++)
  {
    Serial.println(weather[i].minTemp);
    Serial.println(weather[i].maxTemp);
    Serial.println(weather[i].humidity);
    Serial.println(weather[i].cloud);
    Serial.println(weather[i].pressure);
    Serial.println(weather[i].windSpeed);
    Serial.println(weather[i].windBearing);
    Serial.println(weather[i].icon);
    Serial.println(weather[i].summary);
    delay(5000);
  }
}
void weatherCom()
{
  for (byte i = 0; i < 2; i++)

  {
    ET.begin(details(weather[i]), &Serial);
    unsigned long timeOut = millis();
    Serial.print("#WT"); Serial.println(i);
    while (1)
    {
      if (Serial.read() == '#')
      {


        //Send Data struct
        ET.sendData();
        break;

      }
      yield();
      if (millis() - timeOut >= 10000)break;
    }
    delay(50);

  }
  Serial.println("#WK");
  delay(2000);

}

