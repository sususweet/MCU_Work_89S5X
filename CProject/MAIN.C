#include <reg51.h>
#include <intrins.h>
/*#include "REG51.H"
#include "INTRINS.H"  */

/*char code SST516[3] _at_ 0x003b;*/

#define DIGITAL_ARR_NUM 19
#define TWINKLE_ARR_NUM 8

/*定时器0 0.1ms*/
#define TIMER0_TH 0x9C
#define TIMER0_TL 0x9C
#define MAX_YEAR 2025
#define MIN_YEAR 2010
#define KEYBOARD P1
#define WLE P2        // 位选
#define DLE P3        // 段选

void led_disp_bit(unsigned int led_data_bit, unsigned int i);
void led_twinkle(unsigned int led_data[]);
void setTime();
void scan_key();
void time_inc(unsigned int type);
void init_timer0();
void time_ms_clr();
void time_ms_inc();

unsigned int press_key();
unsigned int read_key();
unsigned int is_leap_year(unsigned int year);

// 数码管显示数据表
unsigned char digital[DIGITAL_ARR_NUM] =
        {0x28, 0x7E, 0xA2, 0x62, 0x74, 0x61, 0x21, 0x7A, 0x20,
         0x60, 0x30, 0x25, 0xA9, 0x26, 0xA1, 0xB1, 0xFF, 0xF7, 0xDF};
unsigned int twinkle_bit[TWINKLE_ARR_NUM] = {0, 0, 0, 0, 0, 0, 0, 0};

enum key_states_e {
    KEY_STATE_RELEASE,
    KEY_STATE_WAITING,
    KEY_STATE_PRESSED
};
enum show_mode {
    TIME, DATE, COUNT
};
enum timer_mode {
    CLEAR, STOP, START
};
enum setting_mode {
    NONE, SECOND, MINUTE, HOUR, DAY, MONTH, YEAR
};

unsigned int sec = 50, minute = 59, hour = 23;
unsigned int day = 29, month = 02, year = 2016;
unsigned int count_msec = 0, count_sec = 0, count_minute = 0;
unsigned int disp_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned int show_stage = TIME;
unsigned int timer_stage = CLEAR;
unsigned int setting_stage = NONE;

int main(void) {
    TMOD = 0x22;
    EA = 1;
    init_timer0();
    setTime();
    while (1) {}
    return 0;
}

void interrupt0() {
    led_twinkle(disp_data);
    scan_key();
    return;
}

void init_timer0() {
    ET0 = 1;
    TR0 = 1;
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
}

void clr_twinkle(){
    unsigned int i;
    for (i = 0; i < TWINKLE_ARR_NUM; i++){
        twinkle_bit[i] = 0;
    }
    return;
}

void opr_key(unsigned int key_code) {
    switch (key_code) {
        case 0:{
            clr_twinkle();
            switch (show_stage) {
                case TIME:{
                    show_stage = DATE;
                    setting_stage = NONE;
                    break;
                }
                case DATE:{
                    show_stage = COUNT;
                    setting_stage = NONE;
                    break;
                }
                case COUNT: {
                    show_stage = TIME;
                    setting_stage = NONE;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case 1:{
            if (show_stage == COUNT) {
                clr_twinkle();
                switch (timer_stage) {
                    case START: {
                        timer_stage = STOP;
                        break;
                    }
                    case STOP:{
                        timer_stage = START;
                        break;
                    }
                    case CLEAR:{
                        timer_stage = START;
                        break;
                    }
                    default:
                        break;
                }
            }else {
                if(setting_stage != NONE) time_inc(setting_stage);
            }
            break;
        }
        case 2: {
            clr_twinkle();
            if (show_stage == TIME) {
                switch (setting_stage) {
                    case NONE: {
                        setting_stage = SECOND;
                        twinkle_bit[6] = 1;
                        twinkle_bit[7] = 1;
                        break;
                    }

                    case SECOND: {
                        setting_stage = MINUTE;
                        twinkle_bit[3] = 1;
                        twinkle_bit[4] = 1;
                        break;
                    }

                    case MINUTE: {
                        setting_stage = HOUR;
                        twinkle_bit[0] = 1;
                        twinkle_bit[1] = 1;
                        break;
                    }

                    case HOUR: {
                        setting_stage = NONE;
                        break;
                    }
                    default:
                        break;
                }
            } else if (show_stage == DATE) {
                switch (setting_stage) {
                    case NONE: {
                        setting_stage = DAY;
                        twinkle_bit[6] = 1;
                        twinkle_bit[7] = 1;
                        break;
                    }
                    case DAY: {
                        setting_stage = MONTH;
                        twinkle_bit[4] = 1;
                        twinkle_bit[5] = 1;
                        break;
                    }
                    case MONTH: {
                        setting_stage = YEAR;
                        twinkle_bit[0] = 1;
                        twinkle_bit[1] = 1;
                        twinkle_bit[2] = 1;
                        twinkle_bit[3] = 1;
                        break;
                    }
                    case YEAR: {
                        setting_stage = NONE;
                        break;
                    }
                    default:
                        break;
                }
            }else {
                if (timer_stage != START){
                    timer_stage = CLEAR;
                    time_ms_clr();
                }
            }
            break;
        }
        default:
            break;
    }
}

void scan_key() {
    static int key_state = KEY_STATE_RELEASE;   /*状态机状态初始化，采用static保存状态*/
    unsigned int wait_time = 500;   /*按键扫描等待时间*/
    static int key_code = -1;
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
            if (pressed == 0 && key_code >= 0) {
                opr_key(key_code);  /*opr_key为按键事件响应函数*/
                key_state = KEY_STATE_RELEASE;
                key_code = -1;
            }
            break;
        }
        default:
            break;
    }
}

//判断是否有键按下
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

//扫描键盘，返回键值
unsigned int read_key() {
    unsigned char temp_column, temp_row;
    unsigned int row, column, key_code;
    KEYBOARD = 0xF0;
    temp_column = KEYBOARD;

    KEYBOARD = 0x0F;
    temp_row = KEYBOARD;

    switch (temp_column) {
        case 0x70:{
            column = 0;
            break;
        }

        case 0xB0:{
            column = 1;
            break;
        }

        case 0xD0:{
            column = 2;
            break;
        }

        case 0xE0:{
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
        case 0x0B:{
            row = 1;
            break;
        }
        case 0x0D:{
            row = 2;
            break;
        }
        case 0x0E:{
            row = 3;
            break;
        }
        default:
            break;
    }
    key_code = 4 * row + column;
    return key_code;
}


void setTime() {
    switch (show_stage) {
        case TIME: {
            if (setting_stage == NONE){
                twinkle_bit[2] = 1;
                twinkle_bit[5] = 1;
            }
            disp_data[0] = hour / 10;
            disp_data[1] = hour % 10;
            disp_data[2] = 17;
            disp_data[3] = minute / 10;
            disp_data[4] = minute % 10;
            disp_data[5] = 17;
            disp_data[6] = sec / 10;
            disp_data[7] = sec % 10;
            break;
        }
        case DATE: {
            disp_data[0] = year / 1000;
            disp_data[1] = year % 1000 / 100;
            disp_data[2] = year % 1000 % 100 / 10;
            disp_data[3] = year % 1000 % 100 % 10;
            disp_data[4] = month / 10;
            disp_data[5] = month % 10;
            disp_data[6] = day / 10;
            disp_data[7] = day % 10;
            break;
        }
        case COUNT: {
            if (timer_stage == START){
                twinkle_bit[2] = 1;
                twinkle_bit[5] = 0;
            }
            disp_data[0] = count_minute / 10;
            disp_data[1] = count_minute % 10;
            disp_data[2] = 17;
            disp_data[3] = count_sec / 10;
            disp_data[4] = count_sec % 10;
            disp_data[5] = 18;
            disp_data[6] = count_msec / 10;
            disp_data[7] = count_msec % 10;
            break;
        }
        default:
            break;
    }
}

void led_disp_bit(unsigned int led_data_bit, unsigned int i) {
    //WLE = 0xFF;                // 消影
    //DLE = digital[16];
    WLE = _crol_(0xFE, i);    // 送数据
    DLE = digital[led_data_bit];
}

/**
 * @desc 数码管闪烁显示程序
 * @param led_data 需要显示的数据数组，从左到右显示
 * @param twinkle_bit 需要闪烁的数码管位，若某位数码管需要闪烁，则该位置1，否则为0
 * */
void led_twinkle(unsigned int led_data[]) {
    static unsigned int time = 0, flag = 0, i = 0;
    /*位选和段选熄灭，数码管消影*/
    WLE = 0xFF;
    DLE = digital[16];

    /*数码管某些位闪烁设置*/
    if (twinkle_bit[i] == 1){
        if (flag == 0) {
            /*led_disp_bit 为数码管单位显示函数*/
            led_disp_bit(led_data[i], i);
        } else {
            led_disp_bit(16, i);
        }
    }else{
        led_disp_bit(led_data[i], i);
    }

    i++;
    /*循环扫描数码管的8个数码位*/
    if (i >= 8) i = 0;
    time++;
    /*数码管某些位闪烁时间控制*/
    if (time >= 2500) {
        flag = ~flag;
        time = 0;
    }
    return;
}

/**
 * @desc 数字钟时间自增程序, 为数字钟时间调整开放接口
 * @param type 需要进行自增的量, 包含NONE、SECOND、MINUTE、HOUR、DAY、MONTH、YEAR
 * */
void time_inc(unsigned int type) {
    switch (type) {
        case NONE:{
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
            if (hour >= 24) {
                hour = 0;
                day++;
                if (month == 1 | month == 3 | month == 5 | month == 7 | month == 8 | month == 10 | month == 12) {
                    if (day > 31) {
                        day = 1;
                        month++;
                        if (month >= 13) {
                            month = 1;
                            year++;
                        }
                    }
                }
                if (month == 4 | month == 6 | month == 9 | month == 11) {
                    if (day > 30) {
                        day = 1;
                        month++;
                    }
                }
                if (month == 2) {
                    if (is_leap_year(year)) {
                        if (day >= 30){
                            day = 1;
                            month++;
                        }
                    } else {
                        if (day >= 29){
                            day = 1;
                            month++;
                        }
                    }
                }
            }
            /*时间进位逻辑结束*/
            break;
        }
        case SECOND: {
            sec++;
            if (sec >= 60) sec = 0;
            break;
        }
        case MINUTE: {
            minute++;
            if (minute >= 60) minute = 0;
            break;
        }
        case HOUR: {
            hour++;
            if (hour >= 24) hour = 0;
            break;
        }
        case DAY: {
            day++;
            if (month == 1 | month == 3 | month == 5 | month == 7 | month == 8 | month == 10 | month == 12) {
                if (day > 31) day = 1;
            }
            if (month == 4 | month == 6 | month == 9 | month == 11) {
                if (day > 30) day = 1;
            }
            if (month == 2) {
                if (is_leap_year(year)) {
                    if (day >= 30) day = 1;
                } else {
                    if (day >= 29) day = 1;
                }
            }
            break;
        }
        case MONTH: {
            month++;
            if (month >= 13) month = 1;
            if (month == 1 | month == 3 | month == 5 | month == 7 | month == 8 | month == 10 | month == 12) {
                if (day > 31) day = 1;
            }
            if (month == 4 | month == 6 | month == 9 | month == 11) {
                if (day > 30) day = 30;
            }
            if (month == 2) {
                if (is_leap_year(year)) {
                    if (day >= 30) day = 29;
                } else {
                    if (day >= 29) day = 28;
                }
            }
            break;
        }
        case YEAR: {
            year++;
            if (year > MAX_YEAR) year = MIN_YEAR;
            if (month == 2) {
                if (is_leap_year(year)) {
                    if (day >= 30) day = 29;
                } else {
                    if (day >= 29) day = 28;
                }
            }
            break;
        }
        default:
            break;
    }
    /*setTime 函数将内存中记忆的时间送入缓冲区，以便于在数码管上显示*/
    setTime();
    return;
}

void time_ms_clr() {
    count_msec = 0;
    count_sec = 0;
    count_minute = 0;
    setTime();
    return;
}

void time_ms_inc() {
    count_msec++;
    if (count_msec >= 100) {
        count_sec += 1;
        count_msec = 0;
    }
    if (count_sec >= 60) {
        count_minute += 1;
        count_sec = 0;
    }
    if (count_minute >= 60) {
        count_sec = 0;
        count_msec = 0;
        count_minute = 0;
    }
    setTime();
    return;
}

/**
 * @desc 闰年判断函数
 * @param year 需要进行判断的年份
 * @return 1:是闰年  0:不是闰年
 * */
unsigned int is_leap_year(unsigned int year) {
    unsigned int leap_year;
    if ((year + 2000) % 400 == 0) leap_year = 1;    //  被400整除为闰年
    else if ((year + 2000) % 100 == 0) leap_year = 0;  // 不能被400整除 能被100整除 不是闰年
    else if ((year + 2000) % 4 == 0) leap_year = 1;  // 不能被400、100整除 能被4整除 是闰年
    else leap_year = 0;
    return leap_year;
}

void int0() interrupt 1{
    static unsigned int time_num = 0, timer_num = 0;
    time_num++;
    timer_num++;
    if (timer_stage == START && timer_num >= 50) {
        timer_num = 0;
        time_ms_inc();
    }
    if (time_num >= 5000) {
        time_num = 0;
        if (setting_stage == NONE){
            time_inc(NONE);
        }
        //TH0 = TIMER0_TH;
        //TL0 = TIMER0_TL;
    }
    interrupt0();
}

/*void led_disp(unsigned int led_data[]){
    unsigned int i;
    for (i = 0; i < 8; i++) {
        P2 = 0xFF;				// 消影(必须有)
        P2 = _crol_(0xFE, i);	// 送数据
        P3 = digital[led_data[i]];		// 送数据
        delay(1);
    }
}*/
// 定义一个延时xms毫秒的延时函数,xms代表需要延时的毫秒数
/*void delay(unsigned int xms) {
    unsigned int x,y;
    for(x=xms;x>0;x--)
        for(y=11;y>0;y--);
}*/
