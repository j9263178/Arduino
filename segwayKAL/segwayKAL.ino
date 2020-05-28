/* 
 *  Kalman filter is implemented by 
 Kristian Lauszus, TKJ Electronics
 Web      :  http://www.tkjelectronics.com
 e-mail   :  kristianl@tkjelectronics.com

202004
 CYK modify the code for the balanced car
 */

#include <WSWire.h>
#include <SparkFun_TB6612.h>
#include "Kalman.h"
#include <PID_v1.h>

// Setting the pin for motor
#define AIN1 8   // 控制輸入A1
#define AIN2 7   // 控制輸入A2
#define BIN1 10  // 控制輸入B1
#define BIN2 9   // 控制輸入B2
#define PWMA 6
#define PWMB 5  
#define STBY 11
//###########################################
// parameters for motor:
const int offsetA = -1;  // 馬達A正反轉設定，可能值為1或-1。
const int offsetB = -1;  // 馬達B正反轉設定，可能值為1或-1。
const int maxPower = 125; // 最大輸出強度0-255
//const double balancingFactor = 1.04; //左右輪輸出強度調整


//Define PID Variables
double defaultSetPoint = 94.5;
double defaultKp = 40., defaultKi = 1200, defaultKd = 0.8;

// MPU6050 offset: the value you get from the MPU6050_calibration
// -2255.37 836.31  -1539.85  265.65  -99.90  -44.07
const double OffAccX  = -2255.37;
const double OffAccY  = 836.31 ;
const double OffAccZ  = -1539.85;
const double OffgyroX = 265.65 ;
const double OffgyroY = -99.90  ;
const double OffgyroZ = -44.07;
//###########################################


Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

// kalman filter for angle
Kalman kalmanX; // Create the Kalman instances

/* IMU Data */
double accX, accY, accZ;
double gyroX, gyroY, gyroZ;
int16_t tempRaw;

double gyroXangle; // Angle calculate using the gyro only
double compAngleX; // Calculated angle using a complementary filter
double kalAngleX; // Calculated angle using a Kalman filter

uint32_t timer;
uint8_t i2cData[14]; // Buffer for I2C data


double angleMeasured, angleTarget=defaultSetPoint, angleOutput;
double Kp = defaultKp, Ki = defaultKi, Kd = defaultKd;
PID anglePid(&angleMeasured, &angleOutput, &angleTarget, 0.0, 0.0, 0.0, DIRECT); // 

int motorSpeed=0;

void setupMPU6050();
void setupPid();
void initKalmanFilter();
void getCurrentAngle();
void balancing();
void driveMotorsBalancing();
void printData();

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  setupMPU6050();
  
  delay(100); // Wait for sensor to stabilize
  initKalmanFilter();
  setupPid();

  Serial.begin(115200);
  printTitle();
  printPID();
  Serial.println();  
}

void loop() {
  checkSerial();
  getCurrentAngle();
  angleMeasured = kalAngleX;
  if (angleMeasured > 45. && angleMeasured < 135.0) { // Only drive motors, if robot stands upright
    balancing();
    driveMotorsBalancing();
  }else{
    forward(motor1, motor2, 0);
  }
  printData();
  Serial.println();  
  delay(2);
}
void setupMPU6050(){
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz

  i2cData[0] = 7; // Set the sample rate to 1000Hz - 8kHz/(7+1) = 1000Hz
  i2cData[1] = 0x00; // Disable FSYNC and set 260 Hz Acc filtering, 256 Hz Gyro filtering, 8 KHz sampling
  i2cData[2] = 0x00; // Set Gyro Full Scale Range to ±250deg/s
  i2cData[3] = 0x00; // Set Accelerometer Full Scale Range to ±2g
  while (i2cWrite(0x19, i2cData, 4, false)); // Write to all four registers at once
  while (i2cWrite(0x6B, 0x01, true)); // PLL with X axis gyroscope reference and disable sleep mode

  while (i2cRead(0x75, i2cData, 1));
  if (i2cData[0] != 0x68) { // Read "WHO_AM_I" register
    Serial.print(F("Error reading sensor"));
    while (1);
  }
}
void setupPid() {
  // Angle control loop
  anglePid.SetSampleTime(4); // calcualte every 4ms = 250Hz
  anglePid.SetOutputLimits(-maxPower, maxPower); // output range from -43 to 43 for motor
  anglePid.SetMode(1);
}
void initKalmanFilter(){
  /* Set kalman and gyro starting angle */
  while (i2cRead(0x3B, i2cData, 6));
  accX = (i2cData[0] << 8) | i2cData[1];
  accY = (i2cData[2] << 8) | i2cData[3];
  accZ = (i2cData[4] << 8) | i2cData[5];

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees
  double roll  = atan2(accY, accZ) * RAD_TO_DEG;


  kalmanX.setAngle(roll); // Set starting angle
  gyroXangle = roll;
  compAngleX = roll;

  timer = micros();  
}
void getCurrentAngle(){
  /* Update all the values */
  while (i2cRead(0x3B, i2cData, 14));
  accX    = (double)((i2cData[0] << 8) | i2cData[1])  +OffAccX;
  accY    = (double)((i2cData[2] << 8) | i2cData[3])  +OffAccY;
  accZ    = (double)((i2cData[4] << 8) | i2cData[5])  +OffAccZ;
  tempRaw = (double)((i2cData[6] << 8) | i2cData[7]);
  gyroX   = (double)((i2cData[8] << 8) | i2cData[9])  +OffgyroX;
  gyroY   = (double)((i2cData[10] << 8) | i2cData[11])+OffgyroY;
  gyroZ   = (double)((i2cData[12] << 8) | i2cData[13])+OffgyroZ;

  double dt = (double)(micros() - timer) / 1000000; // Calculate delta time
  timer = micros();

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees
  double roll  = atan2(accY, accZ) * RAD_TO_DEG;
  double gyroXrate = gyroX / 131.0; // Convert to deg/s

  // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
  if ((roll < -90 && kalAngleX > 90) || (roll > 90 && kalAngleX < -90)) {
    kalmanX.setAngle(roll);
    compAngleX = roll;
    kalAngleX = roll;
    gyroXangle = roll;
  } else
    kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter



  gyroXangle += gyroXrate * dt; // Calculate gyro angle without any filter

  compAngleX = 0.93 * (compAngleX + gyroXrate * dt) + 0.07 * roll; // Calculate the angle using a Complimentary filter

  // Reset the gyro angle when it has drifted too much
  if (gyroXangle < -180 || gyroXangle > 180)
    gyroXangle = kalAngleX;

}

void balancing() {

  // PID Parameters (Test)
  
  anglePid.SetTunings(Kp, Ki, Kd);
  anglePid.Compute();

  // Send the calculated values to the motors
  
}

void driveMotorsBalancing() {

  int steering = 0;
  steering = constrain(steering, -7, 7);
  motorSpeed = angleOutput;

//  int leftPower = (-motorSpeed+steering*3);//*balancingFactor;
//  int rightPower = (-motorSpeed-steering*3);///balancingFactor;
//  leftPower = constrain(leftPower, -255, 255); // same range as in setupPID() + 50 offset from above!
//  rightPower = constrain(rightPower, -255, 255); // same range as in setupPID() + 50 offset from above!
  motorSpeed = constrain(-motorSpeed, -255, 255);
  motor1.drive(motorSpeed);
  motor2.drive(motorSpeed);

}
void printData(){
  Serial.print(-motorSpeed); Serial.print("\t");
  Serial.print((angleMeasured-angleTarget)*Kp);Serial.print("\t");
  Serial.print(anglePid.GetIterm()); Serial.print("\t");
  Serial.print(anglePid.GetErrorDiff());Serial.print("\t");    
}

void checkSerial()
{
  if (Serial.available())
  {
    String inString = "";
    char command = 0, chr;
    
    while(chr = Serial.read()){
      if(command>0&&(isDigit(chr)||chr=='.')){
        inString += (char)chr;  
      }else if(command==0){
        command = chr;  
      }else if(command>0&&(chr=='\n'||chr=='\r')){
        break;
      }else{
        return;
      }
    }
    switch(command){
      case 's':
          angleTarget = inString.toFloat();
          break;
      case 'p':
          Kp = inString.toFloat();
          break;
      case 'i':
          Ki = inString.toFloat();
          break;
      case 'd':
          Kd = inString.toFloat();
          break;
      case 'r':
          Kp = defaultKp;
          Ki = defaultKi;
          Kd = defaultKd;
          angleTarget=defaultSetPoint;
          
          break;
      case 'q':
//          streaming = !streaming;
          break;
    }
    printTitle();
    printPID();
    Serial.println();  
  }
  
}
void printPID(){
  char buf[40], outA[8], outP[8], outI[8], outD[8];
  Serial.print("S_");  
  printValue(angleTarget, 3, 4);
  Serial.print("P_");  
  printValue(Kp, 3, 4);
  Serial.print("I_");  
  printValue(Ki, 3, 4);
  Serial.print("D_");  
  printValue(Kd, 3, 4);
}
void printValue(double value, int d, int f){
  char out[10];
  dtostrf(value, d, f, out);
  Serial.print(out);
  Serial.print("/__");  
}
void printTitle(){
  Serial.print("motorOutput"); Serial.print("\t");
    Serial.print("ErrorTerm");Serial.print("\t");
    Serial.print("ErrIntTerm"); Serial.print("\t");
    Serial.print("ErrDifTerm");Serial.print("\t");
  
}
