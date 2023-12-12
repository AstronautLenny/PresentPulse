//Double Tap Logic
//ME313 Project: Present Pulse
//Leonard Farrell
//Created: November 18 2023
//Updated: December 04 2023

//Plan
// 1. Sample flex data at sample rate
// 2. Evaluate flex value to determine if in range
// 3. If in range check if got out of range 500 ms later
// 4. If yes then record time of getting out of range
// 5. if time since last tap is still within tap interval 
//    - if another tap is registered print "Double tap!"

//Note: WORKS!! Took 3hrs :) Take that apple :P

#include <Wire.h>
#include "Adafruit_DRV2605.h"

//Vibration
Adafruit_DRV2605 drv;
uint8_t effect = 1;

//Sensor Sampling
float flexValueNowIndex;
float f=3.0; //System Frequency in Hz
float T=(1/f)*1000; //System Period (1/System Frequency)
float TS=T/20; //Sample Period (1/Sample Frequency)

//Event Triggering
long tPrevCollect,tapTime;
bool inPositionNowIndex,inPositionPrevIndex=0;
long tapInterval=750;
int tapCount;

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

  pinMode(A1, INPUT); //Index

}

void loop() {
  
  tNow=millis();

  if(tNow-tPrevCollect>=TS ){
    tPrevCollect=tNow;
    flexValueNowIndex=analogRead(A1); 
  }  

  //index>375

  inPositionNowIndex=checkInPosition(flexValueNowIndex, 375, 1);

  if(registerTap()){
    tapCount++;   
    //Serial.println("TAP COUNT " + String(tapCount));
  }
     
  if(tapCount==1){
    tapTime=millis();
    Serial.println("");
    Serial.println("TAP " + String(millis()));
    tapCount++; 
  }
      
  if(tNow-tapTime<=tapInterval){   
    if(tapCount==3){
      Serial.println("SECOND TAP " + String(millis()));
      effect=16;
      drv.setWaveform(0, effect);  // play effect 
      drv.setWaveform(1, 0);       // end waveform
      drv.go();
      tapCount=0;
    }
  }
  else{
    tapCount=0;
  }

  inPositionPrevIndex=inPositionNowIndex;
}

bool registerTap(){
  
  bool tapBool=0;
  bool entered,exited;
  long enterPositionTime, exitPositionTime;

  if(!inPositionPrevIndex && inPositionNowIndex && !entered){
    enterPositionTime=millis();
    entered=1;
    //count++;
    //Serial.println("                                                   MOVED INTO POSITION " +String(count));
  }
  if(inPositionPrevIndex && !inPositionNowIndex && !exited){
    exitPositionTime=millis();
    exited=1;
    //count++;
    //Serial.println("                                                   MOVED OUT OF POSITION " +String(count));
  }
  
  if(exitPositionTime-enterPositionTime>=100){
    //count++;
    //Serial.println("TAP "+String(count));        
    tapBool=1;
  }
  
  return tapBool;
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
