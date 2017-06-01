#include <reg51.h>       //51оƬ�ܽŶ���ͷ�ļ�
#include <intrins.h>

#define I2CDelay()  {_nop_();_nop_();_nop_();_nop_();}
sbit I2C_SCL = P3^3;			//WR
sbit I2C_SDA = P3^2;			//RD


/* ����������ʼ�ź� */
void I2CStart()
{
    I2C_SDA = 1; //����ȷ��SDA��SCL���Ǹߵ�ƽ
    I2C_SCL = 1;
    I2CDelay();
    I2C_SDA = 0; //������SDA
    I2CDelay();
    I2C_SCL = 0; //������SCL
}
/* ��������ֹͣ�ź� */
void I2CStop()
{
     //����ȷ��SDA��SCL���ǵ͵�ƽ
    I2C_SDA = 0;
    I2CDelay();
    I2C_SCL = 1; //������SCL
    I2CDelay();
    I2C_SDA = 1; //������SDA
    I2CDelay();
    I2C_SCL = 0;
}

void ACKN()
{
    I2C_SDA = 1;   //8λ���ݷ�����������ͷ�SDA���Լ��ӻ�Ӧ��
    I2CDelay();
    I2C_SCL = 1;   //����SCL
    I2CDelay();
    I2CDelay();
    while(I2C_SDA); //��ȡ��ʱ��SDAֵ����Ϊ�ӻ���Ӧ��ֵ

    I2C_SCL = 0;   //������SCL���Ӧ��λ��������ס����

}
void ACKS()
{
    I2C_SDA = 0;   //8λ���ݷ����������SDA������Ӧ���ź�
    I2CDelay();
    I2C_SCL = 1;   //����SCL
    I2CDelay();
    I2C_SCL = 0;   //������SCL���Ӧ��λ��������ס����
    I2C_SDA = 1;
}
/* I2C����д������dat-��д���ֽڣ�����ֵ-�ӻ�Ӧ��λ��ֵ */
void I2CWrite(unsigned char dat)
{
    unsigned char mask;  //����̽���ֽ���ĳһλֵ���������

    for (mask=0x80; mask!=0; mask>>=1) //�Ӹ�λ����λ���ν���
    {
        if ((mask&dat) == 0)  //��λ��ֵ�����SDA��
            I2C_SDA = 0;
        else
            I2C_SDA = 1;
        I2CDelay();
        I2C_SCL = 1;          //����SCL
        I2CDelay();
        I2C_SCL = 0;          //������SCL�����һ��λ����
    }
}
/* I2C���߶������������ͷ�Ӧ���źţ�����ֵ-�������ֽ� */
unsigned char I2CRead()
{
    unsigned char mask;
    unsigned char dat;

    I2C_SDA = 1;  //����ȷ�������ͷ�SDA
    for (mask=0x80; mask!=0; mask>>=1) //�Ӹ�λ����λ���ν���
    {
        I2CDelay();
        I2C_SCL = 1;      //����SCL
        if(I2C_SDA == 0)  //��ȡSDA��ֵ
            dat &= ~mask; //Ϊ0ʱ��dat�ж�Ӧλ����
        else
            dat |= mask;  //Ϊ1ʱ��dat�ж�Ӧλ��1
        I2CDelay();
        I2C_SCL = 0;      //������SCL����ʹ�ӻ����ͳ���һλ
    }
    return dat;
}


/* E2��ȡ������buf-���ݽ���ָ�룬addr-E2�е���ʼ��ַ��len-��ȡ���� */
void I2CR24(unsigned char *buf, unsigned char addr, unsigned char len)
{
    I2CStart();
    I2CWrite(0x50<<1); //Ӧ��������ѭ������Ӧ���������һ�β�ѯ
    ACKN();
    I2CWrite(addr);            //д����ʼ��ַ
    ACKN();                //�����ظ������ź�
    I2CStart();
    I2CWrite((0x50<<1)|0x01);  //Ѱַ����������Ϊ������
    ACKN();
    while (len > 1)            //������ȡlen-1���ֽ�
    {
        *buf++ = I2CRead(); //����ֽ�֮ǰΪ��ȡ����+Ӧ��
        len--;
        ACKS();
    }
    *buf = I2CRead();       //���һ���ֽ�Ϊ��ȡ����+��Ӧ��
    I2C_SDA = 1;   //8λ���ݷ����������SDA�����ͷ�Ӧ���ź�
    I2CDelay();
    I2C_SCL = 1;   //����SCL
    I2CDelay();
    I2C_SCL = 0;   //������SCL���Ӧ��λ��������ס����
    I2C_SDA = 0;
    I2CStop();
}

void I2CW24(unsigned char *buf, unsigned char addr, unsigned char len)
{
    I2CStart();
	I2CWrite(0x50<<1); //Ӧ��������ѭ������Ӧ���������һ�β�ѯ
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
