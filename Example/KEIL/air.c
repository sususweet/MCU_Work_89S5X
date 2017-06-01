#include <reg51.h>       //51芯片管脚定义头文件
#include <intrins.h>	 //内部包含延时函数 _nop_();
#include <math.h>

#define Delay()  {_nop_();_nop_();_nop_();_nop_();}
char code SST516[3] _at_ 0x003b;

sbit ADC_WR= P2^6;
sbit ADC_RD= P2^7;
sbit ring=P2^4;

unsigned char code FFW[8]={0x1f,0x3f,0x2f,0x6f,0x4f,0xcf,0x8f,0x9f};

unsigned char minute=10;
unsigned char second=10;
unsigned char wendu;
unsigned char now_wendu=25;
unsigned char mode = 1;
unsigned char count_Y=1;
bit trans = 1;
unsigned char code display1[4][10]={
{"选项"},
{"设定工作"},
{"当前状态"},
{"产品信息"}
};
unsigned char code display2[3][14]={
{"设定工作:"},
{"时间：__:__"},
{"控温：__℃"}
};
unsigned char code display3[4][11]={
{"当前状态:"},
{"实时温度："},
{"工作温度："},
{"剩余时间："},
};
unsigned char code display4[3][14]={
{"智能温控系统"},
{"性能：好"},
{"设计者：余鹏"}
};
unsigned char code display5[2][16]={
{"error_too hot"},
{"error_too cold"}
};

extern void  KeyDriver();
extern void KeyScan();
extern void LCDShow(unsigned char x,unsigned char y, unsigned char *str);
extern void InitLcd();
extern void LcdFullClear();
extern void I2CR24(unsigned char *buf, unsigned char addr, unsigned char len);
extern void I2CW24(unsigned char *buf, unsigned char addr, unsigned char len);

void KeyAction(unsigned char keycode)
{
    static unsigned char cursor = 1;
    static unsigned char last_cursor=1;
    static unsigned char cursor2 = 1;
    static unsigned char last_cursor2=1;
	static unsigned char temp=1;					//换了位置
	static unsigned char time=1;



    if(mode==1)
    {
       if (keycode==0x0A)
       {   if(cursor!=1) cursor--; }
       else if (keycode==0x0B)
       {   if(cursor!=3) cursor++; }
       else if (keycode==0x0C)
       {
            if(cursor==1)
            {
                mode=2;
				temp=1;
				time=1;
				cursor2=1;last_cursor2=1;
                LcdFullClear();
                LCDShow(1,0,  display2[0]);
                LCDShow(2,2,  display2[1]);
                LCDShow(3,2,  display2[2]);
                LCDShow(cursor2+1,0,"→");
            }
            else if(cursor==2)
            {
                mode=3;
				LcdFullClear();
           		LCDShow(1,0,display3[0]);
    	   		LCDShow(2,0,display3[1]);
           		LCDShow(3,0,display3[2]);
				LCDShow(4,0,display3[3]);

            }
			else
			{
				mode = 4;
				LcdFullClear();
           		LCDShow(1,1,display4[0]);
    	   		LCDShow(2,2,display4[1]); 
				LCDShow(4,1,display4[2]);
			}
       }
       else;

       if(cursor!=last_cursor)
       {
        LCDShow(last_cursor+1,0,"  ");
        LCDShow(cursor+1,0,"→");
        last_cursor=cursor;
       }
   }
   else if(mode==2)
   {
       
       static unsigned char temp1,temp2;
       static unsigned char minute1,minute2,second1,second2;
       unsigned char tmp[6];
       if (keycode==0x0D)
       {
	   	   LcdFullClear();
           LCDShow(1,3,display1[0]);
    	   LCDShow(2,2,display1[1]);
           LCDShow(3,2,display1[2]);
		   LCDShow(4,2,display1[3]);
           LCDShow(cursor+1,0, "→");
           mode=1;
       }
       else if (keycode==0x0A)
       {   if(cursor2!=1) cursor2--; }
       else if (keycode==0x0B)
       {   if(cursor2!=2) cursor2++; }
       else if (keycode>=0x00 && keycode<= 0x09)
       {
           if(cursor2==1)
           {
                switch(time)
                {
                    case 1:minute1=keycode;tmp[0]=keycode+'0';tmp[1]='\0';LCDShow(2,5,tmp);time++;break;
                    case 2:minute2=keycode;tmp[0]=minute1+'0';tmp[1]=keycode+'0';tmp[2]='\0';LCDShow(2,5,tmp);time++;break;
                    case 3:second1=keycode;tmp[0]=':';tmp[1]=second1+'0';tmp[2]='\0';LCDShow(2,6,tmp);time++;break;
                    case 4:second2=keycode;tmp[0]=second2+'0';tmp[1]='\0';LCDShow(2,7,tmp);time++;break;
                    default:break;
                }
           }
           if(cursor2==2)
           {
                switch(temp)
                {
                    case 1:temp1=keycode;tmp[0]=keycode+'0';tmp[1]='\0';LCDShow(3,5,tmp);temp++;break;
                    case 2:temp2=keycode;tmp[0]=temp1+'0';tmp[1]=keycode+'0';tmp[2]='\0';LCDShow(3,5,tmp);temp++;break;
                    default:break;
                }
           }
       }
       else if (keycode==0x0E)
       {
           if(cursor2==1)
           {
                switch(time)
                {
                    case 2:tmp[0]='_';tmp[1]='_';tmp[2]='\0';LCDShow(2,5,tmp);time--;break;
                    case 3:tmp[0]=minute1+'0';tmp[1]='_';tmp[2]='\0';LCDShow(2,5,tmp);time--;break;
                    case 4:tmp[0]=':';tmp[1]='_';tmp[2]='\0';LCDShow(2,6,tmp);time--;break;
                    case 5:tmp[0]='_';tmp[1]='\0';LCDShow(2,7,tmp);time--;break;
                    default:break;
                }
           }
           if(cursor2==2)
           {
                switch(temp)
                {
                    case 2:tmp[0]='_';tmp[1]='_';tmp[2]='\0';LCDShow(3,5,tmp);temp--;break;
                    case 3:tmp[0]=temp1+'0';tmp[1]='_';tmp[2]='\0';LCDShow(3,5,tmp);temp--;break;
                    default:break;
                }
           }
       }
      else if (keycode==0x0C)
      {
            minute=minute1*10+minute2;
            second=second1*10+second2;
            wendu=temp1*10+temp2;
            temp=1;time=1;
			LcdFullClear();				 
			mode=3;
			count_Y=1;
			trans = 1; 
            LCDShow(1,0,display3[0]);
    		LCDShow(2,0,display3[1]);
        	LCDShow(3,0,display3[2]);
			LCDShow(4,0,display3[3]);

      }
	  if(cursor2!=last_cursor2)
      {
        	LCDShow(last_cursor2+1,0,"  ");
        	LCDShow(cursor2+1,0,"→");
        	last_cursor2=cursor2;
      }

   }
   else if(mode==3 || mode == 4)
   {
		if (keycode==0x0D)
       	{
		   LcdFullClear();
           LCDShow(1,3,  display1[0]);
           LCDShow(2,2,  display1[1]);
           LCDShow(3,2,  display1[2]);
		   LCDShow(4,2,  display1[3]);
           LCDShow(cursor+1,0, "→");
           mode=1;
      	 }
	//	 if(keycode==0x01)
//	{	minute=10;second=10; count_Y=0;trans=0;}
   }
}



unsigned char  ABS(unsigned char xx,unsigned char yy )
{
   if(xx>yy)
		return xx-yy;
   else 
		return yy-xx;
}

void ConfigUART(unsigned int baud)
{
    SCON  = 0x50;  //配置串口为模式1
    TMOD &= 0x0F;  //清零T1的控制位
    TMOD |= 0x20;  //配置T1为模式2
    TH1 = 256 - (12000000/12/32)/baud;  //计算T1重载值
    TL1 = TH1;     //初值等于重载值
    ET1 = 0;       //禁止T1中断
    ES  = 1;       //使能串口中断
    TR1 = 1;       //启动T1
}


void main()
 {
    unsigned char test[6];
	static bit change=0;

    TMOD = 0x01;  //设置T0为模式1
    TH0  = 0xEE;  //为T0赋初值0xFC67，定时5ms
    TL0  = 0x00;
    ET0  = 1;     //使能T0中断
    TR0  = 1;     //启动T0
    
	ConfigUART(9600);  //配置波特率为9600

    I2CR24(test, 0x01, 4);
    minute = test[1];
    second = test[0];
	wendu  = test[2];
	count_Y= test[3];
	EA = 1;       //使能总中断

    InitLcd();
	if(minute || second)
	{
		mode = 3;
		LCDShow(1,0,display3[0]);
    	LCDShow(2,0,display3[1]);
        LCDShow(3,0,display3[2]);
		LCDShow(4,0,display3[3]);
	}
	else
	{
	    LCDShow(1,3,display1[0]);
	    LCDShow(2,2,display1[1]);
	    LCDShow(3,2,display1[2]);
		LCDShow(4,2,display1[3]);
	    LCDShow(2,0,"→");
	}
    while (1)
    {
		static unsigned char tmp_wendu,last_tmp,num;
	    KeyDriver();   //调用按键驱动函数
	
		TR0 = 0;
		P0 = 0xFF;
		ADC_WR = 0;
		DELAY();
		ADC_WR = 1;
		ADC_RD = 0;
		DELAY();
		last_tmp=tmp_wendu;
		tmp_wendu = P0;
		DELAY();
		ADC_RD = 1;
		tmp_wendu =(unsigned char)(tmp_wendu/5.6);

		if(tmp_wendu==last_tmp)
		{
			num++;
			if(num>5)
			{
				num=0;
				now_wendu=tmp_wendu;
			}
		}
		TR0 = 1;
	
		while(((now_wendu>=45)||(now_wendu<=10)) && trans)
		{

			EA = 0;
			trans=0;
			SBUF=01;
			while(!TI);
			TI=0;
			minute=0;second=0; count_Y=0;
			EA = 1;
			break;
		}


		test[1]= minute;
	    test[0]= second;
		test[2]= wendu;
		test[3]= count_Y;
	    I2CW24(test,0x01, 4);
		if(mode==3)
		{
		    test[0]= minute / 10 + '0';
		    test[1]= minute % 10 + '0';
		    test[2]=':';
		    test[3]= second / 10 + '0';
		    test[4]= second % 10 + '0';
		    test[5]='\0';
		    LCDShow(4,5, test);
			test[0]= now_wendu/ 10 + '0';
	   	 	test[1]= now_wendu% 10 + '0';
	    	test[2]='\0';
			LCDShow(2,5, test);
			LCDShow(2,6, "℃");
	   	 	test[0]=wendu / 10 + '0';
	   	 	test[1]=wendu % 10 + '0';
	    	test[2]='\0';
			LCDShow(3,5, test);
			LCDShow(3,6, "℃");
			if(minute || second)
			{
				if(now_wendu>wendu)
					LCDShow(1,5,"制冷");
				else if(now_wendu<wendu)
					LCDShow(1,5,"制热");
				else
					LCDShow(1,5,"平衡");
			}
			else
			   	LCDShow(1,5,"停止");
		}
    }
 }

void InterruptTimer0() interrupt 1
{
    static unsigned int count=450;  //时间计时

    unsigned char str[6];
	static unsigned char  i=10;
	static bit dir;
	TH0 = 0xF9;
	TL0 = 0xCD;
	if(minute ||second);
	else
		count_Y=0;
    if((count_Y | minute|second) && (now_wendu==wendu))
    {
        count++;
        if(count>=500)
        {
            count = 0;
            if(second==0)
                if(minute)
                {
                    second = 59;
                    minute--;
                }
                else
                    count_Y=0;
            else
                second--;
            
        }
   }
   KeyScan();
   if(count_Y)
   {
   if(i==0)
   {
   		static unsigned char k=0;
   		if(now_wendu>wendu)
		{	P3 = (P3 |0xf0) & FFW[k++];dir=1;}  
   		else if (now_wendu<wendu) 
		{	P3 = (P3 |0xF0) & FFW[k--];dir=0;}
		else
		{
			if(dir)
				P3 = (P3 |0xf0) & FFW[k++]	;
			else
				P3 = (P3 |0xf0) & FFW[k--]	;
		}
  		k=k&0x07;
		if (ABS(now_wendu,wendu)>=10)
			i=5;
		else
			i=15-(unsigned char)( ABS(now_wendu,wendu));
   }
   else
   		i--;  
   }
   if(trans==0)
   		ring=~ring;		
}


