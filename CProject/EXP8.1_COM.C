#include "REG51.H"
#include "INTRINS.H"

void send_char(unsigned char txd);
unsigned char tmp;

int main(void){
    /*波特率2400*/
    TMOD = 0x20;
    TH1 = 0xF3;
    TL1 = 0xF3;
    SCON = 0x50;
    PCON &= 0x7F;
    TR1 = 1;
    IE = 0x0;
    while (1){
        if (RI){
            RI = 0;
            tmp = SBUF;
            P0 = tmp;
            send_char(tmp);
        }
    }
    return 0;
}

/*串行口发送字符*/
void send_char(unsigned char txd){
    SBUF = txd;
    while(!TI);
    TI = 0;
}
