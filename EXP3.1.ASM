ORG 0000H
BEGIN:
      MOV A,30H    ;第一个BCD码地址内容送入寄存器A
      SWAP A       ;A的低四位和高四位交换
      XCH A,31H    ;第二个BCD码地址内容和寄存器A内容交换
      SWAP A       ;A的低四位和高四位交换
      MOV 30H,A    ;寄存器A内容送入第一个BCD码地址
SJMP $
