#include "REG51.H"
#include "INTRINS.H"

/*��ʱ��0 2ms*/
#define TIMER0_TH 0xF8
#define TIMER0_TL 0x30
#define KEY_WAIT 10	/*����ɨ���ӳ�����*/
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
                opr_key(key_code);  /*opr_keyΪ�����¼���Ӧ����*/
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