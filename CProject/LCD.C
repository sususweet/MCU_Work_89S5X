#include "REG51.H"
#include "INTRINS.H"
#include "STRING.H"
#include "STDLIB.H"
#include "STDIO.H"

#define DATA_LCD P0
sbit RS_LCD = P2^0;
sbit RW_LCD = P2^1;
sbit EN_LCD = P2^2;
sbit PSB_LCD = P2^3;
sbit RST_LCD = P2^5;

#define KEYBOARD P1

/*定时器0 2ms*/
#define TIMER0_TH 0xF8
#define TIMER0_TL 0x30
#define TWINKLE_FREQ 250    /*闪烁周期*/
#define TIME_FREQ 500    /*时钟计时周期*/
#define DISP_FREQ 200   /*LCD显示刷新周期*/
#define KEY_WAIT 10    /*键盘扫描延迟周期*/
#define MAX_SPACE 3

#define MEASURE_TEMP 0x03
#define MEASURE_HUMI 0x05
#define RESET 0x1e

typedef unsigned char uchar;
typedef struct {
    unsigned int startTime;
    unsigned int endTime;
    unsigned char used;
    unsigned char carID;
    float pay;
} ParkInfo;
//定义共用同类型
typedef union {
    unsigned int i;
    float f;
} value;
enum key_states_e {
    KEY_STATE_RELEASE,
    KEY_STATE_WAITING,
    KEY_STATE_PRESSED
};
enum working_state {
    NORMAL, COME_IN, COME_OUT
};

unsigned char code DateStr[]=__DATE__;
unsigned char code TimeStr[]=__TIME__;
uchar idata LCDTable1[16], LCDTable2[16], LCDTable3[16], LCDTable4[16];

unsigned int working_stage = NORMAL;
unsigned int nowTime = 0;
unsigned int sec = 0, minute = 0, hour = 0;
/*unsigned int day = 0, month = 0, year = 0;*/
ParkInfo idata parkSpace[MAX_SPACE];

unsigned int is_leap_year(unsigned int year);

void time_inc();
void init_settings();
void init_timer0();
void opr_key(unsigned int key_code);
unsigned int press_key();
unsigned int read_key();


/*基础函数库开始*/
void delay_us(unsigned int us) {
    do {
        _nop_();
        us--;
    } while (us);
}

/* 定义一个延时xms毫秒的延时函数,xms代表需要延时的毫秒数 */
void delay_ms(unsigned int ms) {
    do {
        delay_us(1000);
        ms--;
    } while (ms);
}

/**
 * @desc 闰年判断函数
 * @param year 需要进行判断的年份
 * @return 1:是闰年  0:不是闰年
 * */
unsigned int is_leap_year(unsigned int year) {
    unsigned int leap_year;
    if ((year + 2000) % 400 == 0) leap_year = 1;    //  被400整除为闰年
    else if ((year + 2000) % 100 == 0) leap_year = 0;  // 不能被400整除 能被100整除 不是闰年
    else if ((year + 2000) % 4 == 0) leap_year = 1;  // 不能被400、100整除 能被4整除 是闰年
    else leap_year = 0;
    return leap_year;
}

/**
 * @desc 时间自增程序
 * */
void time_inc() {
    sec++;
    /*一般的时间进位逻辑*/
    if (sec >= 60) {
        minute += 1;
        sec = 0;
    }
    if (minute >= 60) {
        hour += 1;
        minute = 0;
    }
    if (hour >= 24) {
        hour = 0;
       /* day++;
        if (month == 1 | month == 3 | month == 5 | month == 7 | month == 8 | month == 10 | month == 12) {
            if (day > 31) {
                day = 1;
                month++;
                if (month >= 13) {
                    month = 1;
                    year++;
                }
            }
        }
        if (month == 4 | month == 6 | month == 9 | month == 11) {
            if (day > 30) {
                day = 1;
                month++;
            }
        }
        if (month == 2) {
            if (is_leap_year(year)) {
                if (day >= 30) {
                    day = 1;
                    month++;
                }
            } else {
                if (day >= 29) {
                    day = 1;
                    month++;
                }
            }
        }*/
    }
    return;
}

void scan_key() {
    static int key_state = KEY_STATE_RELEASE;   /*状态机状态初始化，采用static保存状态*/
    unsigned int wait_time = KEY_WAIT;   /*按键扫描等待时间*/
    static int key_code = -1;
    unsigned int pressed = press_key(); /*press_key为检测是否有按键按下的函数*/
    static scan_time = 0;
    switch (key_state) {
        case KEY_STATE_RELEASE: {   /*若原始状态为无按键按下RELEASE，同时又检测到按键按下，则状态转换到WAITING*/
            if (pressed == 1) {
                key_state = KEY_STATE_WAITING;
            }
            break;
        }
        case KEY_STATE_WAITING: {   /*原始状态为WAITING，对按键进行多次判断*/
            if (pressed) {
                scan_time++;
                if (scan_time >= wait_time) {   /*若按键按下的时间超过一定时间，则认为按键按下，读按键*/
                    key_state = KEY_STATE_PRESSED;
                    scan_time = 0;
                    key_code = read_key();  /*read_key为读按键的函数*/
                }
            } else {    /*若按键松开，则恢复到初始状态*/
                scan_time = 0;
                key_state = KEY_STATE_RELEASE;
            }
            break;
        }
        case KEY_STATE_PRESSED: {   /*若按键被确认按下，则等待按键松开再进行操作*/
            if (pressed == 0 && key_code >= 0) {
                opr_key((unsigned int) key_code);  /*opr_key为按键事件响应函数*/
                key_state = KEY_STATE_RELEASE;
                key_code = -1;
            }
            break;
        }
        default:
            break;
    }
}

//判断是否有键按下
unsigned int press_key() {
    unsigned char temp_line, temp_row;
    KEYBOARD = 0xF0;
    temp_line = KEYBOARD;
    if (temp_line == 0xF0) return 0;

    KEYBOARD = 0x0F;
    temp_row = KEYBOARD;
    if (temp_row == 0x0F) return 0;
    return 1;
}

//扫描键盘，返回键值
unsigned int read_key() {
    unsigned char temp_column, temp_row;
    unsigned int row, column, key_code;
    KEYBOARD = 0xF0;
    temp_column = KEYBOARD;

    KEYBOARD = 0x0F;
    temp_row = KEYBOARD;

    switch (temp_column) {
        case 0x70: {
            column = 0;
            break;
        }

        case 0xB0: {
            column = 1;
            break;
        }

        case 0xD0: {
            column = 2;
            break;
        }

        case 0xE0: {
            column = 3;
            break;
        }

        default:
            break;
    }

    switch (temp_row) {
        case 0x07: {
            row = 0;
            break;
        }
        case 0x0B: {
            row = 1;
            break;
        }
        case 0x0D: {
            row = 2;
            break;
        }
        case 0x0E: {
            row = 3;
            break;
        }
        default:
            break;
    }
    key_code = 4 * row + column;
    return key_code;
}
/*基础函数库结束*/


/*获取剩余空间*/
int getSpace() {
    unsigned int i = 0;
    for (i = 0; i < MAX_SPACE; i++) {
        if (parkSpace[i].used == 0) return i;
    }
    return -1;
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
write_dat(shidu[i]);
delay_50us(1);
}
}
//写字节程序
char s_write_byte(unsigned char value)
{
unsigned char i,error=0;
for (i=0x80;i>0;i>>=1) //高位为1，循环右移
{
if (i&value) DATA_LCD=1; //和要发送的数相与，结果为发送的位
 else DATA_LCD=0;
 SCK=1;
 _nop_();_nop_();_nop_(); //延时3us
 SCK=0;
}
DATA_LCD=1; //释放数据线
SCK=1;
error=DATA_LCD; //检查应答信号，确认通讯正常
_nop_();_nop_();_nop_();
SCK=0;
DATA_LCD=1;
return error; //error=1 通讯错误
}
//读字节程序
char s_read_byte(unsigned char ack)
{
unsigned char i,val=0;
DATA_LCD=1; //释放数据线
for(i=0x80;i>0;i>>=1) //高位为1，循环右移
{
SCK=1;
 if(DATA_LCD) val=(val|i); //读一位数据线的值
 SCK=0;
}
DATA_LCD=!ack; //如果是校验，读取完后结束通讯；
SCK=1;
_nop_();_nop_();_nop_(); //延时3us
SCK=0;
_nop_();_nop_();_nop_();
DATA_LCD=1; //释放数据线
return val;
}
//启动传输
void s_transstart(void)
{
 DATA_LCD=1; SCK=0; //准备
 _nop_();
 SCK=1;
 _nop_();
 DATA_LCD=0;
 _nop_();
 SCK=0;
 _nop_();_nop_();_nop_();
 SCK=1;
 _nop_();
 DATA_LCD=1;
 _nop_();
 SCK=0;
}
//连接复位
void s_connectionreset(void)
{
unsigned char i;
DATA_LCD=1; SCK=0; //准备
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
for (i=0;i<65535;i++) if(DATA_LCD==0) break; //等待测量结束
if(DATA_LCD) error+=1; // 如果长时间数据线没有拉低，说明测量错误
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
void busyCheck_12864(void) {
    unsigned int timeout = 0;
    DATA_LCD = 0xff;
    RS_LCD = 0;
    RW_LCD = 1;
    EN_LCD = 1;
    while ((DATA_LCD & 0x80) && ++timeout != 0);  //忙状态检测，等待超时时间为60ms
    EN_LCD = 0;
}

//写命令子程序
void writeCom_12864(uchar com) {
    busyCheck_12864();
    RS_LCD = 0;
    RW_LCD = 0;
    DATA_LCD = com;
    EN_LCD = 1;
    _nop_();
    _nop_();
    //delay_us(50);    //50us使能延时!!!注意这里，如果是较快的CPU应该延时久一些
    EN_LCD = 0;
}

//写数据子程序
void writeData_12864(uchar dat) {
    //busyCheck_12864();
    RS_LCD = 1;
    RW_LCD = 0;
    DATA_LCD = dat;
    EN_LCD = 1;
    _nop_();
    _nop_();
    //delay_us(50);    //50us使能延时!!!注意这里，如果是较快的CPU应该延时久一些
    EN_LCD = 0;
}


/*void write_dat(uchar dat) {
    RS_LCD=1;
    RW_LCD=0;
    delay_50us(1);
    P0=dat;
    EN_LCD=1;
    delay_50us(2);
    EN_LCD=0;
    delay_50us(2);
}
*/

void write_display_cache() {
    switch (working_stage) {
        case NORMAL: {
            unsigned int park_space = 0;
            char *nowTimeStr = NULL;
            strcpy(LCDTable1,"    欢迎使用    ");
            sprintf(LCDTable2, "剩余车位：%d     ", park_space);
            sprintf(LCDTable3, "                ");
            //sprintf(LCDTable3, "");
            sprintf(LCDTable4, "    %02d:%02d:%02d    ", hour, minute, sec);
            /*sprintf(LCDTable3, "    %02d:%02d:%02d    ", hour, minute, sec);*/
            //strcpy(LCDTable2, strcat("剩余车位：", (unsigned char) park_space));
            /*sprintf(LCDTable2, "剩余车位：%d    ", park_space);

            /*sprintf(LCDTable4, "By sususweet");*/
            /*strcpy(LCDTable3,nowTimeStr);
            strcpy(LCDTable4,"By sususweet");*/
            break;
        }
        default:
            break;
    }
}

void init_LCD(void) {
    RST_LCD = 0;
    delay_us(80);
    RST_LCD = 1;
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

void displayLCD(void) {
    unsigned int i;
    writeCom_12864(0x01);   //清屏，并且DDRAM数据指针清零
    //delay_us(10);
    writeCom_12864(0x80);
    for(i = 0; i < 16; i++){
        if(LCDTable1[i] != '\0'){
            writeData_12864(LCDTable1[i]);
        }else{
            writeData_12864(0x20);
        }
    }

    writeCom_12864(0x90);
    for(i = 0; i < 16; i++){
        if(LCDTable2[i] != '\0'){
            writeData_12864(LCDTable2[i]);
        }else{
            writeData_12864(0x20);
        }
    }
    writeCom_12864(0x88);
    for (i = 0; i < 16; i++) {
        if(LCDTable3[i] != '\0'){
            writeData_12864(LCDTable3[i]);
        }else{
            writeData_12864(0x20);
        }
    }
    writeCom_12864(0x98);
    for (i = 0; i < 16; i++) {
        if(LCDTable4[i] != '\0'){
            writeData_12864(LCDTable4[i]);
        }else{
            writeData_12864(0x20);
        }
    }
}

/*void display1(void) {
    uchar i;
    writeCom_12864(0x80);
    for(i=0;i<16;i++) {
        writeData_12864(LCDTable2[i]);
        delay_us(1);
    }
}

void display2(void) {
    uchar i;
    writeCom_12864(0x90);
    for(i=0;i<16;i++) {
        writeData_12864(LCDTable3[i]);
        delay_us(1);
    }
}

void display3(void) {
    uchar i;
    writeCom_12864(0x88);
    for(i=0;i<16;i++) {
        writeData_12864(LCDTable4[i]);
        delay_us(1);
    }
}
void display4(void) {
    uchar i;
    writeCom_12864(0x98);
    for(i=0;i<16;i++) {
        writeData_12864(LCDTable4[i]);
        delay_us(1);
    }
}
*/

/*
void init12864lcd(void) {
    delay_50ms(2);
    writeCom_12864(0x30);   //设置为基本指令集动作
    delay_50us(4);
    writeCom_12864(0x30);   //设置为基本指令集动作
    delay_50us(4);
    writeCom_12864(0x08);   //设置显示、光标、闪烁全关。
    delay_50us(4);
    writeCom_12864(0x01);   //清屏，并且DDRAM数据指针清零
    delay_50us(1);
    writeCom_12864(0x06);   //进入模式设置
    delay_50us(10);
    writeCom_12864(0x0C);   //开显示，无光标，光标不闪烁
    delay_50us(10);
}*/

void opr_key(unsigned int key_code) {
    switch (key_code) {
        case 0: {

        }
        case 1: {

        }
        case 2: {

        }
        default:
            break;
    }
}

int main(void) {
    /*value humi_val, temp_val; //定义两个共同体，一个用于湿度，一个用于温度   */
    init_settings();
    init_LCD();

    TMOD = 0x01;
    EA = 1;
    init_timer0();

    while (1) {

    }
    return 0;
    /*display1();
    display2();
    display3();*/
    /*

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

void init_settings() {
    unsigned int i = 0;
    for (i = 0; i < MAX_SPACE; i++) {
        ParkInfo parkInfo;
        parkInfo.used = 0;
        parkSpace[i] = parkInfo;
    }
    hour = atoi(strcat(&TimeStr[0], &TimeStr[1]));
    minute = atoi(strcat(&TimeStr[3], &TimeStr[4]));
    sec = atoi(strcat(&TimeStr[6], &TimeStr[7]));
}

void init_timer0() {
    ET0 = 1;
    TR0 = 1;
    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
}

void interrupt0() {
    scan_key();
    return;
}

void int0() interrupt 1{
    static unsigned int time_num = 0,disp_num = 0;
    time_num++;
    disp_num++;

    if (time_num >= TIME_FREQ) {
        time_num = 0;
        time_inc();
        nowTime++;
    }

    if (disp_num >= DISP_FREQ) {
        write_display_cache();
        displayLCD();
        disp_num = 0;
    }

    interrupt0();

    TH0 = TIMER0_TH;
    TL0 = TIMER0_TL;
}
