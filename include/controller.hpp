#ifndef controller_hpp
#define controller_hpp

#include<Arduino.h>
#include<PSX.h>
#include<motor.hpp>
#include"Emm42_V5.0_Driver.hpp"

//ps2接收器引脚
#define dataPin   10  // Brown wire
#define cmdPin    11  // Orange wire
#define attPin    12  // Yellow wire
#define clockPin  13  // Blue wire

PSX ps2;
PSX::PSXDATA PS2data;
int PSXerror;


//坐标x,y坐标结构体
struct Coordinate
{
  int x;
  int y;
  bool is_L=false;   //是否为下层的三个箱子
  bool is_need_rotation=false;  //是否需要旋转
  
};

Coordinate box_pick_coordinate[6];  //6个箱子的xy坐标
Coordinate paper_place_coordinate[6];  //6个纸垛的xy坐标


bool isDebugging=true;    //赛前调试状态
bool isCompetition=false; //比赛状态


// 定义死区值
const int deadZone = 10;

void Debugging() {
    // 需要获取的坐标和其标志位
    int box_id = 0;
    int paper_id = 0;
    bool pick_z_H_saved = false;
    bool enter_z_L_saved = false;
    bool release_z_saved = false;
    bool pick_z_L_saved = false;

    while (isDebugging) {
        PSXerror = ps2.read(PS2data);
        if (PSXerror == PSXERROR_SUCCESS) {
            // 读取左摇杆和右摇杆的模拟值
            int leftStickX = PS2data.JoyLeftX - 128;  // 摇杆中心值为 128
            int leftStickY = PS2data.JoyLeftY - 128;
            int rightStickY = PS2data.JoyRightY - 128;

            // 控制 x 轴运动
            if (abs(leftStickX) > deadZone) {
                if (leftStickX < 0) {
                    // 左摇杆向左，x 轴负方向运动
                    X_Motor.Emm_V5_Vel_Control(0, map(abs(leftStickX), deadZone, 128, 10, 255), false, 0);
                } else {
                    // 左摇杆向右，x 轴正方向运动
                    X_Motor.Emm_V5_Vel_Control(1, map(abs(leftStickX), deadZone, 128, 10, 255), false, 0);
                }
            } else {
                // 摇杆在死区内，停止 x 轴运动
                X_Motor.Emm_V5_Vel_Control(0, 0, false, 0);
            }

            // 控制 y 轴运动
            if (abs(leftStickY) > deadZone) {
                if (leftStickY < 0) {
                    // 左摇杆向上，y 轴正方向运动
                    Y_leftMotor.Emm_V5_Vel_Control(0, map(abs(leftStickY), deadZone, 128, 10, 255), true, 0);
                    Y_rightMotor.Emm_V5_Vel_Control(1, map(abs(leftStickY), deadZone, 128, 10, 255), true, 0);
                    Y_leftMotor.sync_Motor_enable(); // 同步
                } else {
                    // 左摇杆向下，y 轴负方向运动
                    Y_leftMotor.Emm_V5_Vel_Control(1, map(abs(leftStickY), deadZone, 128, 10, 255), true, 0);
                    Y_rightMotor.Emm_V5_Vel_Control(0, map(abs(leftStickY), deadZone, 128, 10, 255), true, 0);
                    Y_leftMotor.sync_Motor_enable(); // 同步
                }
            } else {
                // 摇杆在死区内，停止 y 轴运动
                Y_leftMotor.Emm_V5_Vel_Control(0, 0, true, 0);
                Y_rightMotor.Emm_V5_Vel_Control(0, 0, true, 0);
                Y_leftMotor.sync_Motor_enable();
            }

            // 控制 z 轴运动
            if (abs(rightStickY) > deadZone) {
                if (rightStickY < 0) {
                    // 右摇杆向上，z 轴正方向运动
                    Z_Motor.Emm_V5_Vel_Control(0, map(abs(rightStickY), deadZone, 128, 10, 255), false, 0);
                } else {
                    // 右摇杆向下，z 轴负方向运动
                    Z_Motor.Emm_V5_Vel_Control(1, map(abs(rightStickY), deadZone, 128, 10, 255), false, 0);
                }
            } else {
                // 摇杆在死区内，停止 z 轴运动
                Z_Motor.Emm_V5_Vel_Control(0, 0, false, 0);
            }

            // 处理其他按键事件
            switch (PS2data.buttons) {
                case PSXBTN_TRIANGLE:    //保存上面3个箱子的吸取高度
                    if (!pick_z_H_saved) {
                        pick_z_H = Z_Motor.Emm_V5_ReadCurPos();
                        pick_z_H_saved = true;
                    }
                    break;
                case PSXBTN_CIRCLE:     //保存下面3个箱子的吸取高度
                    if (!pick_z_L_saved) {  
                        pick_z_L = Z_Motor.Emm_V5_ReadCurPos();
                        pick_z_L_saved = true;
                    }
                    break;
                case PSXBTN_SQUARE:     //保存进入铁架台下层的高度
                    if (!enter_z_L_saved) {
                        enter_z_L = Z_Motor.Emm_V5_ReadCurPos();
                        enter_z_L_saved = true;
                    }
                    break;
                case PSXBTN_CROSS:      //保存释放箱子的高度
                    if (!release_z_saved) {
                        release_z = Z_Motor.Emm_V5_ReadCurPos();
                        release_z_saved = true;
                    }
                    break;
                case PSXBTN_SELECT:     //保存:是下层箱子
                    if (box_id < 6) {
                        box_pick_coordinate[box_id].is_L = true;
                    }
                    break;
                case PSXBTN_L2:        //依次保存6个箱子的xy坐标
                    if (box_id < 6) {
                        box_pick_coordinate[box_id].x = X_Motor.Emm_V5_ReadCurPos();
                        box_pick_coordinate[box_id].y = Y_rightMotor.Emm_V5_ReadCurPos();
                        box_id++;
                    }
                    break;
                case PSXBTN_R2:         //依次保存6个纸垛的xy坐标
                    if (paper_id < 6) {
                        paper_place_coordinate[paper_id].x = X_Motor.Emm_V5_ReadCurPos();
                        paper_place_coordinate[paper_id].y = Y_rightMotor.Emm_V5_ReadCurPos();
                        paper_id++;
                    }
                    break;
                case PSXBTN_DOWN:       //是否需要旋转
                    if (box_id < 6){
                        box_pick_coordinate[box_id].is_need_rotation=true;
                    }
                    break;
                case PSXBTN_START:      //退出调试模式，开始比赛模式
                    if (box_id == 6 && paper_id == 6 && pick_z_H_saved && pick_z_L_saved && enter_z_L_saved && release_z_saved) {
                        isDebugging = false;
                        isCompetition = true;
                        Serial.println("Debugging finished");
                        competition();
                    } else {
                        Serial.println("Please complete all the parameters");
                    }
                    break;
                default:
                    break;
            }
        }
        delay(5);
    }
}

    void competition(){
        main_task();
    }



















#endif