#ifndef rotation_hpp
#define rotation_hpp
#include "Arduino.h"

#define myservo 22

//云台舵机参数
#define freq  50
#define channel  1
#define resolution 10
#define myservo    22
int angle_1=1.8/20*pow(2,resolution);    //180
int angle_2=0.5/20*pow(2,resolution);    //0

 //云台旋转  b,c,d,e纸垛放置角度
 void rotate_to_start(){
    digitalWrite(myservo,angle_2);
}


//云台旋转    a,f纸垛放置角度
void rotate_to_angle_1(){
    digitalWrite(myservo,angle_1);
}


#endif