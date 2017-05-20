#include <reg51.h>
#include <intrins.h>
/*#include "REG51.H"
#include "INTRINS.H"*/

#define MOTOR P1
/*定时器0 2ms*/
#define TIMER0_TH 0xF8
#define TIMER0_TL 0x30
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

enum motor_state {
    POSITIVE,
    NEGETIVE,
    HIGH,
    LOW
};
unsigned int motor_stage = POSITIVE;
void interrupt0();
void delay_us(uint us);
void delay_ms(uint ms);
void delay_s(uint s);
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
        startMotor();
    }

    return 0;
}

void delay_us(uint us){
    do {
        _nop_();
        us--;
    } while(us);
}

/* 定义一个延时xms毫秒的延时函数,xms代表需要延时的毫秒数 */
void delay_ms(uint ms) {
    do {
        delay_us(1000);
        ms--;
    } while(ms);
}

void delay_s(uint s){
    do {
        delay_ms(1000);
        s--;
    } while(s);
}

void interrupt0() {

}

/* 根据当前状态全局变量给电机提供驱动脉冲 */
void startMotor(){
    unsigned int i = 0;
    unsigned int delayTime = 0;
    for (i=0;i<4;i++){
        /*电机正反转控制，从定义的表中读取数据*/
        if (motor_stage == NEGETIVE){
            MOTOR = REV[i];
        }else{
            MOTOR = FFW[i];
        }
        switch (motor_stage){
            case POSITIVE:{
                delayTime = NORMAL_SPEED;
                break;
            }
            case NEGETIVE:{
                delayTime = NORMAL_SPEED;
                break;
            }
            case HIGH:{
                delayTime = FAST_SPEED;
                break;
            }
            case LOW:{
                delayTime = SLOW_SPEED;
                break;
            }
            default:break;
        }
        /*电机转速控制，电机延时时间越长，转速越慢*/
        delay_us(delayTime);
    }
}

/*改变当前状态全局变量，由定时器控制，1秒改变一次状态*/
void changeState(){
    switch (motor_stage){
        case POSITIVE:{
            motor_stage = NEGETIVE;
            break;
        }
        case NEGETIVE:{
            motor_stage = HIGH;
            break;
        }
        case HIGH:{
            motor_stage = LOW;
            break;
        }
        case LOW:{
            motor_stage = POSITIVE;
            break;
        }
        default:break;
    }
}

void int0() interrupt 1{
    static unsigned int time_num = 0;
    time_num++;
    if (time_num >= TIME_FREQ) {
        time_num = 0;
        changeState();
    }
    interrupt0();
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
}
