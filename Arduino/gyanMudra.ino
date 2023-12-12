//ME 313 Project: Gyan Mudra Detection
//Leonard Farrell
//Created: November 13 2023
//Updated: November 19 2023
//Updated: December 04 2023

//Algorithm
// 1. Sample index and middle finger flex sensor
// 2. Evaluate if either finger moved in or out of position
// 3. If both fingers are in position activate the session trigger
//    - If currently in session -> end the session + vibrate
//        - While in session record the current session time
//    - If currently out of session -> begin the session + vibrate

#include <Wire.h>
#include "Adafruit_DRV2605.h"

//Initialize Variables For:

//Vibration
Adafruit_DRV2605 drv;
uint8_t effect = 1;

//Flex Data Sampling 
long tPrevCollect;
float flexValueNowIndex;
float flexValueNowMiddle;
float f=3.0; //System Frequency in Hz
float T = 1/f; //System Period (1/System Frequency)
float TS = (T/20)*1000; //Sample Period (1/Sample Frequency)

//Session Triggering Logic
long tStartSession, tNowInSession;
bool inSession=false;
bool inPositionNowIndex, inPositionPrevIndex=0;
bool inPositionNowMiddle, inPositionPrevMiddle=0;
bool indexReady=0, middleReady=0;

//General
long tNow;

void setup() {
  
  Serial.begin(9600);    

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

  //-----------------------------------------------Flex Sensor Setup End-----------------------------------------------
}

void loop() {
  
  tNow=millis();

  //---------------------------------------------Read Flex Sensor Data Begin-------------------------------------------  
  
  if(tNow-tPrevCollect>=TS ){
    tPrevCollect=tNow;
    flexValueNowIndex=analogRead(A1);
    flexValueNowMiddle=analogRead(A2);    
  }  

  //----------------------------------------------Read Flex Sensor Data End--------------------------------------------
  //
  //
  //--------------------------------------------Gesture Evaluation Part 1 Begin----------------------------------------  
  
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

  inPositionPrevIndex=inPositionNowIndex;
  inPositionPrevMiddle=inPositionNowMiddle;

  //---------------------------------------------Gesture Evaluation Part 1 End-----------------------------------------


  //--------------------------------------------Gesture Evaluation Part 2 Begin----------------------------------------  

  if(indexReady && middleReady){
    //Serial.println("                                                   MOVED INTO POSITION");    
    effect=16;
    drv.setWaveform(0, effect);  // set effect
    drv.setWaveform(1, 0);       // set waveform
    drv.go();                    // play effect
    inSession=!inSession;
    indexReady=0;
    middleReady=0;
    
    //If on session trigger, the inSession boolean toggled to inSession
    //meaning the system state was previously out of session, in other words the session has begun
    if(inSession){
      Serial.println("                                                 BEGIN SESSION");
      //Start session timer
      tStartSession=millis();
      
      //After 10 seconds play interval vibration effect
      //Every 1 minute after play interval vibration effect      
    }
    if(!inSession){
      Serial.println("                                                 END SESSION");    
    }    
  }  

  //---------------------------------------------Gesture Evaluation Part 2 End-----------------------------------------

  //Record time in current session
  if(inSession){
    tNowInSession=tNow-tStartSession;
    Serial.println("Time in Session: " + String(tNowInSession));
  }
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
