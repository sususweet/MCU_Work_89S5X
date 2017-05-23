#include "REG51.H"
#include "INTRINS.H"

void send_char(unsigned char txd);
unsigned char tmp;

int main(void){
    TMOD = 0x20;
    TH1 = 0xF3;
    TL1 = 0xF3;
    SCON = 0x50;
    PCON &= 0xEF;
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

void send_char(unsigned char txd){
    SBUF = txd;
    while(!TI);
    TI = 0;
}
