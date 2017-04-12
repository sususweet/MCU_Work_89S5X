ORG 0000H
BEGIN:
  MOV R0,#31H   ;初始化带符号数的获取地址
  MOV R1,#0     ;初始化正数、负数、0个数的计数器
  MOV R2,#0
  MOV R3,#0
  MOV R4,30H    ;初始化字节个数
LOOP:
  CLR C         ;标志位CY置零
  MOV A,@R0
  INC R0
  JZ ZERO       ;若带符号数为0，则跳转到ZERO
  RLC A         ;A中内容左移到标志位，获取最高位数据
  JNC POSITIVE  ;若带符号数最高位为0，则跳转到POSITIVE
  JC NEGETIVE  ;若带符号数最高位为1，则跳转到NEGETIVE  
ZERO:           ;将相应情况的数字进行统计
  MOV A,R1
  INC A
  MOV R1,A
  DJNZ R4,LOOP
  LJMP FINAL
POSITIVE:
  MOV A,R2
  INC A
  MOV R2,A
  DJNZ R4,LOOP
  LJMP FINAL
NEGETIVE:
  MOV A,R3
  INC A
  MOV R3,A
  DJNZ R4,LOOP
  LJMP FINAL
FINAL:          ;统计结果存入指定存储单元
  MOV 22H,R1
  MOV 20H,R2
  MOV 21H,R3
SJMP $
