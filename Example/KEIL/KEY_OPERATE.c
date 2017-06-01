#include <reg51.h>       //51芯片管脚定义头文件
sbit KEY_IN_1 = P1^7;
sbit KEY_IN_2 = P1^6;
sbit KEY_IN_3 = P1^5;
sbit KEY_IN_4 = P1^4;
sbit KEY_OUT_4 = P1^0;
sbit KEY_OUT_3 = P1^1;
sbit KEY_OUT_2 = P1^2;
sbit KEY_OUT_1 = P1^3;

extern void KeyAction(unsigned char keycode);

unsigned char KeySta[4][4] = {
	{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}
	};

unsigned char code KeyCodeMap[4][4] = { //矩阵按键编号到标准键盘键码的映射表
    { 0x01, 0x02, 0x03, 0x0A },
    { 0x04, 0x05, 0x06, 0x0B },
    { 0x07, 0x08, 0x00, 0x0C },
    { 0x1F, 0x00, 0x0E, 0x0D }
    };


void  KeyDriver()
{
	unsigned char i, j;
	static	unsigned char backup [4][4] = {
	{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}
	};

	for(i=0; i<4; i++)
		{
			for(j=0; j<4; j++)
			{
				if(backup[i][j] != KeySta[i][j])
				{
					if(backup[i][j] == 0)
					{
						KeyAction(KeyCodeMap[i][j]);
					}
					backup[i][j] = KeySta[i][j];
				}
			}
		}
}

void KeyScan()
{
    unsigned char i;
    static unsigned char keyout = 0;   //矩阵按键扫描输出索引
    static unsigned char keybuf[4][4] = {  //矩阵按键扫描缓冲区
        {0xFF, 0xFF, 0xFF, 0xFF},  {0xFF, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xFF, 0xFF},  {0xFF, 0xFF, 0xFF, 0xFF}
    };

    //将一行的4个按键值移入缓冲区
    keybuf[keyout][0] = (keybuf[keyout][0] << 1) | KEY_IN_1;
    keybuf[keyout][1] = (keybuf[keyout][1] << 1) | KEY_IN_2;
    keybuf[keyout][2] = (keybuf[keyout][2] << 1) | KEY_IN_3;
    keybuf[keyout][3] = (keybuf[keyout][3] << 1) | KEY_IN_4;
    //消抖后更新按键状态
    for (i=0; i<4; i++)  //每行4个按键，所以循环4次
    {
        if ((keybuf[keyout][i] & 0x0F) == 0x00)
        {   //连续4次扫描值为0，即4*4ms内都是按下状态时，可认为按键已稳定的按下
            KeySta[keyout][i] = 0;
        }
        else if ((keybuf[keyout][i] & 0x0F) == 0x0F)
        {   //连续4次扫描值为1，即4*4ms内都是弹起状态时，可认为按键已稳定的弹起
            KeySta[keyout][i] = 1;
        }
    }
    //执行下一次的扫描输出
    keyout++;                //输出索引递增
    keyout = keyout & 0x03;  //索引值加到4即归零
    switch (keyout)          //根据索引，释放当前输出引脚，拉低下次的输出引脚
    {
        case 0: KEY_OUT_4 = 1; KEY_OUT_1 = 0; break;
        case 1: KEY_OUT_1 = 1; KEY_OUT_2 = 0; break;
        case 2: KEY_OUT_2 = 1; KEY_OUT_3 = 0; break;
        case 3: KEY_OUT_3 = 1; KEY_OUT_4 = 0; break;
        default: break;
    }
}
