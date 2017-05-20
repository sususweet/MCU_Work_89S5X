#include <reg51.h>
#include <intrins.h>
#define uint unsigned int
#define uchar unsigned char
#define noACK 0
#define ACK 1
#define STATUS_REG_W 0x06
#define STATUS_REG_R 0x07
#define MEASURE_TEMP 0x03
#define MEASURE_HUMI 0x05
#define RESET 0x1e
enum {TEMP,HUMI};
typedef union //���干��ͬ����
{
unsigned int i;
float f;
} value;
sbit lcdrs=P2^0;
sbit lcdrw=P2^1;
sbit lcden=P2^2;
sbit SCK = P1^0;
sbit DATA = P1^1;
uchar table2[]="΢��ԭ����Ӧ��";
uchar table3[]="�¶�Ϊ:     ";
uchar table4[]="ʪ��Ϊ:    ";
uchar table5[]="SHT11 ��ʪ�ȼ��";
uchar wendu[6];
uchar shidu[6];
void delay(int z)
{
int x,y;
for(x=z;x>0;x--)
for(y=125;y>0;y--);
}
void delay_50us(uint t)
{
uint j;
for(;t>0;t--)
for(j=19;j>0;j--);
}
void delay_50ms(uint t)
{
uint j;
for(;t>0;t--)
for(j=6245;j>0;j--);
}
/*
void displaywendu(void)
{
uchar i;
write_12864com(0x94);
for(i=0;i<3;i++)
{
write_dat(wendu[i]);
delay_50us(1);
}
for(i=0;i<1;i++)
{
write_dat(table5[i]);
delay_50us(1);
}
for(i=4;i<5;i++)
{
write_dat(wendu[i]);
delay_50us(1);
}
}
void displayshidu(void)
{
uchar i;
write_12864com(0x8C);
for(i=0;i<3;i++)
{
write_dat(shidu[i]);
delay_50us(1);
}
for(i=0;i<1;i++)
{
write_dat(table5[i]);
delay_50us(1);
}
for(i=4;i<5;i++)
{
write_dat(shidu[i]);
delay_50us(1);
}
}
//д�ֽڳ���
char s_write_byte(unsigned char value)
{
unsigned char i,error=0;
for (i=0x80;i>0;i>>=1) //��λΪ1��ѭ������
{
if (i&value) DATA=1; //��Ҫ���͵������룬���Ϊ���͵�λ
 else DATA=0;
 SCK=1;
 _nop_();_nop_();_nop_(); //��ʱ3us
 SCK=0;
}
DATA=1; //�ͷ�������
SCK=1;
error=DATA; //���Ӧ���źţ�ȷ��ͨѶ����
_nop_();_nop_();_nop_();
SCK=0;
DATA=1;
return error; //error=1 ͨѶ����
}
//���ֽڳ���
char s_read_byte(unsigned char ack)
{
unsigned char i,val=0;
DATA=1; //�ͷ�������
for(i=0x80;i>0;i>>=1) //��λΪ1��ѭ������
{
SCK=1;
 if(DATA) val=(val|i); //��һλ�����ߵ�ֵ
 SCK=0;
}
DATA=!ack; //�����У�飬��ȡ������ͨѶ��
SCK=1;
_nop_();_nop_();_nop_(); //��ʱ3us
SCK=0;
_nop_();_nop_();_nop_();
DATA=1; //�ͷ�������
return val;
}
//��������
void s_transstart(void)
{
 DATA=1; SCK=0; //׼��
 _nop_();
 SCK=1;
 _nop_();
 DATA=0;
 _nop_();
 SCK=0;
 _nop_();_nop_();_nop_();
 SCK=1;
 _nop_();
 DATA=1;
 _nop_();
 SCK=0;
}
//���Ӹ�λ
void s_connectionreset(void)
{
unsigned char i;
DATA=1; SCK=0; //׼��
for(i=0;i<9;i++) //DATA���ָߣ�SCKʱ�Ӵ���9�Σ������������䣬ͨѸ����λ
{
SCK=1;
 SCK=0;
}
s_transstart(); //��������
}*/
/*
//����λ����
char s_softreset(void)
{
unsigned char error=0;
s_connectionreset(); //�������Ӹ�λ
error+=s_write_byte(RESET); //���͸�λ����
return error; //error=1 ͨѶ����
}
//��ʪ�Ȳ���
char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
{
unsigned error=0;
unsigned int i;
s_transstart(); //��������
switch(mode) //ѡ��������
 {
case TEMP : error+=s_write_byte(MEASURE_TEMP); break; //�����¶�
 case HUMI : error+=s_write_byte(MEASURE_HUMI); break; //����ʪ��
 default : break;
}
for (i=0;i<65535;i++) if(DATA==0) break; //�ȴ���������
if(DATA) error+=1; // �����ʱ��������û�����ͣ�˵����������
*(p_value) =s_read_byte(ACK); //����һ���ֽڣ����ֽ� (MSB)
*(p_value+1)=s_read_byte(ACK); //���ڶ����ֽڣ����ֽ� (LSB)
*p_checksum =s_read_byte(noACK); //read CRCУ����
return error; // error=1 ͨѶ����
}
//��ʪ��ֵ��ȱ任���¶Ȳ���
void calc_sth10(float *p_humidity ,float *p_temperature)
{
const float C1=-4.0; // 12λʪ�Ⱦ��� ������ʽ
const float C2=+0.0405; // 12λʪ�Ⱦ���������ʽ
const float C3=-0.0000028; // 12λʪ�Ⱦ���������ʽ
const float T1=+0.01; // 14λ�¶Ⱦ��� 5V���� ������ʽ
const float T2=+0.00008; // 14λ�¶Ⱦ��� 5V���� ������ʽ
float rh=*p_humidity; // rh: 12λ ʪ��
float t=*p_temperature; // t: 14λ�¶�
float rh_lin; // rh_lin: ʪ�� linearֵ
float rh_true; // rh_true: ʪ�� tureֵ
float t_C; // t_C : �¶� ��
t_C=t*0.01 - 40; //�����¶�
rh_lin=C3*rh*rh + C2*rh + C1; //���ʪ�ȷ����Բ���
rh_true=(t_C-25)*(T1+T2*rh)+rh_lin; //���ʪ�ȶ����¶������Բ���
if(rh_true>100)rh_true=100; //ʪ���������
if(rh_true<0.1)rh_true=0.1; //ʪ����С����
*p_temperature=t_C; //�����¶Ƚ��
*p_humidity=rh_true; //����ʪ�Ƚ��
}

*/





void write_12864com(uchar com) {
    lcdrs=0;
    lcdrw=0;
    delay_50us(1);
    P0=com;
    lcden=1;
    delay_50us(2);
    lcden=0;
    delay_50us(2);
}
void write_dat(uchar dat) {
    lcdrs=1;
    lcdrw=0;
    delay_50us(1);
    P0=dat;
    lcden=1;
    delay_50us(2);
    lcden=0;
    delay_50us(2);
}

void display1(void) {
    uchar i;
    write_12864com(0x80);
    for(i=0;i<16;i++) {
        write_dat(table2[i]);
        delay_50us(1);

    }
}
void display2(void) {
    uchar i;
    write_12864com(0x90);
    for(i=0;i<18;i++) {
        write_dat(table3[i]);
        delay_50us(1);
    }
}
void display3(void)
{
    uchar i;
    write_12864com(0x88);
    for(i=0;i<8;i++)
    {
        write_dat(table4[i]);
        delay_50us(1);
    }
}
void display4(void) {
    uchar i;
    write_12864com(0x88);
    for(i=0;i<8;i++)
    {
        write_dat(table5[i]);
        delay_50us(1);
    }
}


void init12864lcd(void) {
    delay_50ms(2);
    write_12864com(0x30);
    delay_50us(4);
    write_12864com(0x30);
    delay_50us(4);
    write_12864com(0x0f);
    delay_50us(4);
    write_12864com(0x01);
    delay_50us(1);
    write_12864com(0x06);
    delay_50us(10);
    write_12864com(0x0c);
    delay_50us(10);
}

void main(void) {
    unsigned int temp,humi;
    value humi_val,temp_val; //����������ͬ�壬һ������ʪ�ȣ�һ�������¶�
    unsigned char error; //���ڼ����Ƿ���ִ���
    unsigned char checksum; //CRC
    init12864lcd();display1();
   /*
    display2();
    display3();
    display4();*/
    //s_connectionreset(); //�������Ӹ�λ
    /*while(1)
    {
    error=0; //��ʼ��error=0����û�д���
    error+=s_measure((unsigned char*)&temp_val.i,&checksum,TEMP); //�¶Ȳ���
    error+=s_measure((unsigned char*)&humi_val.i,&checksum,HUMI); //ʪ�Ȳ���
     if(error!=0) s_connectionreset(); ////�����������ϵͳ��λ
     else
     {
    humi_val.f=(float)humi_val.i; //ת��Ϊ������
     temp_val.f=(float)temp_val.i; //ת��Ϊ������
     calc_sth10(&humi_val.f,&temp_val.f); //�������ʪ�ȼ��¶�
     temp=temp_val.f*10;
     humi=humi_val.f*10;
     wendu[0]=temp/1000+0; //�¶Ȱ�λ
     wendu[1]=temp/100+0; //�¶�ʮλ
     wendu[2]=temp/10+0; //�¶ȸ�λ
     wendu[3]=0x2E; //С����
     wendu[4]=temp+0; //�¶�С������һλ
    displaywendu();
    shidu[0]=humi/1000+0; //ʪ�Ȱ�λ
     shidu[1]=humi/100+0; //ʪ��ʮλ
     shidu[2]=humi/10+0; //ʪ�ȸ�λ
     shidu[3]=0x2E; //С����
     shidu[4]=humi+0; //ʪ��С������һλ
    displayshidu();
     }
    delay(800); //�ȴ��㹻����ʱ�䣬��������һ��ת��
    }*/
}