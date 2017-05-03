ORG 0000H
;LJMP START
NUM1 EQU 30H
NUM2 EQU 40H
NUMPOI EQU 2
MOV IE,#00000000B
MOV SP,#0E0H
MOV R0,#28H
LCALL CLEAR
MOV R0,#NUM1
LCALL CLEAR
MOV R0,#NUM2
LCALL CLEAR
BEGIN:
SETB F0
CLR RS1
CLR RS0
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
    LCALL DEL20MS
    JNB P1.3, HANG1
    MOV R5,#01H
    LJMP KEY_END
HANG2:
    LCALL DEL20MS
    JNB P1.2, HANG2
    MOV R5,#02H
    LJMP KEY_END
HANG3:
    LCALL DEL20MS
    JNB P1.1, HANG3
    MOV R5,#03H
    LJMP KEY_END
HANG4:
    LCALL DEL20MS
    JNB P1.0, HANG4
    MOV R5,#04H
    LJMP KEY_END
KEY_END:
    JNB F0,KEYOLD
    ;MOV R0,#NUM1
    ;LCALL CLEAR
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

    CJNE A,#0EH,KEY_OPER
    ;CLR RS1
    ;CLR RS0
    ;MOV R0,#NUM1
    CLR RS1
    SETB RS0
    MOV R0,#31H
    MOV R1,#30H
    MOV R2,#08H
    LCALL RRCBIT
    CLR RS1
    CLR RS0
    ;LCALL CLEAR
    LJMP KEYEND
KEY_OPER:
    CJNE A,#0FH,KEY_NEXT
    LJMP CAL
KEY_NEXT:
    CLR C
    CJNE A,#09H,ISOPER
ISNUM:
    PUSH A

    CLR RS1
    SETB RS0
    MOV R0,#38H
    MOV R1,#37H
    MOV R2,#08H
    LCALL RLCBIT
    CLR RS1
    CLR RS0

    POP A
    MOV NUM1,A   ;MOV 20H,A
    INC R0
    CJNE R0,#19H,KEYEND
    MOV R0,#NUM1
    LJMP KEYEND
ISOPER:
    JC ISNUM
    CLR C
    SUBB A,#10
    PUSH A

    CLR RS1
    SETB RS0
    MOV R0,#30H
    MOV R1,#40H
    LCALL MEMMOVE
    CLR RS1
    CLR RS0
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
MOV R1,#37H
MOV R3,#00H
MOV R2,#09H     ;数码管需要显示的位数+1
CLR F1
CLR C
MOV A,37H
CJNE A,#0FH,JUDERR
LJMP NOERR
JUDERR:
    JZ NOERR
    JNC NOERR
    LCALL SHOWERR
    SETB F0
    CLR RS1
    CLR RS0
    RET
LEDLOOP:
NOERR:
    MOV DPTR,#TND       ;获取将要点亮的数码管段
    MOV A,@R1
    JZ JUGZERO
    CJNE A,#0EFH,SHOWPOINT
SHOWON:
    CJNE A,#17,NOTSHOWZ
    MOVC A,@A+DPTR
    MOV P3,A
    LJMP SHOWWEI
SHOWPOINT:
    JC SHOWON
    SETB F1
    CLR C
    SUBB A,#0F0H
    MOV DPTR,#TD
    MOVC A,@A+DPTR
    MOV P3,A
    LJMP SHOWWEI
NOTSHOWZ:
    MOVC A,@A+DPTR
    MOV P3,A
    SETB F1
    LJMP SHOWWEI
JUGZERO:
   ; CLR RS1
   ; SETB RS0
   ; MOV R0,#30H
   ; LCALL ISZERO
   ; MOV A,R4
    ;CLR RS1
   ; CLR RS0
    ;JNZ ONEZERO

    JB F1,SHOWZERO

    ;DEC R1
    ;MOV A,@R1
    ;INC R1
    ;JNZ SHOWZERO
    MOV P3,#0FFH
    LJMP SHOWWEI
SHOWZERO:
    MOV A,#00H
    MOVC A,@A+DPTR
    MOV P3,A
SHOWWEI:
    MOV DPTR,#TWEI      ;获取将要点亮的数码管位信息
    MOV A,R3
    MOVC A,@A+DPTR
    MOV P2,A

    DEC R1              ;数码管段和位偏移量自增
    INC R3
    DJNZ R2,GOON        ;判断是否完成对8位数码的完整扫描,如果未完成则继续GOON
    JNB F1,ONEZERO
    RET
ONEZERO:
    MOV DPTR,#TND
    MOV A,#00H
    MOVC A,@A+DPTR
    MOV P3,A
    LCALL DELAY
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
DEL20MS:
       mov r6,#100
       d4:mov r7,#100
       djnz r7,$
       djnz r6,d4
       ret
RLCBIT:
  RLCLOOP:
       MOV A,@R1
       MOV @R0,A
       DEC R0
       DEC R1
       DJNZ R2,RLCLOOP
       MOV @R0,#00H
       RET
RRCBIT:
  RRCLOOP:
       MOV A,@R0
       MOV @R1,A
       INC R0
       INC R1
       DJNZ R2,RRCLOOP
       MOV @R0,#00H
       RET

CAL:
    POP A
    CJNE A,#04H,CALJUDG
CALJUDG:
    MOV R7,A
    JNC CALCERR
    RL A
    ADD A,R7
    MOV DPTR,#CALC
    JMP @A+DPTR
CALCERR:
    PUSH A
    LJMP BEGIN
CALC:
    LJMP PLUS
    LJMP MINUS
    LJMP MULTI
    LJMP DIVIDE
    SJMP $

;使用30H位寄存器R0,R1,R3,将40H,30H开始的数码相加,结果存入30H
PLUS:
      SETB RS1
      CLR RS0
      MOV R0,#40H       ;十进制数存放地址初始化
      MOV R1,#30H
      MOV R3,#08H        ;字节长度初始化
      LCALL PLUSFUN
      CLR RS1
      SETB RS0
      MOV R0,#40H
      MOV R1,#30H
      LCALL MEMMOVE
      LJMP BEGIN

;加法计算子程序，注意，使用前需要初始化30H位寄存器R0,R1,R3, R0存被加数,R1存加数, 结果存R0开始的单元
PLUSFUN:
      CLR C
PLUSLOOP:                 ;从50H开始循环遍历十进制数的各个数位
     MOV A,@R0
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
     RET

;使用30H位寄存器R0,R1,R3,将40H,30H开始的数码相减,结果存入30H
MINUS:
     SETB RS1
     CLR RS0
     MOV R0,#40H       ;十进制数存放地址初始化
     MOV R1,#30H
     MOV R3,#08H        ;字节长度初始化

     SETB RS1
     SETB RS0
     MOV R0,#47H
     MOV R1,#37H
     MOV R2,#08H
     LCALL GETMAX
     MOV A,R4
     JNZ DOSWAP
     JZ NOSWAP
DOSWAP:
     SETB RS1
     CLR RS0
     MOV R0,#30H
     MOV R1,#40H
     LJMP MINUSCAL
NOSWAP:
     SETB RS1
     CLR RS0
     MOV R0,#40H
     MOV R1,#30H
     LJMP MINUSCAL
MINUSCAL:
     LCALL MINUSFUN
     SETB RS1
     SETB RS0
     MOV A,R4
     JZ MEM1
     JNZ MEM2
MEM1:
     CLR RS1
     SETB RS0
     MOV R0,#40H
     MOV R1,#30H
     LCALL MEMMOVE
     LJMP BEGIN
MEM2:
     MOV 37H,#17
     LJMP BEGIN             ;程序结束

;减法计算子程序，注意，使用前需要初始化30H位寄存器R0,R1,R3, R0存被减数, R1存减数, R3存数码个数
MINUSFUN:
     CLR C
MINUSLOOP:                 ;从50H开始循环遍历十进制数的各个数位
     MOV A,@R0
MINUSCON:
     SUBB A,@R1
     JC MINUSFIX
     MOV @R0,A          ;暂存十进制数数位相加结果
     LJMP MINUSFIN
MINUSFIX:
     ADD A,#10
     MOV @R0,A
     LJMP MINUSFIN
MINUSFIN:
     INC R0             ;循环遍历
     INC R1
     DJNZ R3,MINUSLOOP
     RET

;使用30H位寄存器R0,R1,R3,将40H,30H开始的数码相乘,结果存入30H
MULTI:
      SETB RS1
      CLR RS0
      MOV R0,#40H
      MOV R1,#30H
      MOV R3,#08H

      SETB RS1
      SETB RS0
      MOV R0,#47H
      MOV R1,#37H
      MOV R2,#08H
      LCALL GETMAX
      MOV A,R4
      JNZ MULSWAP
      JZ MULGOON
MULSWAP:
      CLR RS1
      SETB RS0
      MOV R0,#30H
      MOV R1,#50H
      LCALL MEMMOVE
      MOV R0,#40H
      MOV R1,#30H
      LCALL MEMMOVE
      MOV R0,#50H
      MOV R1,#40H
      LCALL MEMMOVE
MULGOON:
      CLR RS1
      SETB RS0
      MOV R0,#30H
      MOV R1,#50H
      LCALL MEMMOVE
      MOV R0,#40H
      MOV R1,#30H
      LCALL MEMMOVE
      MOV R0,#60H
      LCALL CLEAR
      MOV 60H,#1
      MOV R0,#40H
      LCALL CLEAR

      ;SETB RS1
      ;CLR RS0

      ;MOV R0,#50H
      ;MOV R1,#60H
      ;MOV R3,#08H        ;字节长度初始化
      ;LCALL MINUSFUN
MULTILOOP:
      CLR RS1
      SETB RS0
      MOV R0,#50H
      LCALL ISZERO
      MOV A,R4
      JNZ MINFIN

      SETB RS1
      CLR RS0
      MOV R0,#40H
      MOV R1,#30H
      MOV R3,#08H
      LCALL PLUSFUN

      MOV R0,#50H
      MOV R1,#60H
      MOV R3,#08H        ;字节长度初始化
      LCALL MINUSFUN

      LJMP MULTILOOP
MINFIN:
      CLR RS1
      SETB RS0
      MOV R0,#40H
      MOV R1,#30H
      LCALL MEMMOVE
      LJMP BEGIN

;使用40H位寄存器R0,R1,R3,将40H,30H开始的数码相除,结果存入30H
DIVIDE:
      CLR RS1
      SETB RS0
      MOV R0,#30H
      LCALL ISZERO
      MOV A,R4

      JNZ DIVERR
      JZ DIVNOERR
DIVERR:
       LCALL SHOWERR
       LJMP BEGIN
DIVNOERR:
      CLR RS1    ;商和余数寄存器初始化
      SETB RS0
      MOV R0,#48H
      LCALL CLEAR
      MOV R0,#60H
      LCALL CLEAR
      MOV R0,#70H
      LCALL CLEAR
      MOV 70H,#1

      CLR RS1
      SETB RS0
      MOV R7,#NUMPOI
RBITLOOP:
      MOV R0,#48H
      MOV R1,#47H
      MOV R2,#08H    ;移动次数
      LCALL RLCBIT
      DJNZ R7,RBITLOOP

;除法主程序
SETB RS1
CLR RS0
MOV R5,#00H
DIVIDELOOP:

      ;MOV R0,#40H
      ;MOV R1,#30H
      ;MOV R3,#08H        ;字节长度初始化
      ;LCALL MINUSFUN

      ;MOV R0,#50H
      ;MOV R1,#70H
     ; MOV R3,#08H
     ; LCALL PLUSFUN
	  ;MOV A,R5
	  ;PUSH A


	  ;MOV R0,#48H
     ; MOV R1,#47H
    ;  MOV R2,#08H    ;移动次数
    ;  LCALL RLCBIT
DIVLESS:
      SETB RS1
      SETB RS0
      MOV R0,#4FH
      MOV R1,#37H
      MOV R2,#08H
      LCALL GETMAX
	  ;POP A
      MOV A,R4
      JZ DIVCALCLOOP
      SETB RS1
      CLR RS0
      CJNE R5,#08H,DIVON
      LJMP DIVEND
DIVON:
      CLR RS1
      SETB RS0
      MOV R0,#50H
      MOV R1,#4FH
      MOV R2,#16
      LCALL RLCBIT
      MOV R0,#68H
      MOV R1,#67H
      MOV R2,#8
      LCALL RLCBIT

      SETB RS1
      CLR RS0
      INC R5
      ;交换过顺序，A变为1
      CJNE R5,#09H,DIVLESS
      LJMP DIVEND
      ;JNZ DIVIDELOOP
DIVCALCLOOP:
      SETB RS1
      CLR RS0
      MOV R0,#48H
      MOV R1,#30H
      MOV R3,#08H        ;字节长度初始化
      LCALL MINUSFUN

      MOV R0,#60H
      MOV R1,#70H
      MOV R3,#08H
      LCALL PLUSFUN

      LJMP DIVLESS

	  ;MOV A,R0
	 ; ADD A,R5
	;  MOV R0,A
	;  MOV A,R1
	;  ADD A,R5
	;  MOV R1,A

	 ; CJNE R5,#08H,DIVLESS
DIVEND:
      MOV A,#60H
      ADD A,#NUMPOI
      MOV R0,A
      MOV A,@R0
      ADD A,#0F0H
      MOV @R0,A

      CLR RS1
      SETB RS0
      MOV R0,#60H
      MOV R1,#30H
      LCALL MEMMOVE

      LJMP BEGIN

GETMAX:
   GETLOOP:
     MOV A,@R0
     CLR C
     SUBB A,@R1
     JC GETFINAL
     JNZ GET2
     DEC R0
     DEC R1
     DJNZ R2,GETLOOP
     LJMP GET2
   GETFINAL:	;如果是负数，则交换相减顺序
     MOV R4,#01H
     RET
   GET2:
     MOV R4,#00H
     RET
ISZERO:
      MOV A,R0
      MOV R1,A
      MOV R2,#08H
  ZEROLOOP:MOV A,@R1
           DEC R2
           INC R1
           CJNE R2,#00H,ZERO2
           MOV R4,#01H
           RET
	ZERO2:
           JZ ZEROLOOP
           MOV R4,#00H
           RET
SHOWERR:
        MOV R1,#70H
        MOV R2,#08H
  ERRLOOP:
        MOV @R1,#0FH
        INC R1
        DJNZ R2,ERRLOOP
        CLR RS1
        SETB RS0
        MOV R0,#70H
        MOV R1,#30H
        LCALL MEMMOVE
        CLR RS1
        CLR RS0
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

;AT89C51数码管0-9,A-F编码表,不含小数点,P3口输出
TND: DB 28H,7EH,0A2H,62H,74H,61H,21H,7AH,20H,60H,30H,25H,0A9H,26H,0A1H,0B1H,0FFH,0F7H
;AT89C51数码管位选编码表
TWEI: DB 0FEH,0FDH,0FBH,0F7H,0EFH,0DFH,0BFH,07FH
TWEID: DB 07FH,0BFH,0DFH,0EFH,0F7H,0FBH,0FDH,0FEH
;AT89C51数码管0-9,A-F编码表,含小数点,P3口输出
TD: DB 08H,5EH,82H,42H,54H,41H,01H,5AH,00H,40H,10H,05H,89H,06H,81H,91H
