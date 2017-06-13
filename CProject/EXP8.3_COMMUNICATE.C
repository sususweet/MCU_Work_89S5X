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

/*清除数据，等待下一次接收*/
void clearData() {
    int i = 0;
    for (i = 0; i < 8; i++) result[i] = 0;
    return;
}

/*采用状态机的方法处理接收到的数据*/
void operateData(unsigned char tmp) {
    static unsigned int count = 0;
    switch (receive_stage) {
        /*接收数据初始状态*/
        case RECEIVE_STATE_NONE: {
            /*接收数据缓冲区初始化，以实现单片机串口通信重复接收的功能*/
            clearData();
            count = 0;
            /*发现数据头，进行状态跳转*/
            if (tmp == 0xAA) {
                receive_stage = RECEIVE_STATE_LENGTH;
            }
            break;
        }
        /*发现数据头标志后获取数据的长度*/
        case RECEIVE_STATE_LENGTH: {
            data_length = tmp;
            receive_stage = RECEIVE_STATE_DATA;
            break;
        }
        /*获取数据长度后正式开始数据接收*/
        case RECEIVE_STATE_DATA: {
            result[count] = tmp;
            count++;
            if (count >= data_length) receive_stage = RECEIVE_STATE_CHECK;
            break;
        }
        /*数据接收完毕，进行和校验*/
        case RECEIVE_STATE_CHECK: {
            int i, sum = 0;
            for (i = 0; i < 8; i++) sum += result[i];//将每个数相加
            if (sum > 0xff) {
                sum = ~sum;
                sum += 1;
            }
            sum = sum & 0xff;
            if (sum == tmp) {   /*和校验通过，发送反馈数据*/
                P0 = result[0]; /*LED显示接收到的第一位数据*/
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

/*定时器0初始化，定时器1初始化为波特率发生器，波特率为2400*/
int main(void) {
    unsigned char tmp;
    TMOD = 0x20;
    TH1 = 0xF3;
    TL1 = 0xF3;
    SCON = 0x50;
    PCON &= 0x7F;
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

/*串行口发送字符*/
void send_char(unsigned char txd) {
    SBUF = txd;
    while (!TI);
    TI = 0;
}
