#include "INTRINS.H"
#include "REG51.H"

#define uint unsigned int
#define uchar unsigned char
#define DATA P0
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
sbit RS_LCD=P2^0;
sbit RW_LCD=P2^1;
sbit EN_LCD=P2^2;
sbit SCK = P1^0;
//sbit DATA = P1^1;
uchar table2[]="΢��ԭ����Ӧ��";
uchar table3[]="�¶�Ϊ:     ";
uchar table4[]="ʪ��Ϊ:    ";
uchar table5[]="SHT11 ��ʪ�ȼ��";

void delay_us(unsigned int us){
    do {
        _nop_();
        us--;
    } while(us);
}

/* ����һ����ʱxms�������ʱ����,xms������Ҫ��ʱ�ĺ����� */
void delay_ms(unsigned int ms) {
    do {
        delay_us(1000);
        ms--;
    } while(ms);
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
writeCom_12864(0x8C);
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
writeData_12864(shidu[i]);
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


//æ��⣬��æ��ȴ�����ȴ�ʱ��Ϊ60ms
void busychk_12864(void){
    unsigned int timeout = 0;
    EN_LCD = 0;
    RS_LCD = 0;
    RW_LCD = 1;
    EN_LCD = 1;
    while((DATA & 0x80) && ++timeout != 0);  //æ״̬��⣬�ȴ���ʱʱ��Ϊ60ms
    EN_LCD = 0;
}

//д�����ӳ���
void writeCom_12864(uchar com) {
    busychk_12864();
    EN_LCD = 0;
    RS_LCD=0;
    RW_LCD=0;
    DATA = com;
    EN_LCD=1;
    delay_us(50);    //50usʹ����ʱ!!!ע���������ǽϿ��CPUӦ����ʱ��һЩ
    EN_LCD=0;
}

//д�����ӳ���
void writeData_12864(uchar dat) {
    busychk_12864();
    EN_LCD = 0;
    RS_LCD=1;
    RW_LCD=0;
    EN_LCD=1;
    DATA = dat;
    delay_us(50);    //50usʹ����ʱ!!!ע���������ǽϿ��CPUӦ����ʱ��һЩ
    EN_LCD=0;
}

void display1(void) {
    uchar i;
    writeCom_12864(0x80);
    for(i=0;i<16;i++) {
        writeData_12864(table2[i]);
        delay_us(10);
    }
}
void display2(void) {
    uchar i;
    writeCom_12864(0x90);
    for(i=0;i<16;i++) {
        writeData_12864(table2[i]);
        delay_us(50);
    }
}
void display3(void)
{
    uchar i;
    writeCom_12864(0x88);
    for(i=0;i<8;i++) {
        writeData_12864(table4[i]);
        // delay_50us(1);
    }
}
void display4(void) {
    uchar i;
    writeCom_12864(0x88);
    for(i=0;i<8;i++)
    {
        writeData_12864(table5[i]);
        //delay_50us(1);
    }
}


void init_LCD(void) {
    writeCom_12864(0x30);   //����Ϊ����ָ�����
    delay_us(100);
    writeCom_12864(0x30);   //����Ϊ����ָ�����
    delay_us(37);
    writeCom_12864(0x08);   //������ʾ����ꡢ��˸ȫ�ء�
    delay_us(100);
    writeCom_12864(0x01);   //����������DDRAM����ָ������
    delay_us(100);
    writeCom_12864(0x06);   //����ģʽ����
    writeCom_12864(0x0C);   //����ʾ���޹�꣬��겻��˸
}

void main(void) {
    init_LCD();
    display1();

    display2();
    /*
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