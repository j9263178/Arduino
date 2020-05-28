#include <SoftwareSerial.h> 
#include <Vector.h>
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294 
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370 
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523


int note[]={NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4};
int sh[5];
int count=0;
const int ELEMENT_COUNT_MAX = 100;
int storage_array[ELEMENT_COUNT_MAX];
Vector<int> notein;
int errortime=0;
// Pin10為RX，接HC05的TXD
// Pin11為TX，接HC05的RXD
SoftwareSerial BT(10, 11); 

char val;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(38400); 
  Serial.println("BT is ready!");
  BT.begin(38400);
  notein.setStorage(storage_array);
}

void loop() {
  if (Serial.available()) {
    val = Serial.read();
    BT.print(val);
  }

  if (BT.available()) {
    int val = BT.read();
    if(val != 13 && val !=10){
      notein.push_back(val);
      count+=1;
    }else if(val==13){
      play();
      count=0;
      }
    //String val = BT.read();
    Serial.println(val); 
  }
}

int t=300 ;

void play() {
      for (int i=0;i<notein.size();i++){
      int a=(notein[i]-97)%7;
      tone(8, note[a], t);
      delay(t);
      }
      notein.clear();
}
