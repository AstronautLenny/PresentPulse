//ME 313 Project: Present Pulse Pulse Data Collection
//Leonard Farrell
//Created November 19 2023
//Updated November 28 2023
//Revised December 04 2023

#include "MAX30105.h"             //MAX30105 Pulse Sensor Library
MAX30105 particleSensor;          //Initialize MAX30105 Pulse Sensor

long irValue;                     //Value Read by Pulse Sensor 
long tNow, tLast;                  //Used for interval checking

float sysFreq=3; //3Hz
float sampleFreq=sysFreq*20;
float samplePeriod=(1/sampleFreq)*1000; //milliseconds

void setup(){  

  Serial.begin(115200);           //Set Baud Rate  
  
  Serial.println("\nInitializing MAX30105...");    
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)){   //Initialize Sensor. Use default I2C port, 400kHz speed
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }    
  Serial.println("...MAX30105 initialized.");    
  particleSensor.setup();                             //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A);          //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);           //Turn off Green LED    
 
  Serial.println("time IRValue BeatSpotted");         //Display Column Headings for Data Printed in Loop  
  
}

void loop(){

  tNow=millis();                        //Set to Current Time on Arduino Interal Clock 
   
  if (tNow-tLast>=samplePeriod){
    tLast=tNow;    
    irValue = particleSensor.getIR();   //Read IR Value From Sensor
  }  
  
  String dataString="";
  dataString+=String(tNow);
  dataString+=',';
  dataString+=String(irValue);
  Serial.println(dataString);           //Print time and IR value to serial monitor
  
}
