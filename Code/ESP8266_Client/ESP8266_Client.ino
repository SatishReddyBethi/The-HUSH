/*
Geekstips.com
IoT project - Communication between two ESP8266 - Talk with Each Other
ESP8266 Arduino code example
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
//#include <SoftwareSerial.h>  //header file of software serial port
// IoT platform Credentials
String apiKey = "PUO4H7WC95YNKXNZ";
const char* logServer = "api.thingspeak.com";

// Internet router credentials
const char* ssid = "iPhone";
const char* password = "esdee1234";
const char* host = "http://172.20.10.13/threshold";
const char* serverHost = "http://172.20.10.13/";
ESP8266WebServer server(80);

String Table_No = "";
String dB_level = "";
bool Verbose_Logging = false;
int StartTime = 0;
int Timer = 16;
String Threshold = "50";
String Occupancy = "";
//SoftwareSerial Serial1(2,3); //define software serial port name as Serial1 and define pin2 as RX and pin3 as TX
String dist = ""; //actual distance measurements of LiDAR
  
void setup() {
  Serial.begin(9600); //set bit rate of serial port connecting Arduino with computer
  Setup_WiFi_Internet();
  StartTime = millis();
}

void loop() {
    setupStMode();
    Timer = millis() - StartTime;
    ReadfromSerial();
    SendDatatoServerandRecieve();
    if(WiFi.status() != WL_CONNECTED)
  {
    Setup_WiFi_Internet();
  }   
  Serial.println(";" + Threshold + ";");
  //delay(10);
  //sendHttpRequest();//To send arguments to the server
}

void Setup_WiFi_Internet()
{
  WiFi.mode(WIFI_STA);
  if(Verbose_Logging)
  {
  Serial.println("** SETUP STATION MODE **");
  Serial.println("- disconnect from any other modes");
  }
  WiFi.disconnect();
  if(Verbose_Logging)
  {
  Serial.println();
  Serial.print("- connecting to Home Router SID: ");
  Serial.println(ssid);
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if(Verbose_Logging)
    {
      Serial.print(".");
    }
  }
  if(Verbose_Logging)
  {
  Serial.println();
  Serial.println("- succesfully connected");
  }
}

void setupStMode(){

    
  if(dB_level != "" && Table_No != ""&& Timer > 5000){
  WiFiClient client;
  if(Verbose_Logging)
  {
  Serial.println("- starting client");
  Serial.println("- connecting to Database server: " + String(logServer));
  }
  if (client.connect(logServer, 80)) {
    if(Verbose_Logging)
  {
    Serial.println("- succesfully connected");
  }
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += Table_No;
    postStr += "&field2=";
    postStr += dB_level;
    //postStr += "&field3=";
    //postStr += Occupancy;
    postStr += "\r\n\r\n";
    if(Verbose_Logging)
  {
    Serial.println("- sending data...\n Table No: " + Table_No + " dB Level: " + dB_level);
  }
  client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Table_No = "";
    dB_level = "";
  }
  //StartTime = millis();
  //Timer = 0;
  
  client.stop();
  if(Verbose_Logging)
  {
  Serial.println("- stopping the client");
  }
  }
  /** If your ESP does not respond you can just
  *** reset after each request sending 
  Serial.println("- trying reset");
  ESP.reset();
  **/
  /*if(Timer > 100)//1000 is 1 second
  {
    Timer = millis();
    //return;
  }*/
}

void SendDatatoServerandRecieve(){
  WiFiClient client;
  HTTPClient http;
  if(Verbose_Logging)
  {
  Serial.printf("\n[Connecting to %s ... ", host);
  }
  if (http.begin(client, host))  // HTTP
  {
    if(Verbose_Logging)
  {
    Serial.print("[HTTP] GET...\n");
  }
      // start connection and send HTTP header
      int httpCode = http.GET();
   // httpCode will be negative on error
      if (httpCode > 0) {
        if(Verbose_Logging){
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        }
  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Threshold = payload;
          if(Verbose_Logging){
          Serial.println(payload);
          }
        }
      } else {
        if(Verbose_Logging){
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      }

      http.end();
    } else {
      if(Verbose_Logging){
      Serial.printf("[HTTP} Unable to connect\n");
      }
    }
}
void sendHttpRequest() {
  HTTPClient http;
  http.begin(serverHost);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String data = "Tbl=" + Table_No;
  data += "&Occ=" + Occupancy;
  http.POST(data);
  //http.writeToStream(&Serial);//Send Data Recieved to Serial Port
  http.end();
}
void ReadfromSerial(){
  if(Serial.available()){
    String data = Serial.readStringUntil('\n');
    Table_No = getValue(data,';',0);
    dB_level = getValue(data,';',1);
    //Occupancy = getValue(data,';',3);
    if(Verbose_Logging){
    Serial.println("Read Data - Table No: " + Table_No + " dB Level: " + dB_level);// + "Occupancy: " + Occupancy);
  }
  }
}

String getValue(String data, char separator, int index)//Getting Value from the string
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
