/**
==================================================================
The Doppler Project
==================================================================
Sensors:
- DFRobot PH Sensor
- DFRobot TDS Sensor
- Temperature Sensor
- Water Level Sensor
- DS1302 Time Module
- SD Card Module
- Sim800 Module

**/

#include <virtuabotixRTC.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include "GravityTDS.h"

// DECLARED PINS ////////////////////////////////////////////////
#define PIN_PHSENSOR A0
#define PIN_WATERLEVEL A1
#define PIN_TDSSENSOR A2
#define PIN_TEMPERATURE 57 //or A3

//TIME MODULE //////////////////////
// CLK -> 69 / A15 , DAT -> 68 / A14, Reset -> 67 / A13 //Analog pins converted to Digital Pins
virtuabotixRTC myRTC(69, 68, 67); // If you change the wiring change the pins here also

// Global variables //////////////////////////////////////////////////////////////
String DATA_DATETIME = ""; 
// //////////////////////
int DATA_WATERLEVEL = 0;
String STR_WATERLEVEL = "";
// //////////////////////
float DATA_PHLEVEL;
// //////////////////////
GravityTDS gravityTds;
// //////////////////////
int DATA_TEMP = 0;

void setup() {
  Serial.begin(9600);
  gravityTds.setPin(PIN_TDSSENSOR);
  gravityTds.setAref(5.0);  
  gravityTds.setAdcRange(1024);  
  gravityTds.begin();  

  // Set the current date, and time in the following format:
  // seconds, minutes, hours, day of the week, day of the month, month, year
  // myRTC.setDS1302Time(18, 28, 20, 6, 13, 4, 2024);
}

void loop() {
  // Get current time
  getTime();
  Serial.println(DATA_DATETIME);

  // Get water level
  getWaterLevel();
  Serial.print("WATER LEVEL VAL: ");
  Serial.print(DATA_WATERLEVEL);
  Serial.print(" STATUS: ");
  Serial.println(STR_WATERLEVEL);

  // Get pH level
  getPH_Level();
  Serial.print("PH_LEVEL: ");  
  Serial.println(DATA_PHLEVEL, 2);

  // Get temperature
  get_temperature();
  Serial.print("Temperature: ");
  Serial.print(DATA_TEMP);
  Serial.println("C");

  // Get TDS
  get_tds();
  
  Serial.println("----------------------------------------");
  delay(1000);
}

void getTime() {
  myRTC.updateTime(); 
  DATA_DATETIME = myRTC.hours;
  DATA_DATETIME.concat(":");
  DATA_DATETIME.concat(myRTC.minutes);
  DATA_DATETIME.concat(":");
  DATA_DATETIME.concat(myRTC.seconds);
  DATA_DATETIME.concat(" ");
  DATA_DATETIME.concat(myRTC.dayofmonth);
  DATA_DATETIME.concat("/");
  DATA_DATETIME.concat(myRTC.month);
  DATA_DATETIME.concat("/");
  DATA_DATETIME.concat(myRTC.year); 
}

void getWaterLevel() {
  DATA_WATERLEVEL = analogRead(PIN_WATERLEVEL);
  if (DATA_WATERLEVEL == 0) {
    STR_WATERLEVEL = "Empty";
  } 
  else if (DATA_WATERLEVEL > 1 && DATA_WATERLEVEL < 350) {
    STR_WATERLEVEL = "Gamay Nalang";
  } 
  else if (DATA_WATERLEVEL > 350 && DATA_WATERLEVEL < 400) {
    STR_WATERLEVEL = "Oks Ra";
  } 
  else if (DATA_WATERLEVEL > 450){
    STR_WATERLEVEL = "OPSZ";
  }   
}

void getPH_Level() {
  int buf[10], ph_temp;
  unsigned long int ph_avgValue = 0;
  
  for(int i = 0; i < 10; i++) { 
    buf[i] = analogRead(PIN_PHSENSOR);
    delay(10);
  }
  
  for(int i = 0; i < 9; i++) {        
    for(int j = i + 1; j < 10; j++) {
      if(buf[i] > buf[j]) {
        ph_temp = buf[i];
        buf[i] = buf[j];
        buf[j] = ph_temp;
      }
    }
  }
  
  for(int i = 2; i < 8; i++)                   
    ph_avgValue += buf[i];
  
  DATA_PHLEVEL = (float)ph_avgValue * 5.0 / 1024 / 6; 
  DATA_PHLEVEL = 3.5 * DATA_PHLEVEL;
}

//////////////////////////////////////////////////////////////////////
OneWire oneWire(PIN_TEMPERATURE);
DallasTemperature sensors(&oneWire);

void get_temperature(){
  sensors.requestTemperatures();
  DATA_TEMP = sensors.getTempCByIndex(0);
}

void get_tds(){
  gravityTds.setTemperature(DATA_TEMP);  
  gravityTds.update();  
  float tdsValue = gravityTds.getTdsValue();  
  Serial.print("TDS: ");
  Serial.print(tdsValue, 0);
  Serial.println(" ppm");
  delay(100);
}
