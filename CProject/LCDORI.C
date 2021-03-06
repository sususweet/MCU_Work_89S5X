#include <REG51.H>
#include <INTRINS.H>
#include <STRING.H>
#include <STDLIB.H>
#include <STDIO.H>

/*#include "REG51.H"
#include "INTRINS.H"
#include "STRING.H"
#include "STDLIB.H"
#include "STDIO.H"*/

#define DATA_LCD P0
sbit RS_LCD = P2^0;
sbit RW_LCD = P2^1;
sbit EN_LCD = P2^2;
sbit PSB_LCD = P2^3;
sbit RST_LCD = P2^5;

#define KEYBOARD P1
/*注意，motor只能使用P0的低四位，同时在操作motor时记得关液晶屏的EN*/
#define MOTOR P0
/*注意，读取AD转换器数据时，记得关液晶显示EN*/

//定义ADC的连接端口
#define AD_INPUT P2
sbit AD_WR=P3^6;
sbit AD_RD=P3^7;

/*定时器0 2ms*//*
#define TIMER0_TH 0xF8
#define TIMER0_TL 0x30*/

#define TIMER0_TH 0xF8
#define TIMER0_TL 0x30
#define TWINKLE_FREQ 2    /*LCD闪烁周期*/
#define MOTOR_FREQ 10    /*电机脉冲周期*/
#define TIME_FREQ 500    /*时钟计时周期*/
#define DISP_FREQ 200   /*LCD显示刷新周期*/
#define INFO_SHOW_FREQ 1500   /*LCD信息显示时间*/

#define KEY_WAIT 10    /*键盘扫描延迟周期*/
#define MAX_SPACE 3
#define TWINKLE_ROW_NUM 4
#define MAX_CAR_WIDE 0x99   /*车宽阈值3V*/
#define MAX_KEY_CODE 16

typedef unsigned char uchar;
typedef struct {
    unsigned int startTime;
    unsigned int endTime;
    unsigned char used;
    unsigned int carID;
} ParkInfo;
enum key_states_e {
    KEY_STATE_RELEASE,
    KEY_STATE_WAITING,
    KEY_STATE_PRESSED
};
enum working_state {
    NORMAL, PARK_IN, PARK_OUT
};
enum motor_state {
    MOTOR_CLOSING, MOTOR_CLOSED, MOTOR_OPENING, MOTOR_OPENED, MOTOR_BUSY
};

/*四相双四拍*/
uchar code FFW[8]={0xf3,0xf9,0xfc,0xf6};
uchar code REV[8]={0xf3,0xf6,0xfc,0xf9};
uchar code TimeStr[] = __TIME__;
uchar idata LCDTable1[16], LCDTable2[16], LCDTable3[16], LCDTable4[16];
uchar idata twinkle_row[TWINKLE_ROW_NUM] = {0, 0, 0, 0};
unsigned int car_id = 1;
uchar working_stage = NORMAL;
uchar motor_stage = MOTOR_CLOSED;
uchar motor_cache = MOTOR_BUSY;
unsigned int nowTime = 0, info_num = 0;
unsigned int sec = 0, minute = 0, hour = 0;


ParkInfo idata parkSpace[MAX_SPACE];

void time_inc();
void opr_key(unsigned int key_code);
uchar press_key();
uchar read_key();
void send_char(unsigned char txd);
unsigned char readADC(void);
void startADC(void);

/*基础函数库开始*/
void send_char(unsigned char txd) {
    SBUF = txd;
    while (!TI);
    TI = 0;
}

void send_int(unsigned int txd){
    unsigned int high = txd/10;
    unsigned int low = txd%10;
    unsigned int result = high*16+low;
    send_char((unsigned char) result);
}

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

/* 定义一个延时xms毫秒的延时函数,xms代表需要延时的毫秒数 */
/*void delay_ms(unsigned int ms) {
    do {
        delay_us(1000);
        ms--;
    } while (ms);
}*/

/**
 * @desc 时间自增程序
 * */
void time_inc() {
    sec++;
    /*一般的时间进位逻辑*/
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

unsigned int time_plus_hour(unsigned int timeStamp){
    unsigned int hour = atoi(strcat(&TimeStr[0], &TimeStr[1]));
    unsigned int minute = atoi(strcat(&TimeStr[3], &TimeStr[4]));
    unsigned int sec = atoi(strcat(&TimeStr[6], &TimeStr[7]));
    sec += timeStamp;
    while(sec>=60){
        sec -= 60;
        minute ++;
    }
    while(minute>=60){
        minute -=60;
        hour++;
    }
    return hour;
}

unsigned int time_plus_minute(unsigned int timeStamp){
    unsigned int hour = atoi(strcat(&TimeStr[0], &TimeStr[1]));
    unsigned int minute = atoi(strcat(&TimeStr[3], &TimeStr[4]));
    unsigned int sec = atoi(strcat(&TimeStr[6], &TimeStr[7]));
    sec += timeStamp;
    while(sec>=60){
        sec -= 60;
        minute ++;
    }
    while(minute>=60){
        minute -=60;
        hour++;
    }
    return minute;
}

unsigned int time_plus_second(unsigned int timeStamp){
    unsigned int hour = atoi(strcat(&TimeStr[0], &TimeStr[1]));
    unsigned int minute = atoi(strcat(&TimeStr[3], &TimeStr[4]));
    unsigned int sec = atoi(strcat(&TimeStr[6], &TimeStr[7]));
    sec += timeStamp;
    while(sec>=60){
        sec -= 60;
        minute ++;
    }
    while(minute>=60){
        minute -=60;
        hour++;
    }
    return sec;
}


unsigned int time_hours(unsigned int endTime){
    unsigned int diff_hours;
    unsigned int seconds = endTime;
    diff_hours= (uchar) (seconds / 3600);
    return diff_hours;
}

unsigned int time_minutes(unsigned int endTime){
    unsigned int diff_minutes;
    unsigned int seconds = endTime;
    seconds %= 3600 ;
    diff_minutes= (uchar) (seconds / 60);
    return diff_minutes;
}

unsigned int time_seconds(unsigned int endTime){
    unsigned int diff_seconds;
    unsigned int seconds = endTime;
    seconds %= 3600 ;
    diff_seconds= (uchar) (seconds % 60);
    return diff_seconds;
}

void scan_key() {
    static int key_state = KEY_STATE_RELEASE;   /*状态机状态初始化，采用static保存状态*/
    unsigned int wait_time = KEY_WAIT;   /*按键扫描等待时间*/
    static uchar key_code = MAX_KEY_CODE;
    unsigned int pressed = press_key(); /*press_key为检测是否有按键按下的函数*/
    static scan_time = 0;
    switch (key_state) {
        case KEY_STATE_RELEASE: {   /*若原始状态为无按键按下RELEASE，同时又检测到按键按下，则状态转换到WAITING*/
            if (pressed == 1) {
                key_state = KEY_STATE_WAITING;
            }
            break;
        }
        case KEY_STATE_WAITING: {   /*原始状态为WAITING，对按键进行多次判断*/
            if (pressed) {
                scan_time++;
                if (scan_time >= wait_time) {   /*若按键按下的时间超过一定时间，则认为按键按下，读按键*/
                    key_state = KEY_STATE_PRESSED;
                    scan_time = 0;
                    key_code = read_key();  /*read_key为读按键的函数*/
                }
            } else {    /*若按键松开，则恢复到初始状态*/
                scan_time = 0;
                key_state = KEY_STATE_RELEASE;
            }
            break;
        }
        case KEY_STATE_PRESSED: {   /*若按键被确认按下，则等待按键松开再进行操作*/
            if (pressed == 0) {
                opr_key(key_code);  /*opr_key为按键事件响应函数*/
                key_state = KEY_STATE_RELEASE;
                key_code = MAX_KEY_CODE;
            }
            break;
        }
        default:
            break;
    }
}

//判断是否有键按下
uchar press_key() {
    unsigned char temp_line, temp_row;
    KEYBOARD = 0xF0;
    temp_line = KEYBOARD;
    if (temp_line == 0xF0) return 0;

    KEYBOARD = 0x0F;
    temp_row = KEYBOARD;
    if (temp_row == 0x0F) return 0;
    return 1;
}

//扫描键盘，返回键值
uchar read_key() {
    uchar temp_column, temp_row;
    uchar row, column, key_code;
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

void scan_emergency(){
    uchar tmp;
    if (RI){
        RI = 0;
        tmp = SBUF;
        if (tmp == 0xff){
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
    }
}
/*基础函数库结束*/

/* 根据当前状态全局变量给电机提供驱动脉冲 */
void startMotor(uchar type){
    static unsigned char motor_index = 0, motor_count = 0;
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
                default:return;
            }
            break;
        }
        default:return;
    }

    motor_index++;
    if (motor_index >= 4) {
        motor_index = 0;
        motor_count++;
        if(motor_count>=60) {
            motor_stage = motor_cache;
            motor_count = 0;
        }
    }
}

/*获取剩余空间*/
int getSpace() {
    unsigned int i = 0;
    for (i = 0; i < MAX_SPACE; i++) {
        if (parkSpace[i].used == 0) return i;
    }
    return -1;
}

/*获取剩余空间数目*/
unsigned int getSpaceNum() {
    unsigned int i = 0, count = 0;
    for (i = 0; i < MAX_SPACE; i++) {
        if (parkSpace[i].used == 0) count++;
    }
    return count;
}

// 传送字串
void send_str(unsigned int carID, unsigned int startTime, unsigned int endTime) {
    //unsigned char i = 0;
    //printf("00-浙A%04d-进入时间（%02d:%02d:%02d）-离开时间（%02d:%02d:%02d）-00", carID, time_hours(startTime), time_minutes(startTime), time_seconds(startTime), time_hours(endTime), time_minutes(endTime), time_seconds(endTime));
    send_char(00);
    //send_char('-');
    send_int((unsigned char) carID);
    //send_char('-');
    send_int(time_plus_hour(startTime));
    //send_char(':');
    send_int(time_plus_minute(startTime));
    // send_char(':');
    send_int(time_plus_second(startTime));
    // send_char('-');
    send_int(time_plus_hour(endTime));
    //send_char(':');
    send_int(time_plus_minute(endTime));
    // send_char(':');
    send_int(time_plus_second(endTime));
    //send_char('-');
    send_char(00);
    /*while(sendStr[i] != '\0') {
        SBUF = sendStr[i];
        while(!TI);     // 等特数据传送
        TI = 0;      // 清除数据传送标志
        i++;      // 下一个字符
    }
    i = 0;*/
}


void park_in(unsigned int id){
    unsigned char car_wide, car_wide2;
    clr_twinkle();

    car_wide = readADC();
    car_wide2 = readADC();
    while(car_wide2 != car_wide){
        car_wide = readADC();
        car_wide2 = readADC();
    }

    if(car_wide > MAX_CAR_WIDE){
        strcpy(LCDTable1,"    欢迎光临    ");
        sprintf(LCDTable2, "车牌号:%03d", car_id);
        strcpy(LCDTable3, "    车宽超限    ");
        twinkle_row[2] = 1;
    }else{
        parkSpace[id].startTime = nowTime;
        parkSpace[id].carID = car_id;
        parkSpace[id].used = 1;
        strcpy(LCDTable1,"    欢迎光临    ");
        sprintf(LCDTable2, "车牌号:%03d", parkSpace[id].carID);
        sprintf(LCDTable3, "车位:%d  开始计费",id + 1);
        car_id++;
        //sprintf(LCDTable4, "    %02d:%02d:%02d    ", hour, minute, sec);
    }
    working_stage = PARK_IN;
    info_num = 0;
    return;
}

void park_out(unsigned int id){
    unsigned int park_hours,park_minutes,park_seconds;
    /*unsigned char end_hours,end_minutes,end_seconds;
    unsigned char start_hours,start_minutes,start_seconds;*/
    clr_twinkle();

    if (parkSpace[id].used == 1){
        parkSpace[id].endTime = nowTime;
        parkSpace[id].used = 0;
        park_hours = time_hours(parkSpace[id].endTime-parkSpace[id].startTime);
        park_minutes = time_minutes(parkSpace[id].endTime-parkSpace[id].startTime);
        park_seconds = time_seconds(parkSpace[id].endTime-parkSpace[id].startTime);
        sprintf(LCDTable1, "车牌号:%03d", parkSpace[id].carID);
        sprintf(LCDTable2, "  应付费：%1.2f  ",(float) ((parkSpace[id].endTime - parkSpace[id].startTime) * 0.5));
        sprintf(LCDTable3, "停车时间%02d:%02d:%02d", park_hours, park_minutes, park_seconds);
        /*end_hours = time_hours(parkSpace[id].endTime);
        end_minutes = time_minutes(parkSpace[id].endTime);
        end_seconds = time_seconds(parkSpace[id].endTime);
        start_hours = time_hours(parkSpace[id].startTime);
        start_minutes = time_minutes(parkSpace[id].startTime);
        start_seconds = time_seconds(parkSpace[id].startTime);       */
        send_str(parkSpace[id].carID,  parkSpace[id].startTime, parkSpace[id].endTime);
    }else{
        strcpy(LCDTable1,"    欢迎使用    ");
        strcpy(LCDTable2, "  车位没有车辆  ");
        twinkle_row[1] = 1;
        strcpy(LCDTable3, "                ");
        //sprintf(LCDTable4, "    %02d:%02d:%02d    ", hour, minute, sec);
    }
    working_stage = PARK_OUT;
    info_num = 0;
    return;
}

void waiting() {
    unsigned int park_space = getSpaceNum();
    if(working_stage == NORMAL){
        clr_twinkle();
        sprintf(LCDTable4, "    %02d:%02d:%02d    ", hour, minute, sec);
        strcpy(LCDTable1,"    欢迎使用    ");
        if (park_space <= 0){
            sprintf(LCDTable2, "    车位已满    ", park_space);
            twinkle_row[1] = 1;
        }else{
            sprintf(LCDTable2, "剩余车位：%d     ", park_space);
        }
        if (motor_stage == MOTOR_BUSY){
            sprintf(LCDTable3, "道闸工作注意安全");
            twinkle_row[2] = 1;
        }else{
            sprintf(LCDTable3, "                ");
        }
        /*sprintf(LCDTable3, "    %02d:%02d:%02d    ", hour, minute, sec);*/
        /*sprintf(LCDTable2, "剩余车位：%d    ", park_space);*/
    }else{
        sprintf(LCDTable4, "    %02d:%02d:%02d    ", hour, minute, sec);
    }

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
        case 12: {
            switch (motor_stage){
                case MOTOR_BUSY:{
                    switch (motor_cache){
                        case MOTOR_OPENED:{
                            motor_stage = MOTOR_CLOSING;
                            break;
                        }
                        case MOTOR_CLOSED:{
                            motor_stage = MOTOR_OPENING;
                            break;
                        }
                        default:break;
                    }
                    break;
                }
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


//忙检测，若忙则等待，最长等待时间为60ms
void busyCheck_12864(void) {
    unsigned int timeout = 0;
    DATA_LCD = 0xff;
    RS_LCD = 0;
    RW_LCD = 1;
    EN_LCD = 1;
    while ((DATA_LCD & 0x80) && ++timeout != 0);  //忙状态检测，等待超时时间为60ms
    EN_LCD = 0;
}

//写命令子程序
void writeCom_12864(uchar com) {
    busyCheck_12864();
    RS_LCD = 0;
    RW_LCD = 0;
    DATA_LCD = com;
    EN_LCD = 1;
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();

    //delay_us(10);    //50us使能延时!!!注意这里，如果是较快的CPU应该延时久一些
    EN_LCD = 0;
}

//写数据子程序
void writeData_12864(uchar dat) {
    //busyCheck_12864();
    RS_LCD = 1;
    RW_LCD = 0;
    _nop_();
    _nop_();
    DATA_LCD = dat;
    EN_LCD = 1;
    _nop_();
    _nop_();
    //delay_us(10);    //50us使能延时!!!注意这里，如果是较快的CPU应该延时久一些
    EN_LCD = 0;
}

void init_LCD(void) {
    RST_LCD = 0;
    delay_us(80);
    RST_LCD = 1;
    writeCom_12864(0x30);   //设置为基本指令集动作
    delay_us(100);
    writeCom_12864(0x30);   //设置为基本指令集动作
    delay_us(37);
    writeCom_12864(0x08);   //设置显示、光标、闪烁全关。
    delay_us(100);
    writeCom_12864(0x01);   //清屏，并且DDRAM数据指针清零
    delay_us(100);
    writeCom_12864(0x06);   //进入模式设置
    writeCom_12864(0x0C);   //开显示，无光标，光标不闪烁
}

void displayLCD(void) {
    static unsigned int time = 0, flag = 0;
    unsigned int i;
    writeCom_12864(0x01);   //清屏，并且DDRAM数据指针清零
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
    _nop_();
    _nop_();

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
    _nop_();
    _nop_();

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
    _nop_();
    _nop_();
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
    _nop_();
    _nop_();
    time++;
    /*LCD某些行闪烁时间控制*/

    if (time >= TWINKLE_FREQ) {
        flag = ~flag;
        time = 0;
    }
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

void init_com_send() {
    ES = 0;
    ET1 = 0;
    TR1 = 1;
    /*波特率2400*/
    TH1 = 0xF3;
    TL1 = 0xF3;
    SCON = 0x50;
    PCON &= 0xEF;
}

void startADC(void){
    AD_WR = 1;
    _nop_();
    _nop_();
    AD_WR = 0;
    _nop_();
    _nop_();
    AD_WR = 1;
}

unsigned char readADC(void){
    unsigned char output;
    AD_INPUT = 0xff;
    AD_WR = 0;
    _nop_();
    AD_RD = 0;
    _nop_();
    delay_us(1);
    output = AD_INPUT;
    _nop_();
    AD_RD = 1;
    _nop_();
    AD_WR = 1;
    //init_LCD();
    return output;
}

int main(void) {
    init_settings();
    init_LCD();

    TMOD = 0x21;
    EA = 1;

    init_timer0();
    init_com_send();
    startADC();
    while (1) {

    }
    return 0;
}

void interrupt0() {
    scan_key();
    scan_emergency();
    return;
}

void int0() interrupt 1{
    static unsigned int time_num = 0;
    static unsigned char motor_num = 0, disp_num = 0;
    time_num++;
    disp_num++;
    info_num++;
    motor_num++;

    if (time_num >= TIME_FREQ) {
        time_num = 0;
        time_inc();
        nowTime++;
    }

    if (motor_num >= MOTOR_FREQ) {
        startMotor(motor_stage);
        motor_num = 0;
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

