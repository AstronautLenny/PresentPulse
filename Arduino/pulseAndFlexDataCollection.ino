//ME 314 Project: Present Pulse and Flex Data Collection
//Leonard Farrell
//Created 11-19-2023
//Revised 11-28-2023
//Revised 12-2-2023

#include "MAX30105.h"             //MAX30105 Pulse Sensor Library
//#include "heartRate.h"            //Library Containing checkForBeat() function
#include <Wire.h>
#include "Adafruit_DRV2605.h"

MAX30105 particleSensor;          //Initialize MAX30105 Pulse Sensor

long irValue;                     //Value Read by Pulse Sensor 
long tNow, tLast;                  //Used for interval checking
bool beatSpotted;                 //Boolean representing if Beat is Found During IR Reading

float sysFreq=3; //3Hz
float sampleFreq= 20;//sysFreq*20;
//float sampleFreq=15.5;
float samplePeriod=(1/sampleFreq)*1000; //milliseconds

//With sample Freq at 15.5hz loop time is 1ms
// With sample freq at 60 hz loop time is 21ms ?????

long tPrevCollect;
float f=20;//3.0; //System Frequency in Hz
float T = 1/f; //System Period (1/System Frequency)
float TS = T;//(T/20)*1000; //Sample Period (1/Sample Frequency)

float flexValueNowMiddle,flexValueNowIndex;
bool inPositionNowIndex, inPositionPrevIndex=0;
bool inPositionNowMiddle, inPositionPrevMiddle=0;
bool indexReady=0, middleReady=0;

Adafruit_DRV2605 drv;
uint8_t effect;

bool inSession=false;

long tStartSession, tNowInSession;


void setup(){
  
  //Serial.begin(9600);           //Set Baud Rate
  Serial.begin(115200);           //Set Baud Rate
  Serial.println(sampleFreq);
  Serial.println(samplePeriod);

  //-----------------------------------------MAX30105 Pulse Sensor Setup ---------------------------------------------
  
  Serial.println("\nInitializing MAX30105...");  
  
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)){   //Initialize Sensor. Use default I2C port, 400kHz speed
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }  
  
  Serial.println("...MAX30105 initialized.");  
  
  particleSensor.setup();                             //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A);          //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);           //Turn off Green LED  

  //-----------------------------------------------------------------------------------------------------------------
  //-----------------------------------------------DRV2605 Setup Begin-------------------------------------------------
  
  Serial.println("Initializing DRV2605...");
  
  if (! drv.begin()) {
    Serial.println("Could not find DRV2605");
    while (1) delay(10);
  }
  Serial.println("...DRV2605 inititalized.");
  
  drv.selectLibrary(1);  
  drv.setMode(DRV2605_MODE_INTTRIG); 

  //------------------------------------------------DRV2605 Setup End--------------------------------------------------

  
  //----------------------------------------------Flex Sensor Setup Begin----------------------------------------------
  
  Serial.println("Initializing Flex Sensors...");  

  pinMode(A1, INPUT); //Index
  pinMode(A2, INPUT); //Middle

  Serial.println("...Flex Sensors initialized."); 
  //Serial.println(vibInterval);

  //-----------------------------------------------Flex Sensor Setup End-----------------------------------------------
   
  Serial.println("time IRValue BeatSpotted");         //Display Column Headings for Data Printed in Loop  
}

void loop(){

  tNow=millis();                        //Set to Current Time on Arduino Interal Clock 

  //------------------------------------------------Read Sensors------------------------------------------------------
  
  if (tNow-tLast>=samplePeriod){
      if(!inSession){
    irValue=0;
    tNowInSession=0;
  }
  
  String dataString="";
  dataString+=String(tNow);
  dataString+=',';
  dataString+=String(irValue);
  dataString+=',';
  dataString+=String(flexValueNowIndex);
  dataString+=',';
  dataString+=String(flexValueNowMiddle);
  dataString+=',';
  dataString+=String(tNowInSession);  

    Serial.println(dataString);
    tLast=tNow;    
    irValue = particleSensor.getIR();   //Read IR Value From Sensor
  }
  
  if(tNow-tPrevCollect>=TS ){
    tPrevCollect=tNow;
    flexValueNowIndex=analogRead(A1);
    flexValueNowMiddle=analogRead(A2);    
  }  
  
  //-----------------------------------------------------------------------------------------------------------------
  //
  //
  //--------------------------------------------Gesture Evaluation Part 1------------------------------------------------  
  
  //Index > 400
  //Middle < 310
    
  inPositionNowIndex=checkInPosition(flexValueNowIndex, 400, 1);
  inPositionNowMiddle=checkInPosition(flexValueNowMiddle, 310, -1);
  
  //With this logic the user must take both fingers out of position and bring them back in to position to activate the trigger
  if(!inPositionPrevIndex && inPositionNowIndex){
    indexReady=1;
  }
  else if(inPositionPrevIndex && !inPositionNowIndex){
    indexReady=0;
  }

  if(!inPositionPrevMiddle && inPositionNowMiddle){
    middleReady=1;  
  }
  else if(inPositionPrevMiddle && !inPositionNowMiddle){
    middleReady=0;  
  }
  
  //-----------------------------------------------------------------------------------------------------------------
  //
  //
  //--------------------------------------------Gesture Evaluation Part 2 Begin----------------------------------------  

  if(indexReady && middleReady){
    //Serial.println("                                                   MOVED INTO POSITION");    
    effect=16;
    drv.setWaveform(0, effect);  // play effect 
    drv.setWaveform(1, 0);       // end waveform
    drv.go();
    inSession=!inSession;
    indexReady=0;
    middleReady=0;
    
    //If on session trigger, the inSession boolean toggled to inSession, meaning the system state was previously out of session, in other words the session has begun
    if(inSession){
      //Serial.println("                                                 BEGIN SESSION");
      //Start session timer
      tStartSession=millis();
      //Begin reading and saving IR Value to SD

      //After 10 seconds play interval vibration effect
      //Every 1 minute after play interval vibration effect      
    }
    if(!inSession){
      //Serial.println("                                                 END SESSION");
      //Stop reading and saving IR Value to SD      
    }    
  }
   
  inPositionPrevIndex=inPositionNowIndex;
  inPositionPrevMiddle=inPositionNowMiddle;
  
  //Record time in current session
  if(inSession){
    tNowInSession=tNow-tStartSession;
    //Serial.println("Time in Session: " + String(tNowInSession));
  }
    
  //-----------------------------------------------------------------------------------------------------------------
  //
  //
  //-------------------------------------------Write Data To Serial ------------------------------------------------- 
/*
  if(!inSession){
    irValue=0;
    tNowInSession=0;
  }
  
  String dataString="";
  dataString+=String(tNow);
  dataString+=',';
  dataString+=String(irValue);
  dataString+=',';
  dataString+=String(flexValueNowIndex);
  dataString+=',';
  dataString+=String(flexValueNowMiddle);
  dataString+=',';
  dataString+=String(tNowInSession);  
*/
  //Serial.println(dataString);

  long tEnd = millis();
  //Serial.println("Loop Time: " + String(tEnd-tNow));

  //Largest loop time:
  // baud 9600: 21
  //baud 115200: 3
  //Lets use baud 115200!!!!!!!

  //-----------------------------------------------------------------------------------------------------------------
  //
  //  
}

bool checkInPosition(float flexValue, float threshold, int evalDirection){
  bool inRange=false;

  if (evalDirection == 1){
    if (flexValue>=threshold){
    inRange=true;
    }
  }
  if (evalDirection == -1){
    if (flexValue<=threshold){
    inRange=true;
    }
  }
  
  return inRange;
}
