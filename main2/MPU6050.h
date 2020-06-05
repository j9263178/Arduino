#ifndef _MPU6050_h
#define _MPU6050_h

#include "Kalman.h"
#include <Wire.h>
#include <Math.h>

class MPU6050 {
public:
        const int RECORD_PERIOD=30;
        float fRad2Deg = 57.295779513f; //将弧度转为角度的乘数
        const int MPU = 0x68; //MPU-6050的I2C地址
        static const int nValCnt = 7; //一次读取寄存器的数量
        const int nCalibTimes = 1000; //校准时读数的次数
        int calibData[nValCnt]; //校准数据
        unsigned long nLastTime = 0; //上一次读数的时间
        unsigned long nCurTime; 
        float fLastRoll = 0.0f; //上一次滤波得到的Roll角
        float fLastPitch = 0.0f; //上一次滤波得到的Pitch角
        Kalman kalmanRoll; //Roll角滤波器
        Kalman kalmanPitch; //Pitch角滤波器
        float realVals[7];
        int readouts[nValCnt];

    MPU6050(){
    };
    void setupMPU(){
        Wire.begin();
        WriteMPUReg(0x6B, 0); 
        Calibration();
    }

//向MPU6050写入一个字节的数据
//指定寄存器地址与一个字节的值
    void WriteMPUReg(int nReg, unsigned char nVal) {
        Wire.beginTransmission(MPU);
        Wire.write(nReg);
        Wire.write(nVal);
        Wire.endTransmission(true);
    };
//从MPU6050读出加速度计三个分量、温度和三个角速度计
//保存在指定的数组中
    void ReadAccGyr(int *pVals) {
        Wire.beginTransmission(MPU);
        Wire.write(0x3B);
        Wire.requestFrom(MPU, nValCnt * 2, true);
        Wire.endTransmission(true);

        for (long i = 0; i < nValCnt; ++i) {
            pVals[i] = Wire.read() << 8 | Wire.read();
        }
    };

//对大量读数进行统计，校准平均偏移量
    void Calibration(){
        float valSums[7] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0};
        //先求和
        for (int i = 0; i < nCalibTimes; ++i) {
            int mpuVals[nValCnt];
            ReadAccGyr(mpuVals);
            for (int j = 0; j < nValCnt; ++j) {
            valSums[j] += mpuVals[j];
            }
        }
        //再求平均
        for (int i = 0; i < nValCnt; ++i) {
            calibData[i] = int(valSums[i] / nCalibTimes);
        }

        calibData[2] += 16384; //设芯片Z轴竖直向下，设定静态工作点。
    };

//对读数进行纠正，消除偏移，并转换为物理量。公式见文档。
    void Rectify() {
        for (int i = 0; i < 3; ++i) {
            realVals[i] = (float)(readouts[i] - calibData[i]) / 16384.0f;
        }
        realVals[3] = readouts[3] / 340.0f + 36.53;
        for (int i = 4; i < 7; ++i) {
            realVals[i] = (float)(readouts[i] - calibData[i]) / 131.0f;
        }
    };
    
    float* getData(){
        ReadAccGyr(readouts);
        Rectify();
        return realVals;
    };

    private:
};
    

#endif
