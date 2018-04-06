/*
  ESP_1 membuat server dan mengirim data dari _GET Request ke serial diawali dengan "#"
        aturan serial baca: rule_serial_COM.txt OK
  ESP_2 membuat akses point dan server sendiri dikirim ke DMD dengan baudrate "SANGAT RENDAH"
*/

#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <EEPROMAnything.h>

#define DEBUG false
#define led 2                                     // LED Indikator proses
//konek ke
String APSSID ;
String APPass ;
char ssid[20] = "x";
char pass[20] = "helpdesk";

byte setType ;
unsigned long last ;
boolean HTTPThink[4];


ESP8266WebServer server ( 80 );                   // port Internet yang dipakai


void handleNotFound() {
  digitalWrite ( led, 1 );
  String message = "404 :: No body hehe but checken ! \n try \"\ask\"\n\n";
  message += "\n";
  server.send ( 404, "text/plain", message );     // mengirim data 404 (halaman tidak ada) ke pengirim [fungsi debug]
  digitalWrite ( led, 0 );
}

void handleSentData() {
  digitalWrite ( led, 1 );
  if (DEBUG) Serial.print ( "#0 Data input \n" );
  String dataSerial = "#0";
  String cmd ;
  String data ;
  String message = "SENT DATA\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
    if (server.argName ( i ) == "cmd")                // mencari fungsi _GET "cmd"
    {
      dataSerial = "#1";                              // mengisi string dataSerial dengan "#1"
      cmd = server.arg(i);                            // isi command _GET diisikan ke string cmd
    }
    if (server.argName ( i ) == "data")               // mencari fungsi _GET "data"
    {
      data = server.arg(i);                           // isi command _GET diisikan ke string data
      //data += "<E>";
    }
    if (server.argName ( i ) == "newAPSSID")
    {
       APSSID=server.arg(i);
       setType=1;
    }
    if (server.argName ( i ) == "newAPPass")
    {
       APPass=server.arg(i);
    }
    if (server.argName ( i ) == "newSSID")
    { 
      server.arg(i).toCharArray(ssid,20);
      setType=2;
    }
    if (server.argName ( i ) == "newPass")
    { 
      server.arg(i).toCharArray(pass,20);
    }

        

  }
  server.send ( 200, "text/plain", message );         // mengirim data 200 (OK data diterima)ke pengirim  [fungsi debug]
  dataSerial += cmd;
  dataSerial += data;
  dataSerial += "\n";                                 // menyusun format dataSerial "#1<cmd><data>"
  Serial.print ( dataSerial );                        // mengirim dataSerial
  digitalWrite ( led, 0 );
}


void setup ( void ) {
  pinMode ( led, OUTPUT );
  digitalWrite ( led, 0 );                                // set GPIO sebagai indikator output
  Serial.begin ( 2400);                                   // komunikasi serial baudrate 2400 (stabil). lebih dari itu data rusak di penerima, kurang dari itu penerimaan lambat
  EEPROM.begin(512);
  delay(1000);                                            // menyiapkan serial
  if (DEBUG)Serial.println ( "#0 OK" );
  readAPSSID();
  char ss[20];
  char pp[20];
  APSSID.toCharArray(ss,20);
  APPass.toCharArray(pp,20);
  WiFi.softAP(ss, pp);
  IPAddress myIP = WiFi.softAPIP();
  if (DEBUG)
  {
    Serial.print("#0 AP IP address: "); Serial.println(myIP);
  }
  WiFi.begin(ssid, pass);
    while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
    Serial.print ( "LAN address: " );
  Serial.println ( WiFi.localIP() );

  server.on ( "/cek", []()
  {
    server.send ( 200, "text/html",
                  "<h1>SmartDisplay</h1> <h2>Sip server sudah jalan Joss..</h2> <p>OK</p>" );
  }
            );
  server.on ("/", handleSentData);                        // bila berada di index server
  server.onNotFound ( handleNotFound );                   // bila tiddak di index server
  server.begin();
  if (DEBUG) Serial.println ( "#0 HTTP server started" );
}

void loop ( void ) {
  
  unsigned long now = millis();
  if (now - last >= 15000) {
    // save the last time you blinked the LED
    last = now;
    ambilData();
  }
  server.handleClient();
  if(setType==1) { saveAPSSID(); setType=0;}
}

void saveAPSSID()
{
  for (int i=0;i<20;++i)
  {
    EEPROM.update(i+11, APSSID[i]);
    EEPROM.update(i+31, APPass[i]);
  //EEPROM_writeAnything((i+11), APSSID[i]);
  //EEPROM_writeAnything((i+31), APPass[i]);
  }
  Serial.println("Masuk Save");
  Serial.println(APSSID);
  Serial.println(APPass);
  ESP.restart();
}
void readAPSSID()
{
  for (int i=0;i<20;++i)
  {
    APSSID+=char(EEPROM.read(i+11));
    APPass+=char(EEPROM.read(i+31));
    
  //EEPROM_readAnything((i+11), APSSID[i]);
  //EEPROM_readAnything((i+31), APPass[i]);
  }
  Serial.println("Masuk Read");
  Serial.println(APSSID);
  Serial.println(APPass);
  
}
void ambilData ()
{
  Serial.println("HTTP ke ...");
   HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        // configure traged server and url
        //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
        http.begin("http://api.thingspeak.com/apps/thinghttp/send_request?api_key=3RTEKQUYUIEMDLI7"); //HTTP

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                payload.replace("Subtitle Indonesia", " ## ");
                Serial.println(payload);
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
}

