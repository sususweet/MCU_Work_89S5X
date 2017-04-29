ORG 0000H
;使用01位寄存器R0,R1,R3,将20H,10H开始的数码相减,结果存入20H
MINUS:
      MOV RS1,#1B
      MOV RS0,#0B
      MOV R0,#20H       ;十进制数存放地址初始化
      MOV R1,#10H
      MOV R3,#08H        ;字节长度初始化
      LCALL GETMAX
      CLR C
MINUSLOOP:                 ;从40H开始循环遍历十进制数的各个数位
     MOV A,@R0
MINUSCON:
     SUBB A,@R1
     JC MINUSFIX
     MOV @R0,A          ;暂存十进制数数位相加结果
                   ;清空CY，为接下来的判断做准备
     LJMP MINUSFIN
MINUSFIX:
     ADD A,#10
     MOV @R0,A
     LJMP MINUSFIN
;SUBB A, #10H
     ;JNC MINUSFIX        ;判断数位相加是否大于等于10，若条件满足则进位，否则清空CY标志位
     ;CLR C
     ;LJMP MINUSFIN
MINUSFIN:
     INC R0             ;循环遍历
     INC R1
     DJNZ R3,MINUSLOOP
     MOV A,R4
     JZ MEM1
     JNZ MEM2
MEM1:
     MOV RS1,#0B
     MOV RS0,#1B
     MOV R0,#20H
     MOV R1,#10H
     LCALL MEMMOVE
MEM2:
     MOV RS1,#0B
     MOV RS0,#0B
     MOV 17H,#17
     SJMP $             ;程序结束
GETMAX:
     MOV RS1,#1B
     MOV RS0,#1B
     MOV R0,#27H
     MOV R1,#17H
     MOV R2,#08H
   GETLOOP:
     MOV A,@R0
     CLR C
     SUBB A,@R1
     JC GETFINAL
     DEC R0
     DEC R1
     DJNZ R2,GETLOOP
   GETFINAL:
     JNC FINAL 
     MOV RS1,#1B
     MOV RS0,#0B
     MOV R0,#10H
     MOV R1,#20H
     MOV R4,#01H
     RET
   FINAL:
     MOV RS1,#1B
     MOV RS0,#0B
     MOV R4,#00H
     RET
 ;MEMMOVE 将R0开头的数据移动到R1开始的存储单元，数据共8位
MEMMOVE:
        MOV R2,#09H
  MEMLOOP:
          MOV A,@R0
          MOV @R1,A
          INC R0
          INC R1
          DJNZ R2,MEMLOOP
          RET
