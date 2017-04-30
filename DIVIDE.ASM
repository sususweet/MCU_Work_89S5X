ORG 0000H
;ʹ��10Hλ�Ĵ���R0,R1,R3,��20H,10H��ʼ���������,�������20H
DIVIDE:
      MOV RS1,#0B    ;�̺������Ĵ�����ʼ��
      MOV RS0,#1B
      MOV R0,#40H
      LCALL CLEAR
      MOV R0,#30H
      LCALL CLEAR
      MOV 30H,#1
      
      MOV RS1,#0B
      MOV RS0,#1B
      MOV R7,#03
RBITLOOP:      
      MOV R0,#28H
      MOV R1,#27H
      MOV R2,#08H    ;�ƶ�����
      LCALL RLCBIT
      DJNZ R7,RBITLOOP
      
      MOV RS1,#0B
      MOV RS0,#0B

;      MOV RS1,#1B    ;��ʼ����
  ;    MOV RS0,#0B
  ;    MOV R0,#20H
  ;    MOV R1,#10H
  ;    MOV R3,#08H

  ;    LCALL MINUSFUN

DIVIDELOOP:
      MOV RS1,#1B
      MOV RS0,#0B

      MOV R0,#20H
      MOV R1,#10H
      MOV R3,#08H        ;�ֽڳ��ȳ�ʼ��
      LCALL MINUSFUN

      MOV R0,#40H
      MOV R1,#30H
      MOV R3,#08H
      LCALL PLUSFUN

      MOV RS1,#1B
      MOV RS0,#1B
      MOV R0,#27H
      MOV R1,#17H
      MOV R2,#08H
      LCALL GETMAX
      MOV A,R4

      JZ DIVIDELOOP
      
      MOV A,#40H
      ADD A,#3
      MOV R0,A
      MOV A,@R0
      ADD A,#0F0H
      MOV @R0,A
 
      MOV RS1,#0B
      MOV RS0,#1B
      MOV R0,#40H
      MOV R1,#20H
      LCALL MEMMOVE
      
      SJMP $

;ʹ��10Hλ�Ĵ���R0,R1,R3,��20H,10H��ʼ���������,�������10H
;PLUS:
;      MOV RS1,#1B
 ;     MOV RS0,#0B
 ;     MOV R0,#20H       ;ʮ��������ŵ�ַ��ʼ��
 ;     MOV R1,#10H
     ; LCALL PLUSFUN
 ;     MOV RS1,#0B
  ;    MOV RS0,#1B
    ;  MOV R0,#20H
   ;  MOV R1,#10H
  ;    LCALL MEMMOVE
   ;   LJMP BEGIN

;�ӷ������ӳ���ע�⣬ʹ��ǰ��Ҫ��ʼ��10Hλ�Ĵ���R0,R1,R3, R0�汻����,R1�����, �����R0��ʼ�ĵ�Ԫ
PLUSFUN:
      CLR C
PLUSLOOP:                 ;��40H��ʼѭ������ʮ�������ĸ�����λ
     MOV A,@R0
     ADDC A,@R1
     DA A               ;ʮ��������λ��ӣ����ж�-ʮ���Ƶ���
     MOV @R0,A          ;�ݴ�ʮ��������λ��ӽ��
     CLR C              ;���CY��Ϊ���������ж���׼��
     SUBB A, #10H
     JNC PLUSFIX        ;�ж���λ����Ƿ���ڵ���10���������������λ���������CY��־λ
     CLR C
     LJMP PLUSFIN
PLUSFIX:
     MOV @R0,A          ;CY��λ��Ϊ��λ��׼��
     SETB C
PLUSFIN:
     INC R0             ;ѭ������
     INC R1
     DJNZ R3,PLUSLOOP
     RET


;���������ӳ���ע�⣬ʹ��ǰ��Ҫ��ʼ��10Hλ�Ĵ���R0,R1,R3, R0�汻����, R1�����, R3���������
MINUSFUN:
     CLR C
MINUSLOOP:                 ;��40H��ʼѭ������ʮ�������ĸ�����λ
     MOV A,@R0
MINUSCON:
     SUBB A,@R1
     JC MINUSFIX
     MOV @R0,A          ;�ݴ�ʮ��������λ��ӽ��
     LJMP MINUSFIN
MINUSFIX:
     ADD A,#10
     MOV @R0,A
     LJMP MINUSFIN
MINUSFIN:
     INC R0             ;ѭ������
     INC R1
     DJNZ R3,MINUSLOOP
     RET

;MEMMOVE ��R0��ͷ�������ƶ���R1��ʼ�Ĵ洢��Ԫ�����ݹ�8λ
MEMMOVE:
        MOV R2,#09H
  MEMLOOP:
          MOV A,@R0
          MOV @R1,A
          INC R0
          INC R1
          DJNZ R2,MEMLOOP
          RET
;R0Ϊ��յĿ�ʼλ��CLEAR�ӳ��򽫴�R0��ʼ��8λ�������
CLEAR:
      MOV A,R0
      MOV R1,A
      MOV R2,#09H
  CLOOP:MOV @R1,#00H
        INC R1
        DJNZ R2,CLOOP
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
   GETFINAL:	;����Ǹ������򽻻����˳��
     MOV RS1,#1B
     MOV RS0,#0B
     MOV A,R0
     MOV B,R1
     MOV R0,B
     MOV R1,A
     MOV R4,#01H
     RET
   GET2:
     MOV RS1,#1B
     MOV RS0,#0B
     ;MOV R0,#20H
     ;MOV R1,#10H
     MOV R4,#00H
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