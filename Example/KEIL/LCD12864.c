#include <reg51.h>       //51芯片管脚定义头文件
#include <intrins.h>

#define LCD_DB  P0
sbit LCD_RS = P2^0;
sbit LCD_RW = P2^1;
sbit LCD_EN = P2^2;
//sbit LCD_PSB = P2^3;
//sbit LCD_RST = P2^5;

void DELAY()
{
    unsigned char i;
    for(i=1;i<=10;i++) ;

}
void LCDWait()
{
    unsigned  char sta;
    LCD_DB = 0xFF;
    LCD_RS = 0;
    LCD_RW = 1;
    do{
       LCD_EN   = 1;
       sta = LCD_DB;
       LCD_EN = 0;
    }while ( sta & 0x80);
}

void LCDWriteCmd(unsigned char cmd)
{
    LCDWait();
    LCD_DB = cmd;
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_EN = 1;
    DELAY();
    LCD_EN = 0;
}

void LCDWriteData(unsigned char dat)
{
    LCDWait();
    LCD_DB = dat;
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_EN = 1;
    DELAY();
    LCD_EN = 0;
}

void LcdFullClear()
{
    LCDWriteCmd(0x01);
}

void LCDLineClear(unsigned char x)
{
    unsigned char len=16;
    while (len--)         //连续写入空格
    {
        LCDWriteData(' ');
    }
}

void InitLcd()
{

    LCDWriteCmd(0x34);
    LCDWriteCmd(0x30);
    LCDWriteCmd(0x0C);
    LCDWriteCmd(0x01);
}

/* 设置显示RAM起始地址，亦即光标位置，(x,y)-对应屏幕上的字符坐标 */
void LCDSetCursor(unsigned char x, unsigned char y)
{
    unsigned char addr;
    switch(x)
    {
        case 1:addr = 0x80 + y;break;
        case 2:addr = 0x90 + y;break;
        case 3:addr = 0x88 + y;break;
        case 4:addr = 0x98 + y;break;
        default:break;
    }
    LCDWriteCmd(addr);  //设置RAM地址
}


void LCDShow(unsigned char x, unsigned char y,unsigned char *str)
{
    LCDSetCursor(x,y);

    while (*str != '\0')  //连续写入字符串数据，直到检测到结束符
    {
        LCDWriteData(*str++);
    }
}

