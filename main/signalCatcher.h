#ifndef _signalCatcher_h
#define _signalCatcher_h

#include <QueueArray.h>
#include <Math.h>
#include "PvAlign.h"
#include "ProtoData.h"
#include <avr/pgmspace.h>

class signalCatcher{
    public:
        //three hyperparameters
        static const int RECORD_PERIOD = 5;   
        static const int sampleLength = 10;
        static const int sampleThreshold = 10;

        QueueArray <float> sampleSpace;
        float rssHistory[RECORD_PERIOD];
        bool needSample = false;
        bool alarm = false;
        float AverageRss = 0; 
        float rss = 0;
        int HisCur = 0;
        float last = 0;
        int sampleCur = 0;
        int test = 0;
        bool LockState = false;

        signalCatcher(){
            //simply fill the sample space
            sampleSpace.setPrinter(Serial);  //<-not sure if neccesarry
            for (int j = 0 ;j <sampleLength;j++){
                sampleSpace.enqueue(0);
            }
        }
        //calculate average rss
        void evalAverageRss(float *realVals){
            rss = 100*sqrt(realVals[0]*realVals[0]+realVals[1]*realVals[1]+realVals[2]*realVals[2]);
            if(recordRss()){  //Check if the recording of one period is done
                float sum = 0;
                for (int i = 0; i<RECORD_PERIOD;i++){
                    sum += rssHistory[i];
                    }
                AverageRss = sum/RECORD_PERIOD ;  //update the AverageRss
            }  
        }

        float getAverageRss(){
            return this->AverageRss;
        }       
        
        int getTest(){
            return this->test;
        }     
        //deal with the motion signal sampling 
        void sample(bool msg){
            sampleSpace.enqueue(AverageRss);

            if(needSample){
                
                //this "if" means that the peak triggered the sampling are put in the middle
                if(sampleCur++ == sampleLength/2) {    
                    float vector[sampleLength];
                    int i = 0;
                    while (!sampleSpace.isEmpty()){
                        vector[i++]=sampleSpace.dequeue();
                    }
                    for (int j = 0 ;j <sampleLength ;j++){
                        sampleSpace.enqueue(vector[j]);
                    }

                    if(msg){
                        Serial.println(" ");
                        Serial.println(" Sampled! the vector is : ");
                        Serial.print("{");
                        for (int j = 0 ;j <sampleLength ;j++){
                            Serial.print(vector[j]);
                            Serial.print(",");
                        }
                        Serial.print("}");

                        evalScore(vector);
                    }

                    sampleCur = 0;
                    needSample=false;
                    test = 0;
                }
            }

            else if(LockState){
                if(AverageRss - last > sampleThreshold and AverageRss > last) {
                    if(msg)
                        Serial.println("Start sample! ");
                    needSample = true;
                    alarm = true;
                    test = 100;
                }
            }

            sampleSpace.dequeue();

            last = AverageRss;
        }

    private:
        PvAlign pva;

        //record rss history to determine the average rss in a period
        bool recordRss(){ 
            if (HisCur==RECORD_PERIOD){  //one period of record is done
                HisCur = 0;
                return 1;
            }
            rssHistory[HisCur++] = rss;
            return 0;
        }
        
        void evalScore(float *vec){
            float score = 0;
            

            for(int i=0;i<20;i++){
                float* datavec = read_from_pgm(i);

                for(int j=0;j<20;j++){
                    Serial.print(datavec[j]);
                    Serial.print(" ");
                }

                float cur = pva.evaluate(vec,datavec,10,10);
                Serial.print("score : ");
                Serial.println(cur);
                score += cur;
            }

            Serial.print("average score : ");
            Serial.println(score/4);
            return 0;
        }

        float* read_from_pgm(int index){
            float buffer[10];
            for (int i = 0 ;i< 10;i++)
               buffer[i] = pgm_read_float(&liftPencilBox_0[index][i]);
            return buffer;
        }
      
};

#endif