#include "REG51.H"
#include "INTRINS.H"

/*定时器0 2ms*/
#define TIMER0_TH 0xF8
#define TIMER0_TL 0x30
#define KEY_WAIT 10	/*键盘扫描延迟周期*/
#define KEYBOARD P1

enum key_states_e {
    KEY_STATE_RELEASE,
    KEY_STATE_WAITING,
    KEY_STATE_PRESSED
};

void send_char(unsigned char txd);
void scan_key();
unsigned int press_key();
unsigned int read_key();

unsigned char tmp;

void opr_key(unsigned int key_code) {
    send_char((unsigned char) key_code);
}

void scan_key() {
    static int key_state = KEY_STATE_RELEASE;   /*状态机状态初始化，采用static保存状态*/
    unsigned int wait_time = KEY_WAIT;   /*按键扫描等待时间*/
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



int main(void){
    TMOD = 0x21;
    EA = 1;
    ET0 = 1;
    TR0 = 1;
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;


    TH1 = 0xF3;
    TL1 = 0xF3;
    SCON = 0x50;
    PCON &= 0xEF;
    TR1 = 1;
    ES = 0;
    ET1 = 0;
    while (1){

    }
    return 0;
}

void send_char(unsigned char txd){
    SBUF = txd;
    while(!TI);
    TI = 0;
}

void interrupt0(){
    scan_key();
    return;
}

void int0() interrupt 1{
    interrupt0();
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
}