/*
 * @Author: luomiao  1367928311@qq.com
 * @Date: 2025-03-29 16:26:12
 * @LastEditTime: 2025-04-29 20:35:10
 * @Description: 
 * 
 */
#include<Arduino.h>

//#include"lm_emmc42.hpp"
#include"Emm42_V5.0_Driver.hpp"
#include"motor.hpp"
#include"rotation.hpp"
#include"sucker.hpp"


//引脚
#define buzz 11


//定义视觉串口
HardwareSerial vision_serial(2);
uint8_t vision_data[sizeof(BOX)];//接收视觉发过来的数据
int has_received_data_num=0;



//从视觉传来的数据存放结构体格式 BOX
typedef struct BOX{
    uint8_t box_1[5];
    uint8_t box_2[5];
    uint8_t box_3[5];  // 16进制，一个箱子5位：   0xEE 箱体号码位（1-6） 箱体所在铁架位（1-6） 箱体对应纸垛位（1-6） 0xFF
    uint8_t box_4[5];  //6个箱子  30位的一个数组
    uint8_t box_5[5];
    uint8_t box_6[5];
}BOX;

BOX Box;


    //读取视觉数据
    void get_vision_data(){
        Serial.println("Start getting vision data...");
        while(vision_serial.available()>0 && has_received_data_num<sizeof(BOX)){
            vision_data[has_received_data_num]=vision_serial.read();
            has_received_data_num++;  
        }
        Serial.println("Get enough vision data.");
        //将收到的数据转换为结构体的形式
        if(has_received_data_num==sizeof(BOX)){
            
            memcpy(&Box,vision_data,sizeof(BOX));
        }
        has_received_data_num=0;    //重置为0，以便下次传输
    }



    void Buzz(){
        digitalWrite(buzz,HIGH);
        delay(3000);
        digitalWrite(buzz,LOW);

    }


    //与视觉握手，请求数据
    void handshake_with_vision(){
        //视觉需要等待0xff的信号后才能开始传输数据。
        vision_serial.write(0xFF);
        Serial.println("Handshake with vision...");
    }

    

    //！！！！！！！！！！！！！！！！！！！主任务
    void main_task(BOX Box){

    //使用上电自动回零 上位机调参
    delay(5000);//等待回零完成
    
    //旋转等待视觉读取数据
    rotate_to_angle_1();
    delay(5000);
    rotate_to_start();
    delay(5000);
    
    //请求视觉数据
    handshake_with_vision();
    //获取视觉数据并添加到Box的结构体
    get_vision_data();
    
    //按照箱子的实际序号
    //开抓     
    complete_one_box(Box.box_1);
    complete_one_box(Box.box_2);
    complete_one_box(Box.box_3);
    complete_one_box(Box.box_4);
    complete_one_box(Box.box_5);
    complete_one_box(Box.box_6);

    //抓取任务完成需要回到铁架台一侧
    move_to_y(1000,0x0300);

    //任务完成 蜂鸣器响
    Buzz();



    }  
    




    


    





void setup(){
    //串口初始化
    Serial.begin(115200);
    Serial.println("Setup started");

    //电机串口初始化
    Serial.println("Initializing motor serial...");
    motor_serial.begin(115200, SERIAL_8N1, 38, 39);
    Serial.println("Motor serial initialized.");

    //视觉串口初始化
    Serial.println("Initializing vision serial...");
    vision_serial.begin(115200, SERIAL_8N1, 16, 17);
    Serial.println("Vision serial initialized.");
    
    
    //舵机初始化
    ledcSetup(channel,freq,resolution);
    ledcAttachPin(myservo,channel);

    //电机使能
    all_motor_enable();
    Serial.println("Setup completed");

    //主任务
    main_task(Box);
    


    
}

void loop(){
    
    delay(3000);  //延时
}