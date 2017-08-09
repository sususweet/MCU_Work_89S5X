#include<reg51.h> //ͷ�ļ�
#include<intrins.h>
#define uchar unsigned char //�궨�壬Ϊ������
#define uint unsigned int
sbit SDA=P3^3; //����������
sbit SCL=P3^2; //����ʱ����
uint value=0;
uchar digivalue[]={0x28,0x7e,0x0a2,0x62,0x74,0x61,0x21,0x7a,0x20,0x60};
//�������飬����Ϊ0-9
#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};
#define OP_WRITE 0xa0 // ������ַ�Լ�д�����
#define OP_READ 0xa1 // ������ַ�Լ���ȡ����
void start();
void stop();
uchar shin();
bit shout(uchar write_data);
void write_byte( uchar addr, uchar write_data);
void delayms(uint ms);
uchar read_current();
uchar read_random(uchar random_addr);
/**********************************************************/
void start() //I2C��������
//��ʼλ
{
    SDA = 1; //ʹ�� SDA
    SCL = 1;
    delayNOP();
    SDA = 0;
    delayNOP();
    SCL = 0;
}
/**********************************************************/
void stop() //I2Cֹͣ����
// ֹͣλ
{
    SDA = 0;
    delayNOP();
    SCL = 1;
    delayNOP();
    SDA = 1;
}
/**********************************************************/
uchar shin()
// ��AT24C02�Ƴ����ݵ�MCU
{
    uchar i,read_data;
    for(i = 0; i < 8; i++)
    {
        SCL = 1;
        read_data <<= 1; //��������һλ
        read_data |= SDA;
        SCL = 0;
    }
    return(read_data);
}
/**********************************************************/
bit shout(uchar write_data)
// ��MCU�Ƴ����ݵ�AT24C02
{
    uchar i;
    bit ack_bit;
    for(i = 0; i < 8; i++) // ѭ������8��λ
    {
        SDA = (bit)(write_data & 0x80);
        _nop_();
        SCL = 1;
        delayNOP();
        SCL = 0;
        write_data <<= 1;
    }
    SDA = 1; // ��ȡӦ��
    delayNOP();
    SCL = 1;
    delayNOP();
    ack_bit = SDA;
    SCL = 0;
    return ack_bit; // ����AT24C02Ӧ��λ
}
/**********************************************************/
void write_byte(uchar addr, uchar write_data)
// ��ָ����ַaddr��д������write_data
{
    start();
    shout(OP_WRITE);
    shout(addr);
    shout(write_data);
    stop();
    delayms(10); // д������
}
/**********************************************************/
uchar read_current()
// �ڵ�ǰ��ַ��ȡ
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
// ��ָ����ַ��ȡ
{
    start();
    shout(OP_WRITE);
    shout(random_addr);
    return(read_current());
}
/**********************************************************/
void delayms(uint ms)
// ��ʱ�ӳ���
{
    uchar k;
    while(ms--)
    {
        for(k = 0; k < 120; k++);
    }
}
/**********************************************************/
void delay() //��λ����������
{
    uchar ii=0,jj=0,kk=0;
    for(ii=0;ii<200;ii++)
        for(jj=0;jj<200;jj++);
}

void main() {
    SCL = 0;
    //delay();
    write_byte(3,222);
    value=read_random(55); //��ȡ��Ƭ����λ����
    value=value+1; //�����Ĵ�����һ
    if(value>9) value=0;
    write_byte(0,value);
    value; //��ʾ����
    while(1);
}