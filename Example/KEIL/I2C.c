#include <reg51.h>       //51芯片管脚定义头文件
#include <intrins.h>

#define I2CDelay()  {_nop_();_nop_();_nop_();_nop_();}
sbit I2C_SCL = P3^3;			//WR
sbit I2C_SDA = P3^2;			//RD


/* 产生总线起始信号 */
void I2CStart()
{
    I2C_SDA = 1; //首先确保SDA、SCL都是高电平
    I2C_SCL = 1;
    I2CDelay();
    I2C_SDA = 0; //先拉低SDA
    I2CDelay();
    I2C_SCL = 0; //再拉低SCL
}
/* 产生总线停止信号 */
void I2CStop()
{
     //首先确保SDA、SCL都是低电平
    I2C_SDA = 0;
    I2CDelay();
    I2C_SCL = 1; //先拉高SCL
    I2CDelay();
    I2C_SDA = 1; //再拉高SDA
    I2CDelay();
    I2C_SCL = 0;
}

void ACKN()
{
    I2C_SDA = 1;   //8位数据发送完后，主机释放SDA，以检测从机应答
    I2CDelay();
    I2C_SCL = 1;   //拉高SCL
    I2CDelay();
    I2CDelay();
    while(I2C_SDA); //读取此时的SDA值，即为从机的应答值

    I2C_SCL = 0;   //再拉低SCL完成应答位，并保持住总线

}
void ACKS()
{
    I2C_SDA = 0;   //8位数据发送完后，拉低SDA，发送应答信号
    I2CDelay();
    I2C_SCL = 1;   //拉高SCL
    I2CDelay();
    I2C_SCL = 0;   //再拉低SCL完成应答位，并保持住总线
    I2C_SDA = 1;
}
/* I2C总线写操作，dat-待写入字节，返回值-从机应答位的值 */
void I2CWrite(unsigned char dat)
{
    unsigned char mask;  //用于探测字节内某一位值的掩码变量

    for (mask=0x80; mask!=0; mask>>=1) //从高位到低位依次进行
    {
        if ((mask&dat) == 0)  //该位的值输出到SDA上
            I2C_SDA = 0;
        else
            I2C_SDA = 1;
        I2CDelay();
        I2C_SCL = 1;          //拉高SCL
        I2CDelay();
        I2C_SCL = 0;          //再拉低SCL，完成一个位周期
    }
}
/* I2C总线读操作，并发送非应答信号，返回值-读到的字节 */
unsigned char I2CRead()
{
    unsigned char mask;
    unsigned char dat;

    I2C_SDA = 1;  //首先确保主机释放SDA
    for (mask=0x80; mask!=0; mask>>=1) //从高位到低位依次进行
    {
        I2CDelay();
        I2C_SCL = 1;      //拉高SCL
        if(I2C_SDA == 0)  //读取SDA的值
            dat &= ~mask; //为0时，dat中对应位清零
        else
            dat |= mask;  //为1时，dat中对应位置1
        I2CDelay();
        I2C_SCL = 0;      //再拉低SCL，以使从机发送出下一位
    }
    return dat;
}


/* E2读取函数，buf-数据接收指针，addr-E2中的起始地址，len-读取长度 */
void I2CR24(unsigned char *buf, unsigned char addr, unsigned char len)
{
    I2CStart();
    I2CWrite(0x50<<1); //应答则跳出循环，非应答则进行下一次查询
    ACKN();
    I2CWrite(addr);            //写入起始地址
    ACKN();                //发送重复启动信号
    I2CStart();
    I2CWrite((0x50<<1)|0x01);  //寻址器件，后续为读操作
    ACKN();
    while (len > 1)            //连续读取len-1个字节
    {
        *buf++ = I2CRead(); //最后字节之前为读取操作+应答
        len--;
        ACKS();
    }
    *buf = I2CRead();       //最后一个字节为读取操作+非应答
    I2C_SDA = 1;   //8位数据发送完后，拉低SDA，发送非应答信号
    I2CDelay();
    I2C_SCL = 1;   //拉高SCL
    I2CDelay();
    I2C_SCL = 0;   //再拉低SCL完成应答位，并保持住总线
    I2C_SDA = 0;
    I2CStop();
}

void I2CW24(unsigned char *buf, unsigned char addr, unsigned char len)
{
    I2CStart();
	I2CWrite(0x50<<1); //应答则跳出循环，非应答则进行下一次查询
    ACKN();
    I2CWrite(addr);
    ACKN();
	while(len > 0)
	{
		I2CWrite(*buf++);
		len--;
		ACKN();
	}
    I2CStop();
}
