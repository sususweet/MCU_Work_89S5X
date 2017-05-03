ORG 0000H
NUMPOI EQU 0
;使用40H位寄存器R0,R1,R3,将40H,30H开始的数码相除,结果存入30H
DIVIDE:
      MOV RS1,#0B
      MOV RS0,#1B
      MOV R0,#30H
      LCALL ISZERO
      MOV A,R4

      JNZ DIVERR
      JZ DIVNOERR
DIVERR:
       LCALL SHOWERR
       SJMP $
DIVNOERR:
      MOV RS1,#0B    ;商和余数寄存器初始化
      MOV RS0,#1B
      MOV R0,#48H
      LCALL CLEAR
      MOV R0,#60H
      LCALL CLEAR
      MOV R0,#70H
      LCALL CLEAR
      MOV 70H,#1

      ;MOV RS1,#0B
     ; MOV RS0,#1B
     ; MOV R7,#NUMPOI
RBITLOOP:
      ;MOV R0,#48H
      ;MOV R1,#47H
     ; MOV R2,#08H    ;移动次数
      ;LCALL RLCBIT
     ; DJNZ R7,RBITLOOP

;除法主程序
MOV RS1,#1B
MOV RS0,#0B
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
      MOV RS1,#1B
      MOV RS0,#1B
      MOV R0,#4FH
      MOV R1,#37H
      MOV R2,#08H
      LCALL GETMAX
	  ;POP A
      MOV A,R4
      JZ DIVCALCLOOP
      CJNE R5,#08H,DIVON
      LJMP DIVEND
DIVON:
      MOV RS1,#0B
      MOV RS0,#1B
      MOV R0,#50H
      MOV R1,#4FH
      MOV R2,#16
      LCALL RLCBIT
	  MOV R0,#68H
      MOV R1,#67H
      MOV R2,#8
      LCALL RLCBIT

      MOV RS1,#1B
      MOV RS0,#0B
      INC R5
	  ;交换过顺序，A变为1
      CJNE R5,#09H,DIVLESS
      LJMP DIVEND
      ;JNZ DIVIDELOOP
DIVCALCLOOP:
	  MOV RS1,#1B
      MOV RS0,#0B
      MOV R0,#48H
      MOV R1,#30H
      MOV R3,#08H        ;字节长度初始化
      LCALL MINUSFUN


      ;MOV A,R5
      ;MOV R3,A


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
     ; MOV A,#60H
     ; ADD A,#NUMPOI
     ; MOV R0,A
     ; MOV A,@R0
    ;  ADD A,#0F0H
    ;  MOV @R0,A

      MOV RS1,#0B
      MOV RS0,#1B
      MOV R0,#60H
      MOV R1,#30H
      LCALL MEMMOVE

      SJMP $



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

;R0为清空的开始位，CLEAR子程序将从R0开始的8位数码清空
CLEAR:
      MOV A,R0
      MOV R1,A
      MOV R2,#09H
  CLOOP:MOV @R1,#00H
        INC R1
        DJNZ R2,CLOOP
        RET

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
     ;MOV RS1,#1B
     ;MOV RS0,#0B
     ;MOV R0,#30H
     ;MOV R1,#40H
     MOV R4,#01H
     RET
   GET2:
     ;MOV RS1,#1B
     ;MOV RS0,#0B
     ;MOV R0,#40H
     ;MOV R1,#30H
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
        MOV RS1,#0B
        MOV RS0,#1B
        MOV R0,#70H
        MOV R1,#30H
        LCALL MEMMOVE
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
RLCDIV:
  DIVLOOP:
       MOV A,@R1
       MOV @R0,A
       DEC R0
       DEC R1
       DJNZ R2,DIVLOOP
       RET
RLCBIT:
  RLCLOOP:
       MOV A,@R1
       MOV @R0,A
       DEC R0
       DEC R1
       DJNZ R2,RLCLOOP
       MOV @R0,#00H
       RET
