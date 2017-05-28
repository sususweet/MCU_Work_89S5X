/*#include <reg51.h>
#include <intrins.h>*/
#include "REG51.H"
#include "INTRINS.H"

#define MOTOR P1
/*定时器0 2ms*/
#define TIMER0_TH 0xD8
#define TIMER0_TL 0xF0
#define TIME_FREQ 500	/*时钟计时周期*/
#define NORMAL_SPEED 1000
#define FAST_SPEED 800
#define SLOW_SPEED 2000


typedef unsigned char uchar;
typedef unsigned int uint;
/*四相双四拍*/
uchar code FFW[8]={0xf3,0xf9,0xfc,0xf6};
uchar code REV[8]={0xf3,0xf6,0xfc,0xf9};

/*四相单双八拍*/
uchar code FFW1[8]={0xf1,0xf3,0xf2,0xf6,0xf4,0xfc,0xf8,0xf9};
uchar code REV1[8]={0xf9,0xf8,0xfc,0xf4,0xf6,0xf2,0xf3,0xf1};

void interrupt0();
void startMotor();

/*定时器0初始化*/
void init_timer0() {
    ET0 = 1;
    TR0 = 1;
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
}

int main(void) {
    TMOD = 0x01;
    EA = 1;
    init_timer0();

    while(1){

    }

    return 0;
}

void interrupt0() {
    startMotor();
}

/* 根据当前状态全局变量给电机提供驱动脉冲 */
void startMotor(){
    static unsigned char motor_index = 0, motor_count = 0;
    MOTOR = FFW[motor_index];

    motor_index++;
    if (motor_index >= 4) {
        motor_index = 0;
        motor_count++;
        /*if(motor_count>=5) {
            motor_stage = motor_cache;
        }*/
    }
}


void int0() interrupt 1{
    static unsigned int time_num = 0;
    time_num++;
    if (time_num >= TIME_FREQ) {
        time_num = 0;
    }
    interrupt0();
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
}
