#include <Math.h>
#include "signalCatcher.h"
#include "MPU6050.h"


//critical for determining the size of sample space
static const int DELAYTIME = 2; 
int delaytime = 0;
bool isLocked = false;

signalCatcher sc;
MPU6050 mpu;

void setup() {
  Serial.begin(9600);
  mpu.setupMPU();
  sc.LockState=true;
  delay(100);  // Wait for sensor to stabilize
}

void loop() {
  sc.evalAverageRss(mpu.getData());
  modeMaster(1);
  delay(20);
}

void modeMaster(int mode){
  bool msg = false;

  if(mode == 1){
    Serial.print(sc.getAverageRss());
    Serial.print(",");
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

