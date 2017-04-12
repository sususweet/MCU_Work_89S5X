ORG 0000H
BEGIN:                  ;程序的初始化，变量赋值
      MOV 20H,#0
      MOV 21H,#0
      MOV R0,#8
LOOP:
     MOV A,R1
     RLC A              ;A中高位移入CY标志位，实现R1数据按位读取
     MOV R1,A
     MOV A,20H
     ADDC A,ACC         ;ADDC加上CY，实现转换过程中的乘2加1要求
     DA A               ;累加结果进行二-十进制转换
     MOV 20H,A          ;累加低位送20H保存
     MOV A,21H          ;进制转换高位处理
     ADDC A,ACC         ;ADDC加上CY，处理低位的进位
     MOV 21H,A
     DJNZ R0,LOOP       ;8位二进制处理完成的判断
     SJMP $
