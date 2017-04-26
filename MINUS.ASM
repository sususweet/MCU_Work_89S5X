ORG 0000H
;使用01位寄存器R0,R1,R3,将20H,10H开始的数码相减,结果存入20H
MINUS:
      MOV RS1,#1B
      MOV RS0,#0B
      MOV R0,#20H       ;十进制数存放地址初始化
      MOV R1,#10H
      MOV R3,#08H        ;字节长度初始化
      CLR C
MINUSLOOP:                 ;从40H开始循环遍历十进制数的各个数位
     MOV A,@R0
     SUBB A,@R1
     MOV @R0,A          ;暂存十进制数数位相加结果
                   ;清空CY，为接下来的判断做准备
     LJMP MINUSFIN
;SUBB A, #10H
     ;JNC MINUSFIX        ;判断数位相加是否大于等于10，若条件满足则进位，否则清空CY标志位
     ;CLR C
     LJMP MINUSFIN
MINUSFIX:
     MOV @R0,A          ;CY置位，为进位做准备
     SETB C
MINUSFIN:
     INC R0             ;循环遍历
     INC R1
     DJNZ R3,MINUSLOOP
     MOV RS1,#0B
     MOV RS0,#1B
     MOV R0,#20H
     MOV R1,#10H
     ;LCALL MEMMOVE
     MOV RS1,#0B
     MOV RS0,#0B
     SJMP $             ;程序结束
