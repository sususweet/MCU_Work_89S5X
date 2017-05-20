#include <reg51.h>
#include <intrins.h>
/*#include "REG51.H"
#include "INTRINS.H"*/

#define MOTOR P1
/*��ʱ��0 2ms*/
#define TIMER0_TH 0xF8
#define TIMER0_TL 0x30
#define TIME_FREQ 500	/*ʱ�Ӽ�ʱ����*/
#define NORMAL_SPEED 100
#define FAST_SPEED 70
#define SLOW_SPEED 200

typedef unsigned char uchar;
/*����˫����*/
uchar code FFW[8]={0xf3,0xf9,0xfc,0xf6};
uchar code REV[8]={0xf3,0xf6,0xfc,0xf9};

/*���൥˫����*/
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
void delay(unsigned int xms);
void startMotor();

/*��ʱ��0��ʼ��*/
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

/* ����һ����ʱxms�������ʱ����,xms������Ҫ��ʱ�ĺ����� */
void delay(unsigned int xms) {
    unsigned int a,b,c;
    for(a=0;a<xms;a++)
        for(b=0;b<3;b++)
            for(c=0;c<220;c++);
}

void interrupt0() {

}

/* ���ݵ�ǰ״̬ȫ�ֱ���������ṩ�������� */
void startMotor(){
    unsigned int i = 0;
    unsigned int delayTime = 0;
    for (i=0;i<4;i++){
        /*�������ת���ƣ��Ӷ���ı��ж�ȡ����*/
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
        /*���ת�ٿ��ƣ������ʱʱ��Խ����ת��Խ��*/
        delay(delayTime);
    }
}

/*�ı䵱ǰ״̬ȫ�ֱ������ɶ�ʱ�����ƣ�1��ı�һ��״̬*/
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