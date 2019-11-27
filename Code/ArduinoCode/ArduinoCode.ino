#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Program: Sound Level Measurement   
   
int num_Measure = 128 ; // Set the number of measurements for adc  
int pinSignal_Sound = A0; // pin connected to pin O module sound sensor   
int pinSignal_treshold = A1; // pin connected to pin 1 for analog pot   
//int pin_LED = 20;
int Pin_TableNoUp = 7;
int Pin_TableNoDown = 8;
int Pin_Reset = 13;

long Sound_signal;    // Store the value read Sound Sensor   
long sum = 0 ; // Store the total value of n measurements   
long level = 0 ; // Store the average value   
int Sound_threshold;
int dB;
int TableNo = 0;
int Avg;
int It = 0;
int cal = 0;

int redPin = 6;
int greenPin = 9;
int bluePin = 10;//Not being Used
bool editmode;
long current_password;
long saved_password;
int c_length = 1;
bool reset_password;
bool enter_password;
int wait_timer;
int reset_timer;

int minAvg = 0;
int minItr = 0;
int minCal = 0;

int SigPin = 10;
int Occupancy = 0;
float invcmCosnt = (2*1000000)/(100*344.8); //cmDist
void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.  
  pinMode (pinSignal_Sound, INPUT); // Set the Mic signal pin as input   
  pinMode (pinSignal_treshold, INPUT); // Set the Analog pot signal pin as input  
  //pinMode( pin_LED, OUTPUT);
  pinMode( Pin_TableNoUp, OUTPUT);
  pinMode( Pin_TableNoDown, OUTPUT);
   pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin (9600);
  editmode = false;
  ResetasNew();
}

void loop ()  
{  
  if(digitalRead(Pin_Reset) == HIGH)// If Reset Button is pressed every value goes to default values
  {
    ResetasNew();// The default or factory values of the device
  }
  LedReset();// Turning Anode tri-color LED OFF
  ReadAnalogMic();  // Reading and calibrating the current dB value
  Checkingthreshold();// Checks the current value with threshold value
  if(reset_password)// if the conditions for resetting password are set then change the password 
  {
    ResetPassword();
  }
  else
  {
  if(editmode)
  {
    //ReadAnalogPot();// Reading and calibrating values to set the threshold dB value 
    SetTable();//Set a table number for different devices to distinguish between devices
  }
  else
  {
    Password();// Reading the password to log into edit mode
  }  
  }      
  PrintLCD();// Printing in LCD  
  ReadingAvg(); // All the calculations and timers
  ReadOccupancy();
  ReadFromWiFi();
  SendToWiFi();
  //Debug();// debugging to see if everything is working as expected in the Serial Monitor
  delay(10);
}

void ReadOccupancy()
{
  pinMode(SigPin, OUTPUT);
  digitalWrite(SigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(SigPin, HIGH); 
  delayMicroseconds(5); 
  digitalWrite(SigPin, LOW);
  pinMode(SigPin, INPUT); 
  float rawTime = pulseIn(SigPin, HIGH); //measured in u-seconds 
  float cmDist = rawTime/invcmCosnt; 
  if(cmDist < 12)
  {
    Occupancy = 1;
  }
  else
  {
    Occupancy = 0;
  }
}
void ReadFromWiFi()
{
  if(Serial.available())
  {
    String data = Serial.readStringUntil('\n');
     data = getValue(data,';',1);
     if(data != ""){
     int newdata = data.toInt();
     if(newdata > 30 && newdata < 140)
     {
      Sound_threshold = newdata;
     }
     }
  }
}
void SendToWiFi()
{
  String data_ = String(TableNo) + ";" + String(dB) + ";" + String(Sound_threshold) + ";" + String(Occupancy);
  Serial.println(data_);
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
void Debug()// debugging to see if everything is working as expected in the Serial Monitor
{
  Serial.print("Sound Level: ");
  Serial.println (dB);
  Serial.print("Sound Threshold: ");
  Serial.println (Sound_threshold); 
  Serial.print("Current Password: ");
  Serial.print(current_password);  
  Serial.print(" C_length: ");
  Serial.print(c_length);
  Serial.print(" wait_timer: ");
  Serial.println(wait_timer);
  Serial.print(" reset_timer: ");
  Serial.println(reset_timer);
}
void ResetasNew()// The default or factory values of the device
{
  TableNo = 0;
  Sound_threshold = 50;
  saved_password = 1001;
  c_length = 1;
}
void ResetPassword()// if the conditions for resetting password are set then change the password 
{
  reset_timer = reset_timer + 1;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Reset Password");
  lcd.setCursor(0,1);
  lcd.print(current_password);
  if(digitalRead(Pin_TableNoUp) == HIGH&&digitalRead(Pin_TableNoDown) == HIGH)
  { 
    
      saved_password = current_password;
      reset_password = false;
      editmode = false;
      reset_timer = 0;   
      if(current_password == 0)
      {        
        digitalWrite(redPin,LOW);//display Green if correct and red if not
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Password Removed"); 
      }
      else
      {
        digitalWrite(greenPin,LOW);//display Green if correct and red if not
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Reset Done!");
      }
      Sound_threshold = 50;
      c_length = 1;      
      current_password = 0;
      while(digitalRead(Pin_TableNoUp) == HIGH&&digitalRead(Pin_TableNoDown) == HIGH)
    {//this loop is for getting input from only once after pressing the button    
    }  
  }
  else
  {    
      if(digitalRead(Pin_TableNoUp) == HIGH)
      {
        reset_timer = 0;   
        current_password = c_length * current_password + 1;
        c_length = 10;   
        while(digitalRead(Pin_TableNoUp) == HIGH)
        {//this loop is for getting input from only once after pressing the button
        }
      }
      if(digitalRead(Pin_TableNoDown) == HIGH)
      {
        reset_timer = 0;   
        current_password = c_length * current_password + 0;
        c_length = 10;
      while(digitalRead(Pin_TableNoDown) == HIGH)
        {//this loop is for getting input from only once after pressing the button
        }
      }    
  }
}
void Password()// Reading the password to log into edit mode
{
  if(digitalRead(Pin_TableNoUp) == HIGH&&digitalRead(Pin_TableNoDown) == HIGH)
  {
    
        if(!enter_password)
        {
          enter_password = true;
          current_password = 0;
          c_length = 1;
        }
        else
        {          
          if(current_password == saved_password)//check paassword
          {
            current_password = 0;
            editmode = true;
            digitalWrite(greenPin,LOW);//display Green if correct and red if not
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Correct Password");
            enter_password = false;
            c_length = 1;
          }
          else
          {
            editmode = false;
            digitalWrite(redPin,LOW);
            current_password = 0;
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Wrong Password");
            enter_password = false;
            c_length = 1;
          }
          
        }
          while(digitalRead(Pin_TableNoUp) == HIGH&&digitalRead(Pin_TableNoDown) == HIGH)
      {//this loop is for getting input from only once after pressing the button  
      }
      wait_timer = 0;
  }
  else
  {    
    if(enter_password)
    {
      wait_timer = wait_timer + 1;
      if(digitalRead(Pin_TableNoUp) == HIGH)
      {
        
        current_password = c_length * current_password + 1;
        c_length =  10;  
        wait_timer = 0;    
        while(digitalRead(Pin_TableNoUp) == HIGH)
        {//this loop is for getting input from only once after pressing the button
        }
      }
      if(digitalRead(Pin_TableNoDown) == HIGH)
      {
        
        current_password = c_length * current_password + 0;
        c_length = 10;    
        wait_timer = 0;  
        while(digitalRead(Pin_TableNoDown) == HIGH)
        {//this loop is for getting input from only once after pressing the button
        }
      } 
    }   
  }
}

void LedReset()// Turning Anode tri-color LED OFF
{
  digitalWrite(bluePin,HIGH);
  digitalWrite(greenPin,HIGH);
  digitalWrite(redPin,HIGH);
}

void ReadingAvg()// All the calculations and timers
{
  It = It + 1;
  cal = cal + dB;
  if(It == 10)
  {
    Avg = cal/It;
    cal = 0;
    It = 0;   
    minCal +=Avg;
    minItr += 1; 
  }

  if(minItr == 5)
  {
    minAvg = minCal/minItr;
    minCal = 0;
    minItr = 0;
  }
  if(wait_timer == 40)//if noting is clicked in enter password screen for 4s 
  {
    wait_timer = 0;
    enter_password = false;
    current_password = 0;
    c_length = 1;
  }
  if(reset_timer == 40)//if noting is clicked in reset password screen for 4s 
  {
    reset_timer = 0;
    reset_password = false;
    current_password = 0;
    c_length = 1;
  }
}

void Checkingthreshold()// Checks the current value with threshold value
{
  if(editmode)
  {
    digitalWrite(greenPin,LOW);//if edit mode is on turns the led green
  }else
  {
  if(Avg>Sound_threshold)
  {
    digitalWrite(redPin,LOW);// if edit mode is off then checks the current dB value with the set threshold value
    if(minAvg > Sound_threshold)
    {
      SendToWiFi();
      minAvg = 0;
    }
  }
  }
}
void SetTable()//Set a table number for different devices to distinguish between devices
{
  if(digitalRead(Pin_TableNoUp) == HIGH&&digitalRead(Pin_TableNoDown) == HIGH)
  {
   
    if(Sound_threshold == 0 && TableNo == 0)
    {
      reset_password = true;
      current_password = 0;
    }
    else
    {
    editmode = false;
    current_password = 0;  
    reset_password = false; 
    } 
     while(digitalRead(Pin_TableNoUp) == HIGH&&digitalRead(Pin_TableNoDown) == HIGH)  
    {//this loop is for getting input from only once after pressing the button
    }
  }
  else
  {
      if(digitalRead(Pin_TableNoUp) == HIGH)
      {
        
          TableNo = TableNo + 1;      
          while(digitalRead(Pin_TableNoUp) == HIGH)
        {//this loop is for getting input from only once after pressing the button
        }
      }
      if(digitalRead(Pin_TableNoDown) == HIGH)
      {
        
          TableNo = TableNo - 1;      
          while(digitalRead(Pin_TableNoDown) == HIGH)
        {//this loop is for getting input from only once after pressing the button
        }
      }
      if(TableNo < 0)
      {
        TableNo = 0;
      }  
  }  
}
void ReadAnalogMic()// Reading and calibrating the current dB value
{
  sum = 0 ; // Reset the sum of the measurement values
// Performs 128 signal readings   
  for ( int i = 0 ; i <num_Measure; i ++)  
  {  
   Sound_signal = analogRead (pinSignal_Sound);  
    sum =sum + Sound_signal;  
  }  
  
  level = sum / num_Measure; // Calculate the average value
  dB = (level+680) / 12.5; //Convert ADC value to dB using Regression values   
  
    
   
}

void ReadAnalogPot()// Reading and calibrating values to set the threshold dB value 
{
  sum = 0 ; // Reset the sum of the measurement values 
  
  for ( int i = 0 ; i <num_Measure; i ++)  
  {  
   Sound_threshold = analogRead (pinSignal_treshold);  
    sum =sum + Sound_threshold;  
  }  

  sum = sum/num_Measure;
  Sound_threshold = 0 + (0.1368  * sum);
  
}
void PrintLCD()// Printing in LCD
{
  if(!reset_password)
  {
  if(!enter_password)
  { 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TbNo:");
    lcd.setCursor(5, 0);
    lcd.print(TableNo);
  
    lcd.setCursor(8, 0);
    lcd.print("Th:");
    lcd.setCursor(11, 0);
    lcd.print(Sound_threshold);
    
    lcd.setCursor(0, 1);
    lcd.print("Cu:");  
    lcd.setCursor(3, 1);
    lcd.print(dB);
    
    lcd.setCursor(7,1);
    lcd.print("Avg:");
    lcd.setCursor(11,1);
    lcd.print(Avg);  
  }else
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Enter Password");
    lcd.setCursor(0,1);
    lcd.print(current_password);
  }
  if(editmode)
  {
    lcd.setCursor(14,0);
    lcd.print("Ed");
  }
  }
}
