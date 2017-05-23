#include "REG51.H"
#include "INTRINS.H"

void send_char(unsigned char txd);

unsigned int data_length = 0;

enum receive_states {
    RECEIVE_STATE_NONE,
    RECEIVE_STATE_LENGTH,
    RECEIVE_STATE_DATA,
    RECEIVE_STATE_CHECK
};
unsigned int receive_stage = RECEIVE_STATE_NONE;
unsigned int result[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void clearData() {
    int i = 0;
    for (i = 0; i < 8; i++) result[i] = 0;
    return;
}

void operateData(unsigned char tmp) {
    static unsigned int count = 0;
    switch (receive_stage) {
        case RECEIVE_STATE_NONE: {
            clearData();
            count = 0;
            if (tmp == 0xAA) {
                receive_stage = RECEIVE_STATE_LENGTH;
            }
            break;
        }
        case RECEIVE_STATE_LENGTH: {
            data_length = tmp;
            receive_stage = RECEIVE_STATE_DATA;
            break;
        }
        case RECEIVE_STATE_DATA: {
            result[count] = tmp;
            count++;
            if (count >= data_length) receive_stage = RECEIVE_STATE_CHECK;
            break;
        }
        case RECEIVE_STATE_CHECK: {
            int i, sum = 0;
            for (i = 0; i < 8; i++) sum += result[i];//将每个数相加
            if (sum > 0xff) {
                sum = ~sum;
                sum += 1;
            }
            sum = sum & 0xff;
            if (sum == tmp) {
                P0 = result[0];
                send_char(0xBB);
                send_char((unsigned char) data_length);
                send_char((unsigned char) sum);
            }
            receive_stage = RECEIVE_STATE_NONE;
            break;
        }
        default: {
            receive_stage = RECEIVE_STATE_NONE;
            break;
        }
    }
}

int main(void) {
    unsigned char tmp;
    TMOD = 0x20;
    TH1 = 0xF3;
    TL1 = 0xF3;
    SCON = 0x50;
    PCON &= 0xEF;
    TR1 = 1;
    IE = 0x0;
    while (1) {
        if (RI) {
            RI = 0;
            tmp = SBUF;
            send_char(tmp);
            operateData(tmp);
            //P0 = tmp;
        }
    }
    return 0;
}


void send_char(unsigned char txd) {
    SBUF = txd;
    while (!TI);
    TI = 0;
}
