#include<reg51.h> //头文件
#include<intrins.h>
#define uchar unsigned char //宏定义，为方便编程
#define uint unsigned int
sbit SDA=P3^3; //定义数据线
sbit SCL=P3^2; //定义时钟线
uint value=0;
uchar digivalue[]={0x28,0x7e,0x0a2,0x62,0x74,0x61,0x21,0x7a,0x20,0x60};
//数字数组，依次为0-9
#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};
#define OP_WRITE 0xa0 // 器件地址以及写入操作
#define OP_READ 0xa1 // 器件地址以及读取操作
void start();
void stop();
uchar shin();
bit shout(uchar write_data);
void write_byte( uchar addr, uchar write_data);
void delayms(uint ms);
uchar read_current();
uchar read_random(uchar random_addr);
/**********************************************************/
void start() //I2C启动函数
//开始位
{
    SDA = 1; //使能 SDA
    SCL = 1;
    delayNOP();
    SDA = 0;
    delayNOP();
    SCL = 0;
}
/**********************************************************/
void stop() //I2C停止函数
// 停止位
{
    SDA = 0;
    delayNOP();
    SCL = 1;
    delayNOP();
    SDA = 1;
}
/**********************************************************/
uchar shin()
// 从AT24C02移出数据到MCU
{
    uchar i,read_data;
    for(i = 0; i < 8; i++)
    {
        SCL = 1;
        read_data <<= 1; //数据左移一位
        read_data |= SDA;
        SCL = 0;
    }
    return(read_data);
}
/**********************************************************/
bit shout(uchar write_data)
// 从MCU移出数据到AT24C02
{
    uchar i;
    bit ack_bit;
    for(i = 0; i < 8; i++) // 循环移入8个位
    {
        SDA = (bit)(write_data & 0x80);
        _nop_();
        SCL = 1;
        delayNOP();
        SCL = 0;
        write_data <<= 1;
    }
    SDA = 1; // 读取应答
    delayNOP();
    SCL = 1;
    delayNOP();
    ack_bit = SDA;
    SCL = 0;
    return ack_bit; // 返回AT24C02应答位
}
/**********************************************************/
void write_byte(uchar addr, uchar write_data)
// 在指定地址addr处写入数据write_data
{
    start();
    shout(OP_WRITE);
    shout(addr);
    shout(write_data);
    stop();
    delayms(10); // 写入周期
}
/**********************************************************/
uchar read_current()
// 在当前地址读取
{
    uchar read_data;
    start();
    shout(OP_READ);
    read_data = shin();
    stop();
    return read_data;
}
/**********************************************************/
uchar read_random(uchar random_addr)
// 在指定地址读取
{
    start();
    shout(OP_WRITE);
    shout(random_addr);
    return(read_current());
}
/**********************************************************/
void delayms(uint ms)
// 延时子程序
{
    uchar k;
    while(ms--)
    {
        for(k = 0; k < 120; k++);
    }
}
/**********************************************************/
void delay() //复位消抖动函数
{
    uchar ii=0,jj=0,kk=0;
    for(ii=0;ii<200;ii++)
        for(jj=0;jj<200;jj++);
}

void main() {
    SCL = 0;
    //delay();
    write_byte(3,222);
    value=read_random(55); //读取单片机复位次数
    value=value+1; //读到的次数加一
    if(value>9) value=0;
    write_byte(0,value);
    value; //显示次数
    while(1);
}
