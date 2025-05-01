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
#define release_z 600    //Z轴释放位置，未确定，以实际为准
#define pick_z_H 3000    //铁架台上层可直接吸取高度
#define enter_z  1000    //吸盘能进入铁架台下层的高度
#define pick_z_L 800    //铁架台下层可直接吸取高度

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
void move_to_Box(int iron_Id){
    float x,y,z;
    switch(iron_Id){   //此数据不真实待完善
    case 1://a
        x=1895;
        y=3250;
        z=pick_z_H;
        break;

    case 2://b
        x=1690;
        y=3895;
        z=pick_z_H;
        break;

    case 3://c
        x=1230;
        y=3895;
        z=pick_z_H;
        break;
    case 4://d
        x=770;
        y=3895;
        z=enter_z;
        break;
    
    case 5://e
        x=310;
        y=3895;
        z=enter_z;
        break;
    
    case 6://f
        x=105;
        y=3250;
        z=enter_z;
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

    //z轴需要再往下一步
    move_to_z(float(pick_z_L),0x0300);  //此时4个吸盘已经贴紧箱体，以求4个吸盘都能吸上 


}

//针对一个箱子的整个抓取过程
void complete_one_box(uint8_t* box){
    bool has_ratation=false;    //是否旋转过
    move_to_Box(int(box[2]));
    pick_up();
    move_to_paper(int(box[3]));
    
    if(box[3]==0x01 || box[3]==0x06){
        has_ratation=true;
        rotate_to_start();
    }
    release();
    if(has_ratation==true){//如果旋转过一次，则转回去
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



#endif