#include <avr/pgmspace.h>

void setup(){
    const static PROGMEM float charSet[3][10] = {
      {15.73,23.94,23.94,46.76,46.76,54.39,54.39,59.01,59.01,55.91},
      {15.65,23.64,23.64,39.19,39.19,42.50,42.50,60.16,60.16,60.78},
      {5.82,12.82,12.82,39.77,39.77,38.10,38.10,21.60,21.60,30.89} 
      };

    const static PROGMEM float Set[10] = {
      15.73,23.94,23.94,46.76,46.76,54.39,54.39,59.01,59.01,55.91
      };  
     Serial.begin(9600);
     delay(1000);
    float buffer;
    for (int i = 0; i < 3; i++) {
        buffer = pgm_read_float(&charSet[i]);
      for (int j=0 ; j<10;j++){
        Serial.print(buffer + j);
      }
      Serial.println("  ");  
    }
    
}

void loop(){

}
