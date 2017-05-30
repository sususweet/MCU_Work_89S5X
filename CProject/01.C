#include <reg51.h>
#include <intrins.h>

unsigned char KEY_STATE = 1;
unsigned char KEY_CODE = 0x00;
unsigned char LAST_CODE = 0x00;
unsigned char NEWCODE = 0;
unsigned char tmp = 0x00;

void  key_scan(void);
unsigned char de_code(void);

main()
{
    TMOD = 0x21;				  //定时器1工作于8位自动重载模式, 用于产生波特率
    TH1 = 0xF3;				 	  //波特率2400
    TL1 = 0xF3;
    TH0 = 0xe8;                                   //写定时常数，定时6ms
    TL0 = 0x90;
    SCON = 0x40;				  //设定串行口工作方式
    PCON &= 0x7f;				  //波特率不倍增
    TR0 = 1;					  //启动定时器0
    TR1 = 1;                                      //启动定时器1
    IE = 0x82;					  //仅允许T0中断
    while(1)
    {
	while(tmp != 0x00)
	{
               SBUF = tmp;
	       while(!TI);
               TI = 0;
               tmp = 0;
	}
    }
}

void intT0(void) interrupt 1
{
    TH0 = 0xe8;                                   //重写定时常数，定时6ms
    TL0 = 0x90;
    key_scan();
    if(NEWCODE != 0)
    {
        NEWCODE = 0;
        tmp = de_code();
    }
}

void key_scan()                                   //键盘扫面子程序，将扫描得到的键码返回键码为十进制1-16
{
    switch(KEY_STATE)
    {
        case 1:
            P3 = 0xf0;
            if(P3 != 0xf0)
            {
                KEY_STATE = 2;
                LAST_CODE = P3;
                P3 = 0x0f;
                LAST_CODE += P3;
            }
            break;
        case 2:
            P3 = 0xf0;
            KEY_CODE = P3;
            P3 = 0x0f;
            KEY_CODE += P3;
            if(KEY_CODE == LAST_CODE)
                KEY_STATE = 3;
            else
                KEY_STATE = 1;
            break;
        case 3:
            P3 = 0xf0;
            KEY_CODE = P3;
            P3 = 0x0f;
            KEY_CODE += P3;
            if(KEY_CODE == 0xff)
            {
                KEY_STATE = 1;
                NEWCODE = 1;
            }
            break;
    }
}

unsigned char de_code(void)
{
    unsigned char out;
    switch(LAST_CODE)
    {
        case 0x77:out = 1;break;
        case 0xb7:out = 2;break;
        case 0xd7:out = 3;break;
        case 0xe7:out = 4;break;
        case 0x7b:out = 5;break;
        case 0xbb:out = 6;break;
        case 0xdb:out = 7;break;
        case 0xeb:out = 8;break;
        case 0x7d:out = 9;break;
        case 0xbd:out = 10;break;
        case 0xdd:out = 11;break;
        case 0xed:out = 12;break;
        case 0x7e:out = 13;break;
        case 0xbe:out = 14;break;
        case 0xde:out = 15;break;
        case 0xee:out = 16;break;
    }
    return out;
}

