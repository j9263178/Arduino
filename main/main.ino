
#include "Kalman.h"
#include <Wire.h>
#include <Math.h>
#include <SoftwareSerial.h>
#include <QueueArray.h>
#include "signalCatcher.h"
#include "Player.h"
#include "MPU6050.h"

int state = 0; // 0: locked, and ready to detect, 
               // 1: unlocked, no alarming or motion detections
static const int DELAYTIME = 2; //critical for determining the size of sample space
int delaytime = 0;

signalCatcher sc;
Player py;
MPU6050 mpu;

void setup() {
  Serial.begin(9600);
  py.setup();
  mpu.setupMPU();
  delay(100); // Wait for sensor to stabilize
}

void loop() {

  py.parse();

  if(py.LockState){
  
    if (sc.needSample){
      py.alarm();
    }

    sc.evalAverageRss(mpu.getData());
    
    Serial.println("");  

    modeMaster(2);

  }

  delay(10);
}

//**********************************************************************

void modeMaster(int mode){
  bool msg = false;

  if(mode == 1){
    Serial.print(sc.getAverageRss());
    Serial.print(F(","));
    Serial.println(sc.getTest());
  }

  else if(mode == 2){
    msg = true;
  }
  else if(mode == 3){
  }

  if(delaytime ++ >= DELAYTIME){
      sc.sample(msg);
      delaytime = 0;
  }
}

//**********************************************************************
