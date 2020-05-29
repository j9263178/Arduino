#include <Math.h>
#include "signalCatcher.h"
#include "Player.h"
#include "MPU6050.h"

//critical for determining the size of sample space
static const int DELAYTIME = 2; 
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
  sc.LockState = py.LockState;

  if(py.LockState){
    if (sc.alarm){
      py.alarm();
      for(int i =0 ; i<py.notein.size();i++){
        Serial.print(notein[i]);
      }
      sc.alarm=false;
    }
  }

  sc.evalAverageRss(mpu.getData());
  modeMaster(2);
  delay(20);
}

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

