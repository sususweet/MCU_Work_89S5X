#include "REG51.H"
#include "INTRINS.H"

//定义ADC的连接端口
#define AD_INPUT P1
sbit AD_WR=P3^6;
sbit AD_RD=P3^7;
void delay_us(unsigned int us);
void delay_ms(unsigned int ms);

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
    output = AD_INPUT;
    _nop_();
    AD_RD = 1;
    _nop_();
    AD_WR = 1;
    return output;
}


int main(void){
    startADC();


    while (1){
          delay_ms(1);
          P0 = readADC();
    }
    return 0;
}

void delay_us(unsigned int us){
    do {
        _nop_();
        us--;
    } while(us);
}

/* 定义一个延时xms毫秒的延时函数,xms代表需要延时的毫秒数 */
void delay_ms(unsigned int ms) {
    do {
        delay_us(1000);
        ms--;
    } while(ms);
}
