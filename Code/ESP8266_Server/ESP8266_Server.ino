/*
Geekstips.com
IoT project - Communication between two ESP8266 - Talk with Each Other
ESP8266 Arduino code example
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include <SoftwareSerial.h>  //header file of software serial port
// IoT platform Credentials
String apiKey = "PUO4H7WC95YNKXNZ";
const char* logServer = "api.thingspeak.com";
bool Verbose_Logging = true;
// Internet router credentials
const char* ssid = "iPhone";
const char* password = "esdee1234";

ESP8266WebServer server(80);

String Table_No = "";
int TblNo = 1;
String Ipu = "0";
String dB_level = "";
String Occupancy_ = "";//,"","","","","","","","",""};
int PrintTimer = 0;
int PrintStart = 0;
int StartTime = 0;
int Timer = 16;
String Threshold = "50";
String New_Threshold = "50";
//SoftwareSerial Serial1(2,3); //define software serial port name as Serial1 and define pin2 as RX and pin3 as TX

  
void setup() {
  Serial.begin(9600); //set bit rate of serial port connecting Arduino with computer
  StartTime = millis();
  PrintStart = millis();
  Setup_WiFi_Internet();
  //setupAccessPoint();
}

// Handling the / root web page from my server
void handle_index() {
  /*//String STbl = server.arg("Tbl");
  //String SOcc = server.arg("Occ");
 // int Tbl = STbl.toInt();
  //int Occ = SOcc.toInt();
  //if(Tbl < 10 && Tbl>-1)
  //{
    if(Occ == 1)
    {
    Occupancy_ = "Full";
    }
    else
    {
      Occupancy_ = "Empty";
    }
  //}*/
  String Info = "Current Threshold Value: " + New_Threshold + "\n\n";
  //for(int i = 0; i<10; i++)
  //{
    Info+= "Table No: " + Table_No;
    Info+= " is " + Occupancy_;
    Info+= "\n";
    
  //}
  server.send(200, "text/plain", Info);
}

// Handling the /feed page from my server
void handle_Threshold() {
  server.send(200, "text/plain", New_Threshold);
}

// Handling the /thresholdfeed page from my server
void handle_feed() {
  String Data = server.arg("thr");
  int ThData = Data.toInt();
  if(ThData > 30 && ThData < 140)
  {
  New_Threshold = Data;
  server.send(200, "text/plain", "Got the Threshold as: " + Data + ". This is response to client");
  }
  else
  {
    server.send(200, "text/plain", "Threshold value: " + Data + " is out of limits. Please enter value between 30 and 140");
  }
}
/*
void setupAccessPoint_(){
  Serial.println("** SETUP ACCESS POINT **");
  Serial.println("- disconnect from any other modes");
  WiFi.disconnect();
  Serial.println();
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  setupServer();
}
*/
void setupServer(){
  if(Verbose_Logging){
  Serial.println("** SETUP SERVER **");
  Serial.println("- starting server :");
  }
  if (MDNS.begin("esp8266")) {
    if(Verbose_Logging){
    Serial.println("MDNS responder started");
    }
  }
  server.on("/", handle_index);
  server.on("/threshold", handle_Threshold);
  server.on("/thresholdfeed", handle_feed);
  server.begin();
};

void loop() {
  server.handleClient();
  MDNS.update();
  PrintTimer = millis() - PrintStart;
  Timer = millis() - StartTime;
  ReadfromSerial();
  setupStMode();
  if(WiFi.status() != WL_CONNECTED)
  {
    Setup_WiFi_Internet();
  }
  if(PrintTimer > 100)
  {
  //Serial.println(New_Threshold);    
  PrintStart = millis();
  }
}


void Setup_WiFi_Internet()
{
  WiFi.mode(WIFI_STA);
  if(Verbose_Logging){
  Serial.println("** SETUP STATION MODE **");
  Serial.println("- disconnect from any other modes");
  }
  WiFi.disconnect();
  if(Verbose_Logging){
  Serial.println();
  Serial.print("- connecting to Home Router SID: ");
  Serial.println(ssid);
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if(Verbose_Logging){
    Serial.print(".");
    }
  }
  if(Verbose_Logging){
  Serial.println();
  Serial.println("- succesfully connected");
  }
  IPAddress myIP = WiFi.localIP();
  if(Verbose_Logging){
  Serial.print("- AP IP address is :");
  Serial.print(myIP);
  Serial.println();
  }
  setupServer();
}

void ReadfromSerial(){
  if(Serial.available()){
    String data = Serial.readStringUntil('\n');
    String Idx;
    Idx = getValue(data,';',0);
    dB_level = getValue(data,';',1);
    Threshold = getValue(data,';',2);//Only for Server
    TblNo = Table_No.toInt();
    Ipu = getValue(data,';',3);
    if(Idx != "")
    {
      Table_No = Idx;
    }
    if(Ipu.toInt() == 1)
    {
      Occupancy_ = "Full";
    }
    else if(Ipu.toInt() == 0)
    {
      Occupancy_ = "Empty";
    }
    if(Verbose_Logging){
    Serial.println("Read Data - Table No: " + Table_No + " dB Level: " + dB_level + " Threshold: " + Threshold + " Occupancy:" + Occupancy_);
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


void setupStMode(){

    
  if(dB_level != "" && Table_No != ""&& Timer > 5000){
  if(Verbose_Logging){
  Serial.println("- starting client");
  }
  WiFiClient client;
  if(Verbose_Logging){
  Serial.println("- connecting to Database server: " + String(logServer));
  }
  if (client.connect(logServer, 80)) {
    if(Verbose_Logging){
    Serial.println("- succesfully connected");
    }
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += Table_No;
    postStr += "&field2=";
    postStr += dB_level;
    postStr += "&field3=";
    postStr += Ipu;
    postStr += "\r\n\r\n";
    if(Verbose_Logging){
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
    dB_level = "";
  }
  StartTime = millis();
  Timer = 0;
  
  client.stop();
  if(Verbose_Logging){
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
