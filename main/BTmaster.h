#ifndef _BTmaster_h
#define _BTmaster_h

class BTmaster{
    public:
            
        void BTsetup(){
        pinMode(LED_BUILTIN, OUTPUT);
        Serial.println("BT is ready!");
        // HC-05默認，38400
        BT.begin(38400);
        }

        void BTsendMsg(){
        
        char   str[]="";  
        dtostrf(rss,2,8,str);
        BT.write(str);
        BT.write(44);
        //dtostrf(staticOffset,2,8,str);
        BT.write(str);
        //BT.write(44);
        //dtostrf(fLastPitch,2,8,str);
        //BT.write(str);
        BT.write(10);
        
        }  

        void BTgetCommand(){
        if (Serial.available()) {
            char val = Serial.read();
            BT.print(val);
        }
        if (BT.available()) {
            char val = BT.read();
            Serial.print(val);
        }
        }  

    private:

}

#endif