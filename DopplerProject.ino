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
#include <SPI.h>
#include <SD.h>

// DECLARED PINS ////////////////////////////////////////////////
#define PIN_PHSENSOR A0
#define PIN_WATERLEVEL A1
#define PIN_TDSSENSOR A2
#define PIN_TEMPERATURE 57 //or A3
#define PIN_SDC 53
//TIME MODULE //////////////////////
// CLK -> 69 / A15 , DAT -> 68 / A14, Reset -> 67 / A13 //Analog pins converted to Digital Pins
virtuabotixRTC myRTC(69, 68, 67); // If you change the wiring change the pins here also

// Global variables //////////////////////////////////////////////////////////////
String DATA_DATETIME = ""; 
String DATA_FILENAME = ""; 
String DATA_TEXTLINE = "";

File DATA_FILE;
// SENSOR DATA ////////////////////////////////////////////////////////////////////
// WATERLEVEL //////////////////////
int DATA_WATERLEVEL = 0;
String STR_WATERLEVEL = "";
// PH LEVEL ////////////////////////
float DATA_PHLEVEL;
// TDS /////////////////////////////
GravityTDS gravityTds;
float DATA_TDS;
// TEMPERATURE /////////////////////
int DATA_TEMP = 0;
///////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  gravityTds.setPin(PIN_TDSSENSOR);
  gravityTds.setAref(5.0);  
  gravityTds.setAdcRange(1024);  
  gravityTds.begin();  

  // Set the current date, and time in the following format:
  // seconds, minutes, hours, day of the week, day of the month, month, year
  // myRTC.setDS1302Time(18, 28, 20, 6, 13, 4, 2024);
  /*
  Serial.print("Initializing SD card...");

  pinMode(PIN_SDC, OUTPUT);

  if (!SD.begin(PIN_SDC)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  delay(3000);

  */
}

void loop() {
  // Get current time
  getTime();
  Serial.println(DATA_FILENAME);
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
  get_TDS();
  Serial.print("TDS: ");
  Serial.print(DATA_TDS, 0);
  Serial.println(" ppm");
  
  TaskController();
  Serial.println(DATA_TEXTLINE);

  Serial.println("----------------------------------------");
  delay(1000);


}

// TIME BASED ACTIONS /////////////////////////////////////////////////
void getTime() {
  myRTC.updateTime(); 
  DATA_DATETIME = myRTC.year; 
  DATA_DATETIME.concat("/");
  DATA_DATETIME.concat(myRTC.month);
  DATA_DATETIME.concat("/");
  DATA_DATETIME.concat(myRTC.dayofmonth);
  DATA_DATETIME.concat(" ");
  DATA_DATETIME.concat(myRTC.hours);
  DATA_DATETIME.concat(":");
  DATA_DATETIME.concat(myRTC.minutes);
  DATA_DATETIME.concat(":");
  DATA_DATETIME.concat(myRTC.seconds);
  

  DATA_FILENAME = "DATA_";
  DATA_FILENAME.concat(myRTC.year); 
  DATA_FILENAME.concat("_");
  DATA_FILENAME.concat(myRTC.month);
  DATA_FILENAME.concat("_");
  DATA_FILENAME.concat(myRTC.dayofmonth);
  DATA_FILENAME.concat(".txt");
}

int current_minute = 0; 

void TaskController() {

  // Compile Data to TextLine /////////////////////////////////////////
  // Compiled Data Structure:
  //  DATE_TIME waterlevel;temperature;ph_level;tds_level;
  // Sample:
  // 2024/4/27 10:4:10 365:MID;35C;6.30PH;911.51PPM
  DATA_TEXTLINE = DATA_DATETIME;
  DATA_TEXTLINE.concat(" ");
  DATA_TEXTLINE.concat(DATA_WATERLEVEL);
  DATA_TEXTLINE.concat(":");
  DATA_TEXTLINE.concat(STR_WATERLEVEL);
  DATA_TEXTLINE.concat(";");

  DATA_TEXTLINE.concat(DATA_TEMP);
  DATA_TEXTLINE.concat(" C");
  DATA_TEXTLINE.concat(";");

  DATA_TEXTLINE.concat(DATA_PHLEVEL);
  DATA_TEXTLINE.concat(" PH");
  DATA_TEXTLINE.concat(";");

  DATA_TEXTLINE.concat(DATA_TDS);
  DATA_TEXTLINE.concat(" PPM");

  // Save Data every 5 Minutes based on Time. /////////////////////////
  // e.g 10:00, 10:05, 10:10 etc..
  if (myRTC.minutes % 5 == 0 && myRTC.minutes != current_minute) {
    current_minute = myRTC.minutes; //used to save data only once
    Serial.println("****TRIGGER MINUTES");
    //Save Data to SD Card
    //SaveData();
  } 
}

void SaveData() {
   File dataFile = SD.open(DATA_FILENAME, FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(DATA_TEXTLINE);
      dataFile.close();
      Serial.println("***SAVED TO SD CARD");
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
    }
}

// SENSORS ////////////////////////////////////////////////////////////
void getWaterLevel() {
  DATA_WATERLEVEL = analogRead(PIN_WATERLEVEL);
  if (DATA_WATERLEVEL == 0) {
    STR_WATERLEVEL = "EMPTY";
  } 
  else if (DATA_WATERLEVEL > 1 && DATA_WATERLEVEL < 350) {
    STR_WATERLEVEL = "LOW";
  } 
  else if (DATA_WATERLEVEL > 350 && DATA_WATERLEVEL < 400) {
    STR_WATERLEVEL = "MID";
  } 
  else if (DATA_WATERLEVEL > 450){
    STR_WATERLEVEL = "FULL";
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

void get_TDS(){
  gravityTds.setTemperature(DATA_TEMP);  
  gravityTds.update();  
  DATA_TDS = gravityTds.getTdsValue();  
}
