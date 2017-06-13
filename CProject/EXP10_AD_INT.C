#include "REG51.H"
#include "INTRINS.H"

/*定时器0 2ms*/
#define TIMER0_TH 0xF8
#define TIMER0_TL 0x30

//时钟计时周期
#define TIME_FREQ 500

//定义ADC的连接端口
#define AD_INPUT P1
sbit AD_WR=P3^6;
sbit AD_RD=P3^7;
void delay_us(unsigned int us);
void delay_ms(unsigned int ms);
void send_char(unsigned char txd);
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
    initSend();
    startADC();

    while (1){

    }

}

/*定时器0初始化，定时器1初始化为波特率发生器，波特率为2400*/
void initSend(){
    TMOD = 0x21;
    EA = 1;
    ET0 = 1;
    TR0 = 1;
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;

    ES = 0;
    ET1 = 0;
    TR1 = 1;
    TH1 = 0xF3;
    TL1 = 0xF3;
    SCON = 0x50;
    PCON &= 0x7F;
}

/*串行口发送字符*/
void send_char(unsigned char txd) {
    SBUF = txd;
    while (!TI);
    TI = 0;
}

void interrupt0(){
    unsigned char tmp;
    tmp = readADC();
    P0 = tmp;
    send_char(tmp);
    return;
}

void int0() interrupt 1{
    static unsigned int time_num = 0;
    time_num++;

    if (time_num >= TIME_FREQ) {
        time_num = 0;
        interrupt0();
    }
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
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
