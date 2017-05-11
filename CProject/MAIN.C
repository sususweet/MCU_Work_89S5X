#include <reg51.h>
#include <intrins.h>
/*#include "REG51.H"
#include "INTRINS.H"  */

/*char code SST516[3] _at_ 0x003b;*/

#define DIGITAL_ARR_NUM 18
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
/*void init_timer1();*/

unsigned int press_key();
unsigned int read_key();
unsigned int is_leap_year(unsigned int year);

// 数码管显示数据表
unsigned char digital[DIGITAL_ARR_NUM] =
        {0x28, 0x7E, 0xA2, 0x62, 0x74, 0x61, 0x21, 0x7A, 0x20,
         0x60, 0x30, 0x25, 0xA9, 0x26, 0xA1, 0xB1, 0xFF, 0xF7};
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

unsigned int sec = 0, minute = 7, hour = 16;
unsigned int day = 11, month = 5, year = 2017;
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
/*void init_timer1(){
    ET1 = 1;
    TR1 = 1;
    TH1 = TIMER1_TH;
    TL1 = TIMER1_TL;
}*/

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
                time_inc(setting_stage);
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
    static int key_state = KEY_STATE_RELEASE;
    unsigned int wait_time = 500;
    static int key_code = -1;
    unsigned int pressed = press_key();
    static scan_time = 0;
    switch (key_state) {
        case KEY_STATE_RELEASE: {
            if (pressed == 1) {
                key_state = KEY_STATE_WAITING;
            }
            break;
        }
        case KEY_STATE_WAITING: {
            if (pressed) {
                scan_time++;
                if (scan_time >= wait_time) {
                    key_state = KEY_STATE_PRESSED;
                    scan_time = 0;
                    key_code = read_key();
                }
            } else {
                scan_time = 0;
                key_state = KEY_STATE_RELEASE;
            }
            break;
        }
        case KEY_STATE_PRESSED: {

            if (pressed == 0 && key_code >= 0) {
                opr_key(key_code);
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
                twinkle_bit[5] = 1;
            }
            disp_data[0] = count_minute / 10;
            disp_data[1] = count_minute % 10;
            disp_data[2] = 17;
            disp_data[3] = count_sec / 10;
            disp_data[4] = count_sec % 10;
            disp_data[5] = 17;
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

/*数码管闪烁显示程序*/
void led_twinkle(unsigned int led_data[]) {
    static unsigned int time = 0, flag = 0, i = 0;
    WLE = 0xFF;
    DLE = digital[16];

    if (twinkle_bit[i] == 1){
        if (flag == 0) {
            led_disp_bit(led_data[i], i);
        } else {
            led_disp_bit(16, i);
        }
    }else{
        led_disp_bit(led_data[i], i);
    }

    /*if (led_data[i] != 17) {
        led_disp_bit(led_data[i], i);
    } else {

    }*/


    i++;
    if (i >= 8) i = 0;
    time++;
    if (time >= 2500) {
        flag = ~flag;
        time = 0;
    }
    return;
}

void time_inc(unsigned int type) {
    switch (type) {
        case SECOND: {
            sec++;
            break;
        }
        case MINUTE: {
            minute++;
            break;
        }
        case HOUR: {
            hour++;
            break;
        }
        case DAY: {
            day++;
            break;
        }
        case MONTH: {
            month++;
            break;
        }
        case YEAR: {
            year++;
            if (year >= MAX_YEAR) {
                year = MIN_YEAR;
            }
            break;
        }
        default:
            break;
    }
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
        if (month == 1 | month == 3 | month == 5 | month == 7 | month == 8 | month == 10 | month == 12) {
            day++;
            if (day > 31) {
                day = 1;
                month++;
                if (month == 13) {
                    month = 1;
                    year++;
                }
            }
        }
        if (month == 4 | month == 6 | month == 9 | month == 11) {
            day++;
            if (day > 30) {
                day = 1;
                month++;
            }
        }
        if (month == 2) {
            day++;
            if (is_leap_year(year)) {
                if (day >= 30)day = 1;
            } else {
                if (day >= 29)day = 1;
            }
        }
    }
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
            time_inc(SECOND);
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
