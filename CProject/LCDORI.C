/*#include <REG51.H>
#include <INTRINS.H>
#include <STRING.H>
#include <STDLIB.H>
#include <STDIO.H>*/

#include "REG51.H"
#include "INTRINS.H"
#include "STRING.H"
#include "STDLIB.H"
#include "STDIO.H"

#define DATA_LCD P0
sbit RS_LCD = P2^0;
sbit RW_LCD = P2^1;
sbit EN_LCD = P2^2;
sbit PSB_LCD = P2^3;
sbit RST_LCD = P2^5;

#define KEYBOARD P1
/*ע�⣬motorֻ��ʹ��P0�ĸ���λ��ͬʱ�ڲ���motorʱ�ǵù�Һ������EN*/
#define MOTOR P3
/*ע�⣬��ȡADת��������ʱ���ǵù�Һ����ʾEN*/
#define AD_OUTPUT P0

/*��ʱ��0 1ms*//*
#define TIMER0_TH 0xF8
#define TIMER0_TL 0x30*/

#define TIMER0_TH 0xFC
#define TIMER0_TL 0x18
#define TWINKLE_FREQ 400    /*��˸����*/
#define TIME_FREQ 1000    /*ʱ�Ӽ�ʱ����*/
#define DISP_FREQ 400   /*LCD��ʾˢ������*/
#define INFO_SHOW_FREQ 3000   /*LCD��Ϣ��ʾʱ��*/


#define KEY_WAIT 20    /*����ɨ���ӳ�����*/
#define MAX_SPACE 3
#define TWINKLE_ROW_NUM 4

typedef unsigned char uchar;
typedef struct {
    unsigned int startTime;
    unsigned int endTime;
    unsigned char used;
    unsigned char carID;
} ParkInfo;
enum key_states_e {
    KEY_STATE_RELEASE,
    KEY_STATE_WAITING,
    KEY_STATE_PRESSED
};
enum working_state {
    NORMAL, PARK_IN, PARK_OUT, PARK_NO_SPACE, PARK_NO_CAR
};
enum motor_state {
    MOTOR_CLOSING, MOTOR_CLOSED, MOTOR_OPENING, MOTOR_OPENED, MOTOR_BUSY
};

/*����˫����*/
uchar code FFW[8]={0xf3,0xf9,0xfc,0xf6};
uchar code REV[8]={0xf3,0xf6,0xfc,0xf9};
uchar code DateStr[] = __DATE__;
uchar code TimeStr[] = __TIME__;
uchar idata LCDTable1[16], LCDTable2[16], LCDTable3[16], LCDTable4[16];

unsigned int idata twinkle_row[TWINKLE_ROW_NUM] = {0, 0, 0, 0};
unsigned int car_id = 1;
unsigned int working_stage = NORMAL;
uchar motor_stage = MOTOR_CLOSED;
unsigned int nowTime = 0, info_num = 0;
unsigned int sec = 0, minute = 0, hour = 0;
unsigned int time_num = 0, disp_num = 0;

ParkInfo idata parkSpace[MAX_SPACE];

unsigned int is_leap_year(unsigned int year);

void time_inc();
void init_settings();
void init_timer0();
void opr_key(unsigned int key_code);
unsigned int press_key();
unsigned int read_key();


/*���������⿪ʼ*/
void clr_twinkle(){
    unsigned int i;
    for (i = 0; i < TWINKLE_ROW_NUM; i++){
        twinkle_row[i] = 0;
    }
    return;
}

void delay_us(unsigned int us) {
    do {
        _nop_();
        us--;
    } while (us);
}

/* ����һ����ʱxms�������ʱ����,xms������Ҫ��ʱ�ĺ����� */
void delay_ms(unsigned int ms) {
    do {
        delay_us(1000);
        ms--;
    } while (ms);
}

/**
 * @desc �����жϺ���
 * @param year ��Ҫ�����жϵ����
 * @return 1:������  0:��������
 * */
unsigned int is_leap_year(unsigned int year) {
    unsigned int leap_year;
    if ((year + 2000) % 400 == 0) leap_year = 1;    //  ��400����Ϊ����
    else if ((year + 2000) % 100 == 0) leap_year = 0;  // ���ܱ�400���� �ܱ�100���� ��������
    else if ((year + 2000) % 4 == 0) leap_year = 1;  // ���ܱ�400��100���� �ܱ�4���� ������
    else leap_year = 0;
    return leap_year;
}

/**
 * @desc ʱ����������
 * */
void time_inc() {
    sec++;
    /*һ���ʱ���λ�߼�*/
    if (sec >= 60) {
        minute += 1;
        sec = 0;
    }
    if (minute >= 60) {
        hour += 1;
        minute = 0;
    }
    if (hour >= 24) hour = 0;
    return;
}

unsigned int time_diff_hours(unsigned int endTime, unsigned int startTime){
    unsigned int diff_hours;
    unsigned int seconds = endTime - startTime;
    diff_hours= (uchar) (seconds / 3600);
    return diff_hours;
}

unsigned int time_diff_minutes(unsigned int endTime, unsigned int startTime){
    unsigned int diff_minutes;
    unsigned int seconds = endTime - startTime;
    seconds %= 3600 ;
    diff_minutes= (uchar) (seconds / 60);
    return diff_minutes;
}

unsigned int time_diff_seconds(unsigned int endTime, unsigned int startTime){
    unsigned int diff_seconds;
    unsigned int seconds = endTime - startTime;
    seconds %= 3600 ;
    diff_seconds= (uchar) (seconds % 60);
    return diff_seconds;
}

void scan_key() {
    static int key_state = KEY_STATE_RELEASE;   /*״̬��״̬��ʼ��������static����״̬*/
    unsigned int wait_time = KEY_WAIT;   /*����ɨ��ȴ�ʱ��*/
    static int key_code = -1;
    unsigned int pressed = press_key(); /*press_keyΪ����Ƿ��а������µĺ���*/
    static scan_time = 0;
    switch (key_state) {
        case KEY_STATE_RELEASE: {   /*��ԭʼ״̬Ϊ�ް�������RELEASE��ͬʱ�ּ�⵽�������£���״̬ת����WAITING*/
            if (pressed == 1) {
                key_state = KEY_STATE_WAITING;
            }
            break;
        }
        case KEY_STATE_WAITING: {   /*ԭʼ״̬ΪWAITING���԰������ж���ж�*/
            if (pressed) {
                scan_time++;
                if (scan_time >= wait_time) {   /*���������µ�ʱ�䳬��һ��ʱ�䣬����Ϊ�������£�������*/
                    key_state = KEY_STATE_PRESSED;
                    scan_time = 0;
                    key_code = read_key();  /*read_keyΪ�������ĺ���*/
                }
            } else {    /*�������ɿ�����ָ�����ʼ״̬*/
                scan_time = 0;
                key_state = KEY_STATE_RELEASE;
            }
            break;
        }
        case KEY_STATE_PRESSED: {   /*��������ȷ�ϰ��£���ȴ������ɿ��ٽ��в���*/
            if (pressed == 0 && key_code >= 0) {
                opr_key((unsigned int) key_code);  /*opr_keyΪ�����¼���Ӧ����*/
                key_state = KEY_STATE_RELEASE;
                key_code = -1;
            }
            break;
        }
        default:
            break;
    }
}

//�ж��Ƿ��м�����
unsigned int press_key() {
    unsigned char temp_line, temp_row;
    KEYBOARD = 0xF0;
    temp_line = KEYBOARD;
    if (temp_line == 0xF0) return 0;

    KEYBOARD = 0x0F;
    temp_row = KEYBOARD;
    if (temp_row == 0x0F) return 0;
    return 1;
}

//ɨ����̣����ؼ�ֵ
unsigned int read_key() {
    unsigned char temp_column, temp_row;
    unsigned int row, column, key_code;
    KEYBOARD = 0xF0;
    temp_column = KEYBOARD;

    KEYBOARD = 0x0F;
    temp_row = KEYBOARD;

    switch (temp_column) {
        case 0x70: {
            column = 0;
            break;
        }

        case 0xB0: {
            column = 1;
            break;
        }

        case 0xD0: {
            column = 2;
            break;
        }

        case 0xE0: {
            column = 3;
            break;
        }

        default:
            break;
    }

    switch (temp_row) {
        case 0x07: {
            row = 0;
            break;
        }
        case 0x0B: {
            row = 1;
            break;
        }
        case 0x0D: {
            row = 2;
            break;
        }
        case 0x0E: {
            row = 3;
            break;
        }
        default:
            break;
    }
    key_code = 4 * row + column;
    return key_code;
}
/*�������������*/

/* ���ݵ�ǰ״̬ȫ�ֱ���������ṩ�������� */
void startMotor(uchar type){
    static unsigned char motor_index = 0, motor_count = 0, motor_cache = MOTOR_BUSY;
    switch (type){
        case MOTOR_CLOSING:{
            motor_cache = MOTOR_CLOSED;
            motor_stage = MOTOR_BUSY;
            MOTOR = FFW[motor_index];
            break;
        }
        case MOTOR_OPENING:{
            motor_cache = MOTOR_OPENED;
            motor_stage = MOTOR_BUSY;
            MOTOR = REV[motor_index];
            break;
        }
        case MOTOR_BUSY:{
            switch (motor_cache){
                case MOTOR_CLOSED:{
                    MOTOR = FFW[motor_index];
                    break;
                }
                case MOTOR_OPENED:{
                    MOTOR = REV[motor_index];
                    break;
                }
                default:break;
            }
        }
        default:break;
    }

    motor_index++;
    if (motor_index >= 4) {
        motor_index = 0;
        motor_count++;
        if(motor_count>=5) {
            motor_stage = motor_cache;
        }
    }
}

/*��ȡʣ��ռ�*/
int getSpace() {
    unsigned int i = 0;
    for (i = 0; i < MAX_SPACE; i++) {
        if (parkSpace[i].used == 0) return i;
    }
    return -1;
}

/*��ȡʣ��ռ���Ŀ*/
unsigned int getSpaceNum() {
    unsigned int i = 0, count = 0;
    for (i = 0; i < MAX_SPACE; i++) {
        if (parkSpace[i].used == 0) count++;
    }
    return count;
}

void park_in(unsigned int id){
    //clr_twinkle();
    parkSpace[id].startTime = nowTime;
    parkSpace[id].carID = car_id;
    car_id++;
    parkSpace[id].used = 1;
    strcpy(LCDTable1,"    ��ӭ����    ");
    sprintf(LCDTable2, "���ƺţ���A%04d", parkSpace[id].carID);
    sprintf(LCDTable3, "��λ:%d  ��ʼ�Ʒ�",id + 1);
    sprintf(LCDTable4, "    %02d:%02d:%02d    ", hour, minute, sec);
    working_stage = PARK_IN;
    info_num = 0;
    return;
}

void park_out(unsigned int id){
    //clr_twinkle();
    strcpy(LCDTable1,"    һ·˳��    ");
    twinkle_row[0] = 1;
    if (parkSpace[id].used == 1){
        parkSpace[id].endTime = nowTime;
        parkSpace[id].used = 0;
        sprintf(LCDTable2, "���ƺţ���A%04d", parkSpace[id].carID);
        sprintf(LCDTable3, "  Ӧ���ѣ�%1.2f  ",(float) ((parkSpace[id].endTime - parkSpace[id].startTime) * 0.5));
        sprintf(LCDTable4, "ͣ��ʱ��%02d:%02d:%02d", time_diff_hours(parkSpace[id].endTime, parkSpace[id].startTime),time_diff_minutes(parkSpace[id].endTime, parkSpace[id].startTime), time_diff_seconds(parkSpace[id].endTime, parkSpace[id].startTime));
    }else{
        sprintf(LCDTable2, "  ��λû�г���  ", parkSpace[id].carID);
        strcpy(LCDTable3, "                ");
        sprintf(LCDTable4, "    %02d:%02d:%02d    ", hour, minute, sec);
    }
    working_stage = PARK_OUT;
    info_num = 0;
    return;
}

void opr_key(unsigned int key_code) {
    switch (key_code) {
        case 0: {
            int space_id = getSpace();
            if (space_id != -1){
                park_in((unsigned int) space_id);
            }
            break;
        }
        case 1: {
            park_out(0);
            break;
        }
        case 2: {
            park_out(1);
            break;
        }
        case 3: {
            park_out(2);
            break;
        }
        case 4: {
            switch (motor_stage){
                case MOTOR_CLOSED:{
                    motor_stage = MOTOR_OPENING;
                    break;
                }
                case MOTOR_OPENED:{
                    motor_stage = MOTOR_CLOSING;
                    break;
                }
                default:break;
            }
        }
        default: break;
    }
}


//æ��⣬��æ��ȴ�����ȴ�ʱ��Ϊ60ms
void busyCheck_12864(void) {
    unsigned int timeout = 0;
    DATA_LCD = 0xff;
    RS_LCD = 0;
    RW_LCD = 1;
    EN_LCD = 1;
    while ((DATA_LCD & 0x80) && ++timeout != 0);  //æ״̬��⣬�ȴ���ʱʱ��Ϊ60ms
    EN_LCD = 0;
}

//д�����ӳ���
void writeCom_12864(uchar com) {
    busyCheck_12864();
    RS_LCD = 0;
    RW_LCD = 0;
    DATA_LCD = com;
    EN_LCD = 1;
    _nop_();
    _nop_();
    //delay_us(50);    //50usʹ����ʱ!!!ע���������ǽϿ��CPUӦ����ʱ��һЩ
    EN_LCD = 0;
}

//д�����ӳ���
void writeData_12864(uchar dat) {
    //busyCheck_12864();
    RS_LCD = 1;
    RW_LCD = 0;
    DATA_LCD = dat;
    EN_LCD = 1;
    _nop_();
    _nop_();
    //delay_us(50);    //50usʹ����ʱ!!!ע���������ǽϿ��CPUӦ����ʱ��һЩ
    EN_LCD = 0;
}


/*void write_dat(uchar dat) {
    RS_LCD=1;
    RW_LCD=0;
    delay_50us(1);
    P0=dat;
    EN_LCD=1;
    delay_50us(2);
    EN_LCD=0;
    delay_50us(2);
}
*/

void waiting() {
    unsigned int park_space = getSpaceNum();
    if(working_stage == NORMAL){
	sprintf(LCDTable4, "    %02d:%02d:%02d    ", hour, minute, sec);
        strcpy(LCDTable1,"    ��ӭʹ��    ");
        if (park_space <= 0){
            sprintf(LCDTable2, "    ��λ����    ", park_space);
        }else{
            sprintf(LCDTable2, "ʣ�೵λ��%d     ", park_space);
        }

        sprintf(LCDTable3, "                ");
        //sprintf(LCDTable3, "");

        /*sprintf(LCDTable3, "    %02d:%02d:%02d    ", hour, minute, sec);*/
        //strcpy(LCDTable2, strcat("ʣ�೵λ��", (unsigned char) park_space));
        /*sprintf(LCDTable2, "ʣ�೵λ��%d    ", park_space);

        /*sprintf(LCDTable4, "By sususweet");*/
        /*strcpy(LCDTable3,nowTimeStr);
        strcpy(LCDTable4,"By sususweet");*/
    }
	if (motor_stage==MOTOR_BUSY){
        sprintf(LCDTable3, "������բע�ⰲȫ");
        twinkle_row[2] = 0;
    }else{
        sprintf(LCDTable3, "                ");
    }

}

void init_LCD(void) {
    RST_LCD = 0;
    delay_us(80);
    RST_LCD = 1;
    writeCom_12864(0x30);   //����Ϊ����ָ�����
    delay_us(100);
    writeCom_12864(0x30);   //����Ϊ����ָ�����
    delay_us(37);
    writeCom_12864(0x08);   //������ʾ����ꡢ��˸ȫ�ء�
    delay_us(100);
    writeCom_12864(0x01);   //����������DDRAM����ָ������
    delay_us(100);
    writeCom_12864(0x06);   //����ģʽ����
    writeCom_12864(0x0C);   //����ʾ���޹�꣬��겻��˸
}

void displayLCD(void) {
    static unsigned int time = 0, flag = 0;
    unsigned int i;
    writeCom_12864(0x01);   //����������DDRAM����ָ������
    //delay_us(10);
    writeCom_12864(0x80);
    for(i = 0; i < 16; i++){
        if(twinkle_row[0] == 1){
            if (flag == 0) {
                writeData_12864(LCDTable1[i]);
            } else {
                writeData_12864(0x20);
            }
        }else{
            writeData_12864(LCDTable1[i]);
        }
    }
    delay_us(10);
    writeCom_12864(0x90);
    for(i = 0; i < 16; i++){
        if(twinkle_row[1] == 1){
            if (flag == 0) {
                writeData_12864(LCDTable2[i]);
            } else {
                writeData_12864(0x20);
            }
        }else{
            writeData_12864(LCDTable2[i]);
        }
    }
    delay_us(10);
    writeCom_12864(0x88);
    for (i = 0; i < 16; i++) {
        if(twinkle_row[2] == 1){
            if (flag == 0) {
                writeData_12864(LCDTable3[i]);
            } else {
                writeData_12864(0x20);
            }
        }else{
            writeData_12864(LCDTable3[i]);
        }
    }
    delay_us(10);
    writeCom_12864(0x98);
    for (i = 0; i < 16; i++) {
        if(twinkle_row[3] == 1){
            if (flag == 0) {
                writeData_12864(LCDTable4[i]);
            } else {
                writeData_12864(0x20);
            }
        }else{
            writeData_12864(LCDTable4[i]);
        }
    }
    delay_us(10);
    time++;
    /*LCDĳЩ����˸ʱ�����*/
    /*if (time >= TWINKLE_FREQ) {
        flag = ~flag;
        time = 0;
    }*/
}


int main(void) {
    init_settings();
    init_LCD();

    TMOD = 0x01;
    EA = 1;
    init_timer0();

    while (1) {

    }
    return 0;
}

void init_settings() {
    unsigned int i = 0;
    for (i = 0; i < MAX_SPACE; i++) {
        ParkInfo parkInfo;
        parkInfo.used = 0;
        parkSpace[i] = parkInfo;
    }
    hour = atoi(strcat(&TimeStr[0], &TimeStr[1]));
    minute = atoi(strcat(&TimeStr[3], &TimeStr[4]));
    sec = atoi(strcat(&TimeStr[6], &TimeStr[7]));
}

void init_timer0() {
    ET0 = 1;
    TR0 = 1;
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
}

void interrupt0() {
    scan_key();
    EA = 0;
    startMotor(motor_stage);
    EA = 1;
    return;
}

void int0() interrupt 1{
    time_num++;
    disp_num++;
    info_num++;

    if (time_num >= TIME_FREQ) {
        time_num = 0;
        time_inc();
        nowTime++;
    }

    if (disp_num >= DISP_FREQ) {
        waiting();
        displayLCD();
        disp_num = 0;
    }

    if (info_num >= INFO_SHOW_FREQ) {
        working_stage = NORMAL;
        info_num = 0;
    }

    interrupt0();

    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
}
