#include <reg51.h>
#include <intrins.h>

/*char code SST516[3] _at_ 0x003b;*/

#define DIGITAL_ARR_NUM 18
/*定时器0 0.1ms*/
#define TIMER0_TH 0x9C
#define TIMER0_TL 0x9C
#define KEYBOARD P1
#define WLE P2        // 位选
#define DLE P3        // 段选

void led_disp_bit(unsigned int led_data_bit, unsigned int i);

void led_twinkle(unsigned int led_data[]);

void setTime();
void scan_key();
void time_inc();

unsigned int press_key();
unsigned int read_key();

unsigned int is_leap_year(unsigned int year);

// 数码管显示数据表
unsigned char digital[DIGITAL_ARR_NUM] =
        {0x28, 0x7E, 0xA2, 0x62, 0x74, 0x61, 0x21, 0x7A, 0x20,
         0x60, 0x30, 0x25, 0xA9, 0x26, 0xA1, 0xB1, 0xFF, 0xF7};
unsigned int key_map[4][4] =
        {{0, 1, 2, 3},{4, 5, 6, 7},{8, 9, 10, 11},{12, 13, 14, 15}};
enum key_states_e{
    KEY_STATE_RELEASE,
    KEY_STATE_WAITING,
    KEY_STATE_PRESSED
};
enum show_mode{
    TIME,
    DATE,
    COUNT
};

unsigned int sec = 0, minute = 0, hour = 0;
unsigned int day = 1, month = 1, year = 2017;
unsigned int disp_data[8] = {2, 0, 17, 0, 0, 17, 0, 0};
unsigned int timer_num = 0;
unsigned int show = TIME;

int main(void) {
    TMOD = 0x10;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
    while (1) {}
    return 0;
}

void interrupt0() {
    led_twinkle(disp_data);
    scan_key();
}

void opr_key(unsigned int key_code){
    switch(key_code){
        case 1:
            show = DATE;
            break;
        case 2:
            break;
        default:
            break;
    }
}

void scan_key(){
    static key_state = KEY_STATE_RELEASE;
    unsigned int wait_time = 100;
    static scan_time = 0;
    switch(key_state) {
        case KEY_STATE_RELEASE:{
            if (press_key() == 1){
                key_state = KEY_STATE_WAITING;
            }
            break;
        }
        case KEY_STATE_WAITING:{
            if (press_key() == 1){
                scan_time++;
                if (scan_time >= wait_time){
                    key_state = KEY_STATE_PRESSED;
                    scan_time = 0;
                }
            }else {
                scan_time = 0;
                key_state = KEY_STATE_RELEASE;
            }
            break;
        }
        case KEY_STATE_PRESSED:{
            if (press_key() == 0){
                opr_key(read_key());
                key_state = KEY_STATE_RELEASE;
            }
            break;
        }
    }
}
//判断是否有键按下
unsigned int press_key(){
    unsigned char temp_line,temp_row;
    KEYBOARD = 0xF0;
    temp_line = KEYBOARD;
    if (temp_line == 0xF0) return 0;
    KEYBOARD = 0x0F;
    temp_row = KEYBOARD;
    if (temp_line == 0x0F) return 0;
    return 1;
}

//扫描键盘，返回键值
unsigned int read_key(){
    unsigned char temp_line,temp_row,key_code;
    unsigned int row,line;
    KEYBOARD = 0xF0;
    temp_line = KEYBOARD;

    KEYBOARD = 0x0F;
    temp_row = KEYBOARD;

    switch (temp_line){
        case 0x70: line = 1; break;
        case 0xB0: line = 2; break;
        case 0xD0: line = 3; break;
        case 0xE0: line = 4; break;
    }

    switch (temp_row){
        case 0x07: row = 1; break;
        case 0x0B: row = 2; break;
        case 0x0D: row = 3; break;
        case 0x0E: row = 4; break;
    }
    key_code = key_map[row][line];
    return key_code;
}


void setTime() {
    switch (show){
        case TIME:{
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
        case DATE:{
            disp_data[0] = year / 1000;
            disp_data[1] = year % 1000 /100;
            disp_data[2] = year % 1000 % 100 / 10;
            disp_data[3] = year % 1000 % 100 % 10;
            disp_data[4] = month / 10;
            disp_data[5] = month % 10;
            disp_data[6] = day / 10;
            disp_data[7] = day % 10;
        }
    }
}

void led_disp_bit(unsigned int led_data_bit, unsigned int i) {
    WLE = 0xFF;                // 消影
    DLE = digital[16];
    WLE = _crol_(0xFE, i);    // 送数据
    DLE = digital[led_data_bit];
}

/*数码管闪烁显示程序*/
void led_twinkle(unsigned int led_data[]) {
    static unsigned int time = 0, flag = 0, i = 0;
    if (led_data[i] != 17) {
        led_disp_bit(led_data[i], i);
        //P3 = digital[led_data[i]];		// 送数据
    } else {
        if (flag == 0) {
            led_disp_bit(17, i);
            //   P3 = digital[17];		// 送数据
        } else {
            led_disp_bit(16, i);
            // P3 = digital[16];		// 送数据
        }
    }
    i++;
    if (i >= 8) i = 0;
    time++;
    if (time >= 100) {
        flag = ~flag;
        time = 0;
    }
}

void time_inc() {
    sec++;
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
        if(month == 1|month == 3|month==5|month==7|month==8|month==10|month==12) {
            day++;
            if(day>31){
                day= 1 ;month++;
                if(month==13){
                    month=1;year++;
                }
            }
        }
        if(month==4|month==6|month==9|month==11) {
            day++;
            if(day>30){
                day=1;month++;
            }
        }
        if(month==2) {
            day++;
            if(is_leap_year(year)){
                if(day>=30)day=1;
            }
            else {
                if(day>=29)day=1;
            }
        }
    }

    return;
}
unsigned int is_leap_year(unsigned int year){
    unsigned int leap_year;
    if((year+2000)%400==0) leap_year=1;    //  被400整除为闰年
    else if((year+2000)%100==0) leap_year=0;  // 不能被400整除 能被100整除 不是闰年
    else if((year+2000)%4==0) leap_year=1;  // 不能被400、100整除 能被4整除 是闰年
    else leap_year=0;
    return leap_year;
}
void int0() interrupt 1{
    timer_num++;
    setTime();
    interrupt0();

    if (timer_num >= 10000){
        timer_num = 0;
        time_inc();
        TH0 = TIMER0_TH;
        TL0 = TIMER0_TL;
    }
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
