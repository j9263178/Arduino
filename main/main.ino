#include <Math.h>
#include "signalCatcher.h"
#include "Player.h"
#include "MPU6050.h"
#include "enduser_setup.c"
#include <FirebaseArduino.h> 
#include <ESP8266WiFi.h>             //wifi library
#define WIFI_SSID "SSID"             //replace SSID with your wifi username
#define WIFI_PASSWORD "PWD"          //replace PWD with your wifi password
#define WIFI_LED D5                  //connect a led to any of the gpio pins of the board and replace pin_number with it eg. D4        

#define FIREBASE_HOST "xxxxxxxxxxx.firebaseio.com"                         //link of api
#define FIREBASE_AUTH "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"           //database secret


//critical for determining the size of sample space
static const int DELAYTIME = 2; 
int delaytime = 0;
bool isLocked = false;

signalCatcher sc;
Player py;
MPU6050 mpu;

void setup() {
  Serial.begin(9600);
  pinMode(WIFI_LED,OUTPUT);                         //define pinmodes
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  enduser_setup.start() 
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);     //connect to Database
  delay(100);  
  py.setup();
  mpu.setupMPU();
  delay(100); // Wait for sensor to stabilize
}

void loop() {
  check();

  if (isLocked && sc.alarm){ 
    Firebase.setInt("test1/stolen", 1);
    sc.alarm = false;
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

void connect2wifi(){
  while (WiFi.status() != WL_CONNECTED) {           //wait till connected to WiFi
    delay(100);  
    digitalWrite(WIFI_LED,LOW);                     //Blink the light till connected to WiFi
    delay(100);
    digitalWrite(WIFI_LED,HIGH);
    Serial.print("."); }

  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(WIFI_LED,HIGH);  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void check(){
  isLocked = Firebase.getBool("test1/isLocked");
  if (Firebase.failed()) {
      Serial.print("pushing /MyTest failed:");
      Serial.println(Firebase.error());
      return;
  }

}