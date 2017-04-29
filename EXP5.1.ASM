ORG 0000H
;LJMP START
NUM1 EQU 10H
NUM2 EQU 20H
MOV SP,#30H
MOV R0,#NUM1
LCALL CLEAR
BEGIN:
SETB F0
MOV RS1,#0B
MOV RS0,#0B
KEY_SCAN: ;键盘扫描使用P1口,P1.4-P1.7为列扫描,P1.0-P1.3为行扫描
    MOV P1,#0F0H
    JNB P1.4, LIE4
    JNB P1.5, LIE3
    JNB P1.6, LIE2
    JNB P1.7, LIE1
    LCALL DISP_LED
    SJMP KEY_SCAN
LIE1:
    ;LCALL DEL10MS
    MOV R4,#01H
    LJMP JUDGELIE
LIE2:
    MOV R4,#02H
    LJMP JUDGELIE
LIE3:
    MOV R4,#03H
    LJMP JUDGELIE
LIE4:
    MOV R4,#04H
    LJMP JUDGELIE
JUDGELIE:
    MOV P1,#0FH
    JNB P1.0, HANG4
    JNB P1.1, HANG3
    JNB P1.2, HANG2
    JNB P1.3, HANG1
    LJMP KEY_SCAN
HANG1:
    LCALL DEL10MS
    JNB P1.3, HANG1
    MOV R5,#01H
    LJMP KEY_END
HANG2:
    LCALL DEL10MS
    JNB P1.2, HANG2
    MOV R5,#02H
    LJMP KEY_END
HANG3:
    LCALL DEL10MS
    JNB P1.1, HANG3
    MOV R5,#03H
    LJMP KEY_END
HANG4:
    LCALL DEL10MS
    JNB P1.0, HANG4
    MOV R5,#04H
    LJMP KEY_END
KEY_END:
    JNB F0,KEYOLD
    MOV R0,#NUM1
    LCALL CLEAR
    CLR F0
  KEYOLD:
    MOV A,R5
    DEC A
    MOV B,#04H
    MUL AB
    ADD A,R4
    DEC A                ;计算键值在表中的偏移量
    MOV DPTR,#KEY_TABLE  ;查表获得实际按键
    MOVC A,@A+DPTR

    CJNE A,#0FH,KEY_NEXT
    LJMP CAL
KEY_NEXT:
    CLR C
    CJNE A,#09H,ISOPER
ISNUM:
    PUSH A
    LCALL RLCBIT
    POP A
    MOV @R0,A   ;MOV 20H,A
    INC R0
    CJNE R0,#19H,KEYEND
    MOV R0,#NUM1
    LJMP KEYEND
ISOPER:
    JC ISNUM
    CLR C
    SUBB A,#10
    PUSH A

    MOV RS1,#0B
    MOV RS0,#1B
    MOV R0,#10H
    MOV R1,#20H
    LCALL MEMMOVE
    MOV RS1,#0B
    MOV RS0,#0B
    MOV R0,#NUM1
    LCALL CLEAR


    LJMP KEYEND
KEYEND:
    LJMP KEY_SCAN
KEY_TABLE:    ;0AH:+  0BH:-  0CH:x  0DH:/  0EH:*  0FH:#
    DB 01H,02H,03H,0AH,04H,05H,06H,0BH,07H,08H,09H,0CH,0EH,00H,0FH,0DH


;R0为清空的开始位，CLEAR子程序将从R0开始的8位数码清空
CLEAR:
      MOV A,R0
      MOV R1,A
      MOV R2,#09H
  CLOOP:MOV @R1,#00H
        INC R1
        DJNZ R2,CLOOP
        RET

DISP_LED:
;;MOV 20H,#01H    ;给20H到27H的存储空间赋初值
;MOV 21H,#02H
;MOV 22H,#03H
;MOV 23H,#04H
;MOV 24H,#05H
;MOV 25H,#06H
;MOV 26H,#07H
;MOV 27H,#08H
MOV R1,#10H
MOV R3,#00H
MOV R2,#09H     ;数码管需要显示的位数+1
CLR F1
LEDLOOP:
    MOV DPTR,#TND       ;获取将要点亮的数码管段
    MOV A,@R1
    JZ JUGZERO
    MOVC A,@A+DPTR
    MOV P0,A
    SETB F1
    LJMP SHOWWEI
JUGZERO:
    JNB F1,SHOWZERO
    INC R1
    MOV A,@R1
    DEC R1
    JNZ SHOWZERO
    MOV P0,#0FFH
    LJMP SHOWWEI
SHOWZERO:
    MOV A,#00H
    MOVC A,@A+DPTR
    MOV P0,A
SHOWWEI:
    MOV DPTR,#TWEID      ;获取将要点亮的数码管位信息
    MOV A,R3
    MOVC A,@A+DPTR
    MOV P2,A

    INC R1              ;数码管段和位偏移量自增
    INC R3
    DJNZ R2,GOON        ;判断是否完成对8位数码的完整扫描,如果未完成则继续GOON
    RET
    ;MOV R0,#20H         ;如果已经完成了完整扫描,则重新开始扫描
    ;;MOV R1,#00H
    ;MOV R2,#08H
GOON:
    LCALL DELAY         ;调用DELAY函数配合视觉暂留,实现多位数码管同时显示
    AJMP LEDLOOP
DELAY:
       mov r5,#20
       d1:mov r6,#10
       d2:mov r7,#5
       djnz r7,$
       djnz r6,d2
       djnz r5,d1
       ret
DEL10MS:
       mov r5,#20
       d3:mov r6,#20
       d4:mov r7,#10
       djnz r7,$
       djnz r6,d4
       djnz r5,d3
       ret
RLCBIT:
       MOV RS1,#0B
       MOV RS0,#1B
       MOV R0,#18H
       MOV R1,#17H
  RLCLOOP:
       MOV A,@R1
       MOV @R0,A
       DEC R0
       DEC R1
       CJNE R1,#0FH,RLCLOOP
       MOV 10H,#10H
       MOV RS1,#0B
       MOV RS0,#0B
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

CAL:
    POP A
    RL A
    ADD A,ACC
    MOV DPTR,#CALC
    JMP @A+DPTR
CALC:
    LJMP PLUS
    LJMP MINUS
    SJMP $

;使用01位寄存器R0,R1,R3,将20H,10H开始的数码相加,结果存入20H
PLUS:
      MOV RS1,#1B
      MOV RS0,#0B
      MOV R0,#20H       ;十进制数存放地址初始化
      MOV R1,#10H
      MOV R3,#08H        ;字节长度初始化
      CLR C
PLUSLOOP:                 ;从40H开始循环遍历十进制数的各个数位
     ;CJNE R0,#10H,PLUS1Z
     ;MOV A,#00H
PLUS1Z:
     MOV A,@R0
     ;CJNE R1,#10H,PLUS2Z
     ;ADDC A,#00H
PLUS2Z:
     ADDC A,@R1
     DA A               ;十进制数数位相加，进行二-十进制调整
     MOV @R0,A          ;暂存十进制数数位相加结果
     CLR C              ;清空CY，为接下来的判断做准备
     SUBB A, #10H
     JNC PLUSFIX        ;判断数位相加是否大于等于10，若条件满足则进位，否则清空CY标志位
     CLR C
     LJMP PLUSFIN
PLUSFIX:
     MOV @R0,A          ;CY置位，为进位做准备
     SETB C
PLUSFIN:
     INC R0             ;循环遍历
     INC R1
     DJNZ R3,PLUSLOOP
     MOV RS1,#0B
     MOV RS0,#1B
     MOV R0,#20H
     MOV R1,#10H
     LCALL MEMMOVE
     MOV RS1,#0B
     MOV RS0,#0B
     LJMP BEGIN

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
     LJMP BEGIN      ;减法程序结束
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


;AT89C51数码管0-9,A-F编码表,不含小数点,P0口输出
TND: DB 28H,7EH,0A2H,62H,74H,61H,21H,7AH,20H,60H,30H,25H,0A9H,26H,0A1H,0B1H,0FFH,08H
;AT89C51数码管位选编码表
TWEI: DB 0FEH,0FDH,0FBH,0F7H,0EFH,0DFH,0BFH,07FH
TWEID: DB 07FH,0BFH,0DFH,0EFH,0F7H,0FBH,0FDH,0FEH
;AT89C51数码管0-9,A-F编码表,含小数点,P0口输出
TD: DB 08H,5EH,82H,42H,54H,41H,01H,5AH,00H,40H,10H,05H,89H,06H,81H,91H
