#include "REG51.H"
#include "INTRINS.H"

//定义ADC的连接端口
#define AD_INPUT P1
sbit AD_WR=P3^6;
sbit AD_RD=P3^7;
void delay_us(unsigned int us);
void delay_ms(unsigned int ms);
void initSend();

/*开始AD转换*/
void startADC(void){
    AD_WR = 1;
    _nop_();
    _nop_();
    AD_WR = 0;
    _nop_();
    _nop_();
    AD_WR = 1;
}

/*读取AD转换数据*/
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
    unsigned char tmp;
    startADC();
    initSend();

    delay_ms(1);
    tmp = readADC();
    P0 = tmp;
    SBUF = tmp;
    while (1){

    }
    return 0;
}

/*串口中断初始化，定时器1初始化为波特率发生器，波特率为2400*/
void initSend(){
    TMOD = 0x20;
    TH1 = 0xF3;
    TL1 = 0xF3;
    SCON = 0x50;
    PCON &= 0x7F;
    TR1 = 1;
    EA = 1;
    ES = 1;
}

/*串行口发送字符*/
void sInterrupt(){
    unsigned char tmp;
    TI = 0;
    tmp = readADC();
    P0 = tmp;
    SBUF = tmp;
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

void int0() interrupt 4{
    sInterrupt();
}
