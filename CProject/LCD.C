/*#include <REG52.H>
#include <INTRINS.H>
#include <STRING.H>
#include <STDLIB.H>
#include <STDIO.H>*/

#include "REG52.H"
#include "INTRINS.H"
#include "STRING.H"
#include "STDLIB.H"
#include "STDIO.H"

char code SST516[3] _at_ 0x003b; //仿真器保留

#define DATA_LCD P0
sbit RS_LCD = P2^0;
sbit RW_LCD = P2^1;
sbit EN_LCD = P2^2;
sbit PSB_LCD = P2^3;
sbit RST_LCD = P2^5;

sbit AD_WR=P3^6;
sbit AD_RD=P3^7;
sbit RD_SDA=P3^3; //定义数据线
sbit WR_SCL=P3^2; //定义时钟线

#define KEYBOARD P1
/*注意，motor只能使用P0的低四位，同时在操作motor时记得关液晶屏的EN*/
#define MOTOR P0
//定义ADC的连接端口
#define AD_INPUT P0
/*注意，读取AD转换器数据时，记得关液晶显示EN*/

#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};
#define OP_WRITE 0xa0 // 器件地址以及写入操作
#define OP_READ 0xa1 // 器件地址以及读取操作


/*定时器0 2ms*/
#define TIMER0_TH 0xF8
#define TIMER0_TL 0x30
#define TWINKLE_FREQ 2    /*LCD闪烁周期*/
#define MOTOR_FREQ 13    /*电机脉冲周期*/
#define TIME_FREQ 500    /*时钟计时周期*/
#define DISP_FREQ 250   /*LCD显示刷新周期*/
#define INFO_SHOW_FREQ 1500   /*LCD信息显示时间*/

#define KEY_WAIT 10    /*键盘扫描延迟周期*/
#define MAX_SPACE 3
#define TWINKLE_ROW_NUM 4
#define MAX_CAR_WIDE 0x99   /*车宽阈值3V*/
#define MAX_KEY_CODE 16
#define MAX_MOTOR_COUNT 4
#define MAX_MOTOR_CIRCLE 60

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
    NORMAL, PARK_IN, PARK_IN_NOVALID, PARK_OUT, PARK_OUT_EMPTY, PARK_IN_MOTOR, PARK_OUT_MOTOR
};
enum motor_state {
    MOTOR_CLOSING, MOTOR_CLOSED, MOTOR_OPENING, MOTOR_OPENED, MOTOR_BUSY
};
enum return_type {
    SECOND, MINUTE, HOUR
};

/*四相双四拍*/
uchar code FFW[4]={0xf3,0xf9,0xfc,0xf6};
uchar code REV[4]={0xf3,0xf6,0xfc,0xf9};
uchar code TimeStr[] = __TIME__;
uchar code LCDRows[] = {0x80,0x90,0x88,0x98};
/*uchar code display_twinkle[4][16]={
{"车位已满"},
{"车位没有车辆"},
{"注意安全"},
{"车宽超限"}
};*/

uchar code display_welcome[5][16]={
{"欢迎光临"},
{"一路顺风"},
{"请您通行"},
{"费用："},
{"元/ 秒"}
};
uchar code display_space[5][16]={
{"车位已满"},
{"剩余车位："},
{"车位没有车辆"},
{"注意安全"}
};

uchar code display_park_info[5][16]={
{"车牌号：浙A "},
{"车位："},
{"应付费："},
{"停车时间"},
{"车宽超限"}
};

uchar displayCache[9];
//uchar idata twinkleCache[4];
/*uchar twinkle_row, twinkleIndex;*/

unsigned int car_id = 1;
uchar working_stage = NORMAL;
uchar motor_stage = MOTOR_CLOSED;
uchar motor_cache = MOTOR_BUSY;
unsigned int nowTime = 0, info_num = 0;
uchar sec = 0, minute = 0, hour = 0;
uchar sec_begin, minute_begin, hour_begin;
uchar car_wide;
unsigned int time_num = 0;
uchar motor_num = 0;
uchar motor_index = 0, motor_count = 0;
uchar pays = 5;
ParkInfo idata parkSpace[MAX_SPACE];
uchar park_space = MAX_SPACE;
uchar state_change = 1;

void time_inc();
void opr_key(uchar key_code);
uchar press_key();
uchar read_key();
void send_char(uchar txd);
uchar getLCDMiddleByLength (uchar length, uchar address);
uchar getLCDMiddleByStr (uchar *str, uchar address);
uchar displayLCDRow(uchar *str, uchar address);
void displayLCDRowMiddle(uchar *str, uchar address);
void LcdFullClear();
void startLineTwinkle(uchar* str, uchar index);
void LCDLineClear(unsigned char row);


/*基础函数库开始*/
void send_char(unsigned char txd) {
    SBUF = txd;
    while (!TI);
    TI = 0;
}

void send_int(unsigned int txd){
    send_char((unsigned char) (txd / 10 * 16 + txd % 10));
}

void delay_us(unsigned int us) {
    do {
        _nop_();
        us--;
    } while (us);
}

/* 定义一个延时xms毫秒的延时函数,xms代表需要延时的毫秒数 */
void delay_ms(unsigned int ms) {
    do {
        delay_us(1000);
        ms--;
    } while (ms);
}

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

uchar time_plus(unsigned int timeStamp, uchar type){
    uchar hour = hour_begin;
    uchar minute = minute_begin;
    uchar sec = sec_begin;
    sec += timeStamp;
    while(sec>=60){
        sec -= 60;
        minute ++;
    }
    while(minute>=60){
        minute -=60;
        hour++;
    }
    switch (type){
        case SECOND:{
            return sec;
        }
        case MINUTE:{
            return minute;
        }
        case HOUR:{
            return hour;
        }
        default:break;
    }
    return 0;
}

unsigned int time_diff_hours(unsigned int endTime){
    unsigned int diff_hours;
    unsigned int seconds = endTime;
    diff_hours= (uchar) (seconds / 3600);
    return diff_hours;
}

unsigned int time_diff_minutes(unsigned int endTime){
    unsigned int diff_minutes;
    unsigned int seconds = endTime;
    seconds %= 3600 ;
    diff_minutes= (uchar) (seconds / 60);
    return diff_minutes;
}

unsigned int time_diff_seconds(unsigned int endTime){
    unsigned int diff_seconds;
    unsigned int seconds = endTime;
    seconds %= 3600 ;
    diff_seconds= (uchar) (seconds % 60);
    return diff_seconds;
}

void scan_key() {
    static uchar key_state = KEY_STATE_RELEASE;   /*状态机状态初始化，采用static保存状态*/
    static uchar key_code = MAX_KEY_CODE;
    uchar pressed = press_key(); /*press_key为检测是否有按键按下的函数*/
    static uchar scan_time = 0;
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
                if (scan_time >= KEY_WAIT) {   /*若按键按下的时间超过一定时间，则认为按键按下，读按键*/
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
    uchar temp_line, temp_row;
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
    key_code = (uchar) (4 * row + column);
    return key_code;
}

void changeMotorState(){
    if (working_stage == NORMAL){
        state_change = 1;
        LCDLineClear(2);
    }

    switch (motor_stage){
        case MOTOR_BUSY:{
            switch (motor_cache){
                case MOTOR_OPENED:{
                    motor_stage = MOTOR_CLOSING;
                    motor_index = (uchar) (MAX_MOTOR_COUNT - motor_index);
                    motor_count = (uchar) (MAX_MOTOR_CIRCLE - motor_count);
                    break;
                }
                case MOTOR_CLOSED:{
                    motor_stage = MOTOR_OPENING;
                    motor_index = (uchar) (MAX_MOTOR_COUNT - motor_index);
                    motor_count = (uchar) (MAX_MOTOR_CIRCLE - motor_count);
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

void scan_emergency(){
    uchar tmp;
    if (RI){
        RI = 0;
        tmp = SBUF;
        if (tmp == 0xff){
            changeMotorState();
        }else{
            pays = tmp;
            state_change = 1;
        }
    }
}
/*基础函数库结束*/

/* 根据当前状态全局变量给电机提供驱动脉冲 */
void startMotor(uchar type){
    switch (type){
        /*道闸关闭状态被激活，电机开启关道闸模式*/
        case MOTOR_CLOSING:{
            motor_cache = MOTOR_CLOSED;
            motor_stage = MOTOR_BUSY;
            MOTOR = FFW[motor_index];
            break;
        }
        /*道闸开启状态被激活，电机开启开道闸模式*/
        case MOTOR_OPENING:{
            motor_cache = MOTOR_OPENED;
            motor_stage = MOTOR_BUSY;
            MOTOR = REV[motor_index];
            break;
        }
        /*道闸正忙，接收紧急开关信号*/
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

    /*电机一共转5圈，随后停止*/
    if (motor_index >= MAX_MOTOR_COUNT) {
        motor_index = 0;
        motor_count++;
        if(motor_count >= MAX_MOTOR_CIRCLE) {
            motor_stage = motor_cache;
            working_stage = NORMAL;
            state_change = 1;
            LcdFullClear();
            motor_count = 0;
        }
    }
}

/*获取剩余空间*/
unsigned int getSpace() {
    unsigned int i = 0;
    for (i = 0; i < MAX_SPACE; i++) {
        if (parkSpace[i].used == 0) return i;
    }
    return MAX_SPACE;
}

// 传送字串
void send_str(unsigned int carID, unsigned int startTime, unsigned int endTime) {
    send_char(00);
    //send_char('-');
    send_int(carID);
    //send_char('-');
    send_int(time_plus(startTime,HOUR));
    //send_char(':');
    send_int(time_plus(startTime,MINUTE));
    // send_char(':');
    send_int(time_plus(startTime,SECOND));
    // send_char('-');
    send_int(time_plus(endTime,HOUR));
    //send_char(':');
    send_int(time_plus(endTime,MINUTE));
    // send_char(':');
    send_int(time_plus(endTime,SECOND));
    //send_char('-');
    send_char(00);
}

void disp_time(){
    uchar middleAddr = 0x9A;
    //char hourcache;
    //sprintf(displayCache, "%02d:%02d:%02d", (uchar)hour, (uchar)minute, (uchar)sec);
    displayCache[0] = (uchar) (hour / 10 + '0');
    displayCache[1] = (uchar) (hour % 10 + '0');
    displayCache[2] = ':';
    displayCache[3] = (uchar) (minute / 10 + '0');
    displayCache[4] = (uchar) (minute % 10 + '0');
    displayCache[5] = ':';
    displayCache[6] = (uchar) (sec / 10 + '0');
    displayCache[7] = (uchar) (sec % 10 + '0');
    displayCache[8] = '\0';

    //middleAddr = displayLCDRow(hour, middleAddr);
    displayLCDRow(displayCache, middleAddr);
   /* sprintf(strCache, "%02d",minute);
    middleAddr = displayLCDRow((uchar *) strCache, middleAddr);
    middleAddr = displayLCDRow((uchar *) ':', middleAddr);
    sprintf(strCache, "%02d",(unsigned int)sec);
    middleAddr = displayLCDRow((uchar *) strCache, middleAddr);*/
    return;
}

void park_in(unsigned int id){
    uchar address_cache;
    state_change = 1;
    LcdFullClear();
    disp_time();

    displayLCDRowMiddle(display_welcome[0],LCDRows[0]);

    /*显示车牌号*/
    address_cache = displayLCDRow(display_park_info[0],getLCDMiddleByLength(16,LCDRows[1]));
    sprintf((char *) displayCache, "%04d", car_id);
    /*
    displayCache[0] = (uchar) (car_id / 10 + '0');
    displayCache[1] = (uchar) (car_id % 10 + '0');
    displayCache[2] = '\0';*/
    displayLCDRow(displayCache, address_cache);


    if(car_wide > MAX_CAR_WIDE){
        /*车宽超限提醒*/
        /*twinkleIndex = 3;
        twinkle_row = 2;*/
        //startLineTwinkle(display_park_info[4], 2);
        displayLCDRowMiddle(display_park_info[4], LCDRows[2]);
        working_stage = PARK_IN_NOVALID;
    }else{
        park_space -= 1;
        parkSpace[id].startTime = nowTime;
        parkSpace[id].carID = car_id;
        parkSpace[id].used = 1;
        /*显示车位序号*/
        address_cache = displayLCDRow(display_park_info[1],getLCDMiddleByLength(6,LCDRows[2]));
        sprintf((char *) displayCache, "%d", id + 1);
        displayLCDRow(displayCache, address_cache);
        car_id++;
        working_stage = PARK_IN;
    }

    info_num = 0;
    return;
}

void park_out(unsigned int id){
    unsigned int park_hours,park_minutes,park_seconds;
    uchar address_cache;
    state_change = 1;
    LcdFullClear();

    if (parkSpace[id].used == 1){
        /*首先判断此停车位是否已经被使用，如果被使用则成功出车*/
        displayLCDRowMiddle(display_welcome[1],LCDRows[0]);

        parkSpace[id].endTime = nowTime;
        parkSpace[id].used = 0;
        park_hours = time_diff_hours(parkSpace[id].endTime - parkSpace[id].startTime);
        park_minutes = time_diff_minutes(parkSpace[id].endTime - parkSpace[id].startTime);
        park_seconds = time_diff_seconds(parkSpace[id].endTime - parkSpace[id].startTime);

        /*显示车牌号*/
        address_cache = displayLCDRow(display_park_info[0],LCDRows[1]);
        sprintf(displayCache, "%04d",parkSpace[id].carID);
        displayLCDRow(displayCache, address_cache);

        /*停车费用显示*/
        address_cache = displayLCDRow(display_park_info[2],LCDRows[2]);
        sprintf(displayCache,"%1.2f",(float) ((parkSpace[id].endTime - parkSpace[id].startTime) * pays / 10));
        displayLCDRow(displayCache, address_cache);

        /*停车时间显示*/
        address_cache = displayLCDRow(display_park_info[3],LCDRows[3]);
        park_space += 1;
        sprintf((char *) displayCache, "%02d:%02d:%02d", park_hours, park_minutes, park_seconds);
        address_cache = displayLCDRow(displayCache, address_cache);
        send_str(parkSpace[id].carID,  parkSpace[id].startTime, parkSpace[id].endTime);
        working_stage = PARK_OUT;
    }else{
        /*停车位未被使用，显示相关提示信息*/
        disp_time();
        displayLCDRowMiddle(display_welcome[0],LCDRows[0]);

        /*车位没有车辆提醒*/
        //startLineTwinkle(display_space[2], 1);
        displayLCDRowMiddle(display_space[2],LCDRows[1]);
        //twinkle_row[1] = 1;
        working_stage = PARK_OUT_EMPTY;
        info_num = 0;
        //strcpy(twinkleCache, display_space[2]);
    }


    return;
}

void waiting() {
    uchar address_cache;
    if (working_stage == NORMAL && state_change == 1) {
        //LcdFullClear();
        state_change = 0;
        displayLCDRowMiddle(display_welcome[0],LCDRows[0]);
        if (park_space <= 0) {
            displayLCDRowMiddle(display_space[0],LCDRows[1]);
            //startLineTwinkle(display_space[0], 1);
            /*displayLCDRowMiddle(display_space[0],LCDRows[1]);
            //strcpy(twinkleCache, display_space[0]);
            twinkle_row[1] = 1;*/
        } else {
            address_cache = displayLCDRow(display_space[1],LCDRows[1]);
            displayCache [0] =  (uchar) (park_space % 10 + '0');
            displayCache [1] = '\0';
            //sprintf((char *) displayCache, "%d", park_space);
            displayLCDRow(displayCache, address_cache);


            /*displayCache[0] = (uchar) (park_space % 10 + '0');
            displayCache[1] = '\0';*/

            //sprintf(LCDTable2, "剩余车位：%d     ", park_space);
        }

        if (motor_stage == MOTOR_BUSY) {
           // LCDLineClear(2);
            displayLCDRowMiddle(display_space[3],LCDRows[2]);
            state_change = 0;
           // twinkleIndex = 2;
            //twinkle_row = 2;

            //startLineTwinkle(display_space[3], 2);
           /* displayLCDRowMiddle(display_space[3],LCDRows[2]);
            twinkle_row[2] = 1;*/
        }else{
            //LCDLineClear(2);
            address_cache = displayLCDRow(display_welcome[3],LCDRows[2]);
            sprintf(displayCache,"%1.2f",(float) pays / 10);
            displayLCDRow(displayCache, address_cache);
            displayLCDRow(display_welcome[4], 0x8D);
            state_change = 0;
        }
    }
    if (working_stage != PARK_OUT && working_stage != PARK_OUT_MOTOR){
        disp_time();
    }
}

void opr_key(uchar key_code) {
    switch (key_code) {
        case 0: {
            if (working_stage == NORMAL){
                unsigned int space_id = getSpace();
                if (space_id != MAX_SPACE){
                    park_in(space_id);
                }
            }
            break;
        }
        case 1: {
            if (working_stage == NORMAL){
                park_out(0);
            }
            break;
        }
        case 2: {
            if (working_stage == NORMAL){
                park_out(1);
            }
            break;
        }
        case 3: {
            if (working_stage == NORMAL){
                park_out(2);
            }
            break;
        }
        case 12: {
            switch (working_stage){
                case PARK_IN:{
                    working_stage = PARK_IN_MOTOR;
                    //LCDLineClear(0);
                    displayLCDRowMiddle(display_welcome[2],LCDRows[0]);
                    break;
                }
                case PARK_OUT:{
                    working_stage = PARK_OUT_MOTOR;
                    //LCDLineClear(0);
                    displayLCDRowMiddle(display_welcome[2],LCDRows[0]);
                    break;
                }
                default:break;
            }

            changeMotorState();
        }
        default: break;
    }
}


//忙检测，若忙则等待，最长等待时间为60ms
void busyCheck_12864(void) {
    uchar timeout = 0;
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
    delay_us(50);    //50us使能延时!!!注意这里，如果是较快的CPU应该延时久一些
    EN_LCD = 0;
}

//写数据子程序
void writeData_12864(uchar dat) {
    //busyCheck_12864();
    RS_LCD = 1;
    RW_LCD = 0;
    DATA_LCD = dat;
    EN_LCD = 1;
    delay_us(50);    //50us使能延时!!!注意这里，如果是较快的CPU应该延时久一些
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

void LCDLineClear(unsigned char row) {
    uchar len=16;
    writeCom_12864(LCDRows[row]);
    while (len--) {
        writeData_12864(0x20);
    }
}

/*void clr_twinkle(){
    twinkle_row = TWINKLE_ROW_NUM;
    return;
}          */

void LcdFullClear() {
    writeCom_12864(0x01);
    //clr_twinkle();
}
/*void startLineTwinkle(uchar* str, uchar index) {
    sprintf(displayCache, str);
    twinkle_row = index;
    return;
}*/
/*void LCDLineTwinkle() {
    static bit flag = 0;
    static uchar time = 0;
    if (twinkle_row == TWINKLE_ROW_NUM) return;
    if (flag == 0) {
        displayLCDRowMiddle(display_twinkle[twinkleIndex],LCDRows[twinkle_row]);
    } else {
        LCDLineClear(twinkle_row);
    }
    time++;

    /*LCD某些行闪烁时间控制*/
    /*if (time >= TWINKLE_FREQ) {
        flag = ~flag;
        time = 0;
    }
} */

uchar getLCDMiddleByLength (uchar length, uchar address){
    uchar middle_addr = address;
	middle_addr = (uchar) (middle_addr + (16 - length) / 4);
    return middle_addr;
}

uchar getLCDMiddleByStr (uchar *str, uchar address){
   uchar index = 0, middle_addr = address;
   while (*str != '\0') {
       str++;
       index++;
   }
   middle_addr = (uchar) (middle_addr + (16 - index) / 4);
   return middle_addr;
}
uchar displayLCDRow(uchar *str, uchar address){
   uchar i = 0, length = 0;
   writeCom_12864(address);
   while (*str != '\0') {
       writeData_12864(*str);
       str++;
       i++;
       length++;
       if (i == 2){
           i = 0;
           address++;
       }
   }
   /* for(;length<16;length++){
        writeData_12864(0x20);
    }*/
   return address;
}
/*注意这个函数只能输入行首地址！！！*/
void displayLCDRowMiddle(uchar *str, uchar address){
    uchar addr = getLCDMiddleByStr(str, address);
    displayLCDRow(str,addr);
    return;
}

void init_settings() {
    uchar i = 0;
    for (i = 0; i < MAX_SPACE; i++) {
        ParkInfo parkInfo;
        parkInfo.used = 0;
        parkSpace[i] = parkInfo;
    }
    hour = atoi(strcat(&TimeStr[0], &TimeStr[1]));
    minute = atoi(strcat(&TimeStr[3], &TimeStr[4]));
    sec = atoi(strcat(&TimeStr[6], &TimeStr[7]));
    hour_begin = atoi(strcat(&TimeStr[0], &TimeStr[1]));
    minute_begin = atoi(strcat(&TimeStr[3], &TimeStr[4]));
    sec_begin = atoi(strcat(&TimeStr[6], &TimeStr[7]));
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
    PCON &= 0x7F;
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
    _nop_();
    //delay_us(1);
    output = AD_INPUT;
    _nop_();
    AD_RD = 1;
    _nop_();
    AD_WR = 1;
    return output;
}



void start() //I2C启动函数
//开始位
{
    RD_SDA = 1; //使能 RD_SDA
    WR_SCL = 1;
    delayNOP();
    RD_SDA = 0;
    delayNOP();
    WR_SCL = 0;
}

void stop() //I2C停止函数
// 停止位
{
    RD_SDA = 0;
    delayNOP();
    WR_SCL = 1;
    delayNOP();
    RD_SDA = 1;
}

bit shout(uchar write_data)
// 从MCU移出数据到AT24C02
{
    uchar i;
    bit ack_bit;
    for(i = 0; i < 8; i++) // 循环移入8个位
    {
        RD_SDA = (bit)(write_data & 0x80);
        _nop_();
        WR_SCL = 1;
        delayNOP();
        WR_SCL = 0;
        write_data <<= 1;
    }
    RD_SDA = 1; // 读取应答
    delayNOP();
    WR_SCL = 1;
    delayNOP();
    ack_bit = RD_SDA;
    WR_SCL = 0;
    return ack_bit; // 返回AT24C02应答位
}

void write_byte(uchar addr, uchar write_data)
// 在指定地址addr处写入数据write_data
{
    start();
    shout(OP_WRITE);
    shout(addr);
    shout(write_data);
    stop();
    delay_ms(1); // 写入周期
}

void write_int(uchar addr, unsigned int write_data) {
    write_byte(addr,(uchar) (write_data >> 0x08));
    write_byte((uchar) (addr + 1), (uchar) (write_data));
}

uchar shin()
// 从AT24C02移出数据到MCU
{
    uchar i,read_data;
    for(i = 0; i < 8; i++)
    {
        WR_SCL = 1;
        read_data <<= 1; //数据左移一位
        read_data |= RD_SDA;
        WR_SCL = 0;
    }
    return(read_data);
}

uchar read_current()
// 在当前地址读取
{
    uchar read_data;
    start();
    shout(OP_READ);
    read_data = shin();
    stop();
    return read_data;
}

uchar read_random(uchar random_addr)
// 在指定地址读取
{
    start();
    shout(OP_WRITE);
    shout(random_addr);
    return(read_current());
}


unsigned int read_random_int(uchar random_addr) {
    unsigned int tmp,tmp2;
    tmp =  (unsigned int) read_random(random_addr);
    tmp2 =  (unsigned int) read_random((uchar) (random_addr + 1));
    tmp = (tmp << 8) + tmp2;
    return tmp;
}

void interrupt0() {
    scan_key();
    scan_emergency();
    return;
}

void storeData(){
    unsigned int i = 0;
    uchar j = 0;
    write_byte(0, hour);
    write_byte(1, minute);
    write_byte(2, sec);
    write_int(3, nowTime);
    write_int(5, car_id);
    write_byte(7, pays);
    write_byte(8, park_space);

    j = 9;
    for (i = 0; i < MAX_SPACE; i++) {
        write_int(j, parkSpace[i].startTime);
        j+=2;
        write_int(j, parkSpace[i].endTime);
        j+=2;
        write_byte(j, parkSpace[i].used);
        j++;
        write_int(j, parkSpace[i].carID);
        j+=2;
    }
    return;
}

void recoverData(){
    unsigned int i = 0;
    uchar j = 0;
    hour = read_random(0);
    minute = read_random(1);
    sec = read_random(2);
    nowTime = read_random_int(3);
    car_id = read_random_int(5);
    pays = read_random(7);
    park_space = read_random(8);
    j = 9;
    for (i = 0; i < MAX_SPACE; i++) {
        parkSpace[i].startTime = read_random_int(j);
        j+=2;
        parkSpace[i].endTime = read_random_int(j);
        j+=2;
        parkSpace[i].used = read_random(j);
        j++;
        parkSpace[i].carID = read_random_int(j);
        j+=2;
    }
    hour_begin = hour;
    minute_begin = minute;
    sec_begin = sec;
    return;
}

void int0() interrupt 1{
    time_num++;
    //disp_num++;
    info_num++;
    motor_num++;

    if (time_num >= TIME_FREQ) {
        time_num = 0;
        time_inc();
        waiting();
        nowTime++;
        //LCDLineTwinkle();
        /*掉电保护*/
        if (motor_stage != MOTOR_BUSY){
            storeData();
            writeCom_12864(0x0C);
            car_wide = readADC();
        }

        //write_byte(3,222);
        //read_random(55);
        /*WR_SCL = 0;
        write_byte(3,222);
        read_random(55); //读取单片机复位次数*/
    }

    /*if (disp_num >= DISP_FREQ) {

        //disp_time();
        //car_wide = readADC();
        disp_num = 0;
    }*/

    if (motor_num >= MOTOR_FREQ) {
        startMotor(motor_stage);
        motor_num = 0;
    }


    if (info_num >= INFO_SHOW_FREQ && (working_stage == PARK_OUT_EMPTY || working_stage == PARK_IN_NOVALID)) {
        working_stage = NORMAL;
        info_num = 0;
        state_change = 1;
        LcdFullClear();
    }

    interrupt0();

    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
}


void main() {
    init_settings();
    recoverData();
    init_LCD();
    TMOD = 0x21;
    EA = 1;

    init_timer0();
    init_com_send();
    startADC();
    while (1) {

    }
    return;
}
