/*#include <reg51.h>
#include <intrins.h>*/
#include "REG51.H"
#include "INTRINS.H"

#define MOTOR P1
/*��ʱ��0 2ms*/
#define TIMER0_TH 0xD8
#define TIMER0_TL 0xF0
#define TIME_FREQ 500	/*ʱ�Ӽ�ʱ����*/
#define NORMAL_SPEED 1000
#define FAST_SPEED 800
#define SLOW_SPEED 2000


typedef unsigned char uchar;
typedef unsigned int uint;
/*����˫����*/
uchar code FFW[8]={0xf3,0xf9,0xfc,0xf6};
uchar code REV[8]={0xf3,0xf6,0xfc,0xf9};

/*���൥˫����*/
uchar code FFW1[8]={0xf1,0xf3,0xf2,0xf6,0xf4,0xfc,0xf8,0xf9};
uchar code REV1[8]={0xf9,0xf8,0xfc,0xf4,0xf6,0xf2,0xf3,0xf1};

void interrupt0();
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

    }

    return 0;
}

void interrupt0() {
    startMotor();
}

/* ���ݵ�ǰ״̬ȫ�ֱ���������ṩ�������� */
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