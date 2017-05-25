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
typedef union //定义共用同类型
{
    unsigned int i;
    float f;
} value;
sbit RS_LCD=P2^0;
sbit RW_LCD=P2^1;
sbit EN_LCD=P2^2;
sbit SCK = P1^0;
//sbit DATA = P1^1;
uchar table2[]="微机原理与应用";
uchar table3[]="温度为:     ";
uchar table4[]="湿度为:    ";
uchar table5[]="SHT11 温湿度检测";

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
//写字节程序
char s_write_byte(unsigned char value)
{
unsigned char i,error=0;
for (i=0x80;i>0;i>>=1) //高位为1，循环右移
{
if (i&value) DATA=1; //和要发送的数相与，结果为发送的位
 else DATA=0;
 SCK=1;
 _nop_();_nop_();_nop_(); //延时3us
 SCK=0;
}
DATA=1; //释放数据线
SCK=1;
error=DATA; //检查应答信号，确认通讯正常
_nop_();_nop_();_nop_();
SCK=0;
DATA=1;
return error; //error=1 通讯错误
}
//读字节程序
char s_read_byte(unsigned char ack)
{
unsigned char i,val=0;
DATA=1; //释放数据线
for(i=0x80;i>0;i>>=1) //高位为1，循环右移
{
SCK=1;
 if(DATA) val=(val|i); //读一位数据线的值
 SCK=0;
}
DATA=!ack; //如果是校验，读取完后结束通讯；
SCK=1;
_nop_();_nop_();_nop_(); //延时3us
SCK=0;
_nop_();_nop_();_nop_();
DATA=1; //释放数据线
return val;
}
//启动传输
void s_transstart(void)
{
 DATA=1; SCK=0; //准备
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
//连接复位
void s_connectionreset(void)
{
unsigned char i;
DATA=1; SCK=0; //准备
for(i=0;i<9;i++) //DATA保持高，SCK时钟触发9次，发送启动传输，通迅即复位
{
SCK=1;
 SCK=0;
}
s_transstart(); //启动传输
}*/
/*
//软复位程序
char s_softreset(void)
{
unsigned char error=0;
s_connectionreset(); //启动连接复位
error+=s_write_byte(RESET); //发送复位命令
return error; //error=1 通讯错误
}
//温湿度测量
char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
{
unsigned error=0;
unsigned int i;
s_transstart(); //启动传输
switch(mode) //选择发送命令
 {
case TEMP : error+=s_write_byte(MEASURE_TEMP); break; //测量温度
 case HUMI : error+=s_write_byte(MEASURE_HUMI); break; //测量湿度
 default : break;
}
for (i=0;i<65535;i++) if(DATA==0) break; //等待测量结束
if(DATA) error+=1; // 如果长时间数据线没有拉低，说明测量错误
*(p_value) =s_read_byte(ACK); //读第一个字节，高字节 (MSB)
*(p_value+1)=s_read_byte(ACK); //读第二个字节，低字节 (LSB)
*p_checksum =s_read_byte(noACK); //read CRC校验码
return error; // error=1 通讯错误
}
//温湿度值标度变换及温度补偿
void calc_sth10(float *p_humidity ,float *p_temperature)
{
const float C1=-4.0; // 12位湿度精度 修正公式
const float C2=+0.0405; // 12位湿度精度修正公式
const float C3=-0.0000028; // 12位湿度精度修正公式
const float T1=+0.01; // 14位温度精度 5V条件 修正公式
const float T2=+0.00008; // 14位温度精度 5V条件 修正公式
float rh=*p_humidity; // rh: 12位 湿度
float t=*p_temperature; // t: 14位温度
float rh_lin; // rh_lin: 湿度 linear值
float rh_true; // rh_true: 湿度 ture值
float t_C; // t_C : 温度 ℃
t_C=t*0.01 - 40; //补偿温度
rh_lin=C3*rh*rh + C2*rh + C1; //相对湿度非线性补偿
rh_true=(t_C-25)*(T1+T2*rh)+rh_lin; //相对湿度对于温度依赖性补偿
if(rh_true>100)rh_true=100; //湿度最大修正
if(rh_true<0.1)rh_true=0.1; //湿度最小修正
*p_temperature=t_C; //返回温度结果
*p_humidity=rh_true; //返回湿度结果
}

*/


//忙检测，若忙则等待，最长等待时间为60ms
void busychk_12864(void){
    unsigned int timeout = 0;
    EN_LCD = 0;
    RS_LCD = 0;
    RW_LCD = 1;
    EN_LCD = 1;
    while((DATA & 0x80) && ++timeout != 0);  //忙状态检测，等待超时时间为60ms
    EN_LCD = 0;
}

//写命令子程序
void writeCom_12864(uchar com) {
    busychk_12864();
    EN_LCD = 0;
    RS_LCD=0;
    RW_LCD=0;
    DATA = com;
    EN_LCD=1;
    delay_us(50);    //50us使能延时!!!注意这里，如果是较快的CPU应该延时久一些
    EN_LCD=0;
}

//写数据子程序
void writeData_12864(uchar dat) {
    busychk_12864();
    EN_LCD = 0;
    RS_LCD=1;
    RW_LCD=0;
    EN_LCD=1;
    DATA = dat;
    delay_us(50);    //50us使能延时!!!注意这里，如果是较快的CPU应该延时久一些
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
    writeCom_12864(0x30);   //设置为基本指令集动作
    delay_us(100);
    writeCom_12864(0x30);   //设置为基本指令集动作
    delay_us(37);
    writeCom_12864(0x08);   //设置显示、光标、闪烁全关。
    delay_us(100);
    writeCom_12864(0x01);   //清屏，并且DDRAM数据指针清零
    delay_us(100);
    writeCom_12864(0x06);   //进入模式设置
    writeCom_12864(0x0C);   //开显示，无光标，光标不闪烁
}

void main(void) {
    init_LCD();
    display1();

    display2();
    /*
    display3();
    display4();*/
    //s_connectionreset(); //启动连接复位
    /*while(1)
    {
    error=0; //初始化error=0，即没有错误
    error+=s_measure((unsigned char*)&temp_val.i,&checksum,TEMP); //温度测量
    error+=s_measure((unsigned char*)&humi_val.i,&checksum,HUMI); //湿度测量
     if(error!=0) s_connectionreset(); ////如果发生错误，系统复位
     else
     {
    humi_val.f=(float)humi_val.i; //转换为浮点数
     temp_val.f=(float)temp_val.i; //转换为浮点数
     calc_sth10(&humi_val.f,&temp_val.f); //修正相对湿度及温度
     temp=temp_val.f*10;
     humi=humi_val.f*10;
     wendu[0]=temp/1000+0; //温度百位
     wendu[1]=temp/100+0; //温度十位
     wendu[2]=temp/10+0; //温度个位
     wendu[3]=0x2E; //小数点
     wendu[4]=temp+0; //温度小数点后第一位
    displaywendu();
    shidu[0]=humi/1000+0; //湿度百位
     shidu[1]=humi/100+0; //湿度十位
     shidu[2]=humi/10+0; //湿度个位
     shidu[3]=0x2E; //小数点
     shidu[4]=humi+0; //湿度小数点后第一位
    displayshidu();
     }
    delay(800); //等待足够长的时间，以现行下一次转换
    }*/
}
