#ifndef motor_hpp
#define motor_hpp
#include<HardwareSerial.h>
#include "Emm42_V5.0_Driver.hpp"
#include"sucker.hpp"
#include"rotation.hpp"

HardwareSerial motor_serial(1);

//运动齿轮的半径
#define Y_GEARTEETH_R 20  //驱动3个电机运动的齿轮半径，值不是准确的，以实际为准
#define Z_GEARTEETH_R 20
#define X_GEARTEETH_R 20

#define PI  3.1415926535897932384626433832795

//释放箱体高度
float release_z;     //Z轴释放位置，未确定，以实际为准
float pick_z_H;     //铁架台上层可直接吸取高度
float enter_z_L;    //吸盘能进入铁架台下层的高度
float pick_z_L;  //铁架台下层可直接吸取高度

//一些参数
#define CW 0x00
#define CCW 0x01


//定义4个电机
Emm42_V5_0_Driver Y_leftMotor(&motor_serial,1,0x6B,true);
Emm42_V5_0_Driver Y_rightMotor(&motor_serial,2,0x6B,true);
Emm42_V5_0_Driver X_Motor(&motor_serial,3,0x6B,true);
Emm42_V5_0_Driver Z_Motor(&motor_serial,4,0x6B,true);



//电机使能
void all_motor_enable(){
    Serial.println("Enabling all motors...");
    Y_leftMotor.Emm_V5_Motor_enable();
    Y_rightMotor.Emm_V5_Motor_enable();
    X_Motor.Emm_V5_Motor_enable();
    Z_Motor.Emm_V5_Motor_enable();
    Serial.println("All motors enabled.");
}






//y轴运动
void move_to_y(float y,uint16_t speed){//采用绝对运动，y是关于全局零点的位置
    Serial.println("Moving to Y position...");
    float pulse=(y/(2*PI*Y_GEARTEETH_R))*3200;
    uint32_t pulse_num=(uint32_t)(pulse);
    Y_leftMotor.Emm_V5_Pos_Control(CW,speed,pulse_num,true,false,0x00);
    Serial.println("Y_leftMotor sent success");
    Y_rightMotor.Emm_V5_Pos_Control(CCW,speed,pulse_num,true,false,0x00);
    Serial.println("Y_rightMotor sent success");
    Y_leftMotor.sync_Motor_enable();//开启同步运动   只要调用一次即可
   // Y_rightMotor.sync_Motor_enable();   
    Serial.println("Y axis movement completed.");
}


//x轴运动
void move_to_x(float x,uint16_t speed){
    Serial.println("Moving to X position...");
    float pulse=(x/(2*PI*X_GEARTEETH_R))*3200;
    uint32_t pulse_num=(uint32_t)(pulse);
    X_Motor.Emm_V5_Pos_Control(CW,speed,pulse_num,false,false,0x00);  //计划X和Z同步运动，提高效率，所以此处先开启同步
    Serial.println("X axis movement completed.");
}


//z轴运动
void move_to_z(float z,uint16_t speed){
    Serial.println("Moving to Z position...");
    float pulse=(z/(2*PI*Z_GEARTEETH_R))*3200;
    uint32_t pulse_num=(uint32_t)(pulse);
    Z_Motor.Emm_V5_Pos_Control(CW,speed,pulse_num,true,false,0x00);
    Serial.println("Z axis movement complet   ed.");
}


//运动到对应纸垛位置
void move_to_paper(int box_Id){
    float x,y,z;
    switch (box_Id)
    {
    case 1://a
        x=1895;
        y=3250;
        z=release_z;
        break;

    case 2://b
        x=1690;
        y=3895;
        z=release_z;
        break;

    case 3://c
        x=1230;
        y=3895;
        z=release_z;
        break;

    case 4://d
        x=770;
        y=3895;
        break;

    case 5://e
        x=310;
        y=3895;
        z=release_z;
        break;
    
    case 6://f
        x=105;
        y=3250;
        z=release_z;
        break;
    
    default:
        break;
    }
    
    //运动到对应的坐标
    //先x，z
    move_to_x(x,0x0300);//x
    move_to_z(z,0x0300);//z
    
    
    //开启x，z同步
    X_Motor.sync_Motor_enable();  //x，z两电机一起 开始运动
    delay(10);  //防止刚开始还没来得及运动就进入死循环  
    while(X_Motor.EmmV5_ReadCurRPM()!=0 && Z_Motor.EmmV5_ReadCurRPM()!=0){
        delay(100);  //等待x,z电机到位
    }
    
    //再y
    move_to_y(y,0x0300); //y轴运动内置同步
    delay(10);
    while(Y_leftMotor.EmmV5_ReadCurRPM()!=0 && Y_rightMotor.EmmV5_ReadCurRPM()!=0){
        delay(100);  //等待Y轴电机到位
    }


}



//运动到对应箱子的可夹取坐标
void move_to_Box(int box_id){
    int index=box_id-1;
    float x=box_pick_coordinate[index].x;
    float y=box_pick_coordinate[index].y;
    float z=0;
    if(box_pick_coordinate[index].is_L==true){        //是下层的三个箱子之一
        move_to_z(enter_z_L,0x0300);
        while(Z_Motor.EmmV5_ReadCurRPM()!=0){
            delay(10);
        }
        move_to_x(x,0x0300);
        move_to_y(y,0x0300);
        while(X_Motor.EmmV5_ReadCurRPM()!=0 && Y_leftMotor.EmmV5_ReadCurRPM()!=0 && Y_rightMotor.EmmV5_ReadCurRPM()!=0){
            delay(10);
        } 
        move_to_z(pick_z_L,0x0300);
        while(Z_Motor.EmmV5_ReadCurRPM()!=0){
            delay(10);
        }

    }else{                                            //是上层的三个箱子之一
        move_to_x(x,0x0300);
        move_to_y(y,0x0300);
        while(X_Motor.EmmV5_ReadCurRPM()!=0 && Y_leftMotor.EmmV5_ReadCurRPM()!=0 && Y_rightMotor.EmmV5_ReadCurRPM()!=0){
            delay(10);
        }
        move_to_z(pick_z_H,0x0300);
        while(Z_Motor.EmmV5_ReadCurRPM()!=0){
            delay(10);
        }
    }

}

//针对一个箱子的整个抓取过程
// 假设在某个头文件中已经包含了以下声明
#include "controller.hpp"
#include "motor.hpp"
#include "sucker.hpp"
#include "rotation.hpp"

// 针对一个箱子的整个抓取过程
void complete_one_box(int box_id, int paper_id) {
    bool has_ratation = false;    // 是否旋转过

    // 运动到对应箱子的可夹取坐标
    move_to_Box(box_id);
    pick_up();

    // 运动到对应纸垛位置
    move_to_paper(paper_id);

    // 判断是否需要旋转
    if (box_pick_coordinate->is_need_rotation==true) {
        rotate_to_start();
        has_ratation = true;
    }

    // 释放箱子
    release();
    

    // 如果旋转过一次，则转回去
    if (has_ratation == true) {
        rotate_to_angle_1();
    }
}


//回到铁架台一侧
void back_to_A_side(){
    move_to_y(1000,0x0300);
}


//y轴回零
void Y_rezero(uint16_t speed){
    //修改回零参数
    Y_leftMotor.Emm_V5_Modify_ZeroParams(0x02,CW,speed,uint32_t(10000),uint16_t(300),uint16_t(400),uint16_t(20),false,true);
    Y_rightMotor.Emm_V5_Modify_ZeroParams(0x02,CCW,speed,uint32_t(10000),uint16_t(300),uint16_t(400),uint16_t(20),false,true);


    //触发回零
    Emm42_CMDstatus Y_leftMotor_status=Y_leftMotor.Emm_V5_TriggerToZero(true,2);  
    Emm42_CMDstatus Y_rightMotor_status=Y_rightMotor.Emm_V5_TriggerToZero(true,2);
    Y_leftMotor.sync_Motor_enable();   //两电机同步开始回零
    if(Y_leftMotor_status==Emm42_OK && Y_rightMotor_status==Emm42_OK){  //两个电机都成功开始回零
        while((Y_leftMotor.is_rezero_finish()==false) && (Y_rightMotor.is_rezero_finish()==false)){   //当两个电机仍在回零过程中时
            delay(100);
        }
        Serial.println("Y_rezero complete.");  //回零完成
        Y_leftMotor.Emm_V5_Reset_CurPos_To_Zero();
        Y_rightMotor.Emm_V5_Reset_CurPos_To_Zero();   //清除位置 设定为零点 至此Y轴电机的位置为零 回零完成
}else{
    Serial.println("Y_rezero trigger failed.");
}




}


//！！！！！！！！！！！！！！！！！！！主任务
void main_task(){

    //使用上电自动回零 上位机调参
    delay(5000);//等待回零完成
    
    //旋转等待视觉读取数据
    //rotate_to_angle_1();
    ////delay(5000);
    //rotate_to_start();
   // delay(5000);
    
    //请求视觉数据
   // handshake_with_vision();
    //获取视觉数据并添加到Box的结构体
   // get_vision_data();
    
    //按照箱子的实际序号
    //开抓     
    for(int i=0;i<6;i++){
        complete_one_box(i+1,i+1);
    }

    //抓取任务完成需要回到铁架台一侧
    move_to_y(1000,0x0300);

    //任务完成 蜂鸣器响
    Buzz();



    }  



#endif