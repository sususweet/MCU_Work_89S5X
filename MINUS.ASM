ORG 0000H
;ʹ��01λ�Ĵ���R0,R1,R3,��20H,10H��ʼ���������,�������20H
MINUS:
      MOV RS1,#1B
      MOV RS0,#0B
      MOV R0,#20H       ;ʮ��������ŵ�ַ��ʼ��
      MOV R1,#10H
      MOV R3,#08H        ;�ֽڳ��ȳ�ʼ��
      LCALL GETMAX
      CLR C
MINUSLOOP:                 ;��40H��ʼѭ������ʮ�������ĸ�����λ
     MOV A,@R0
MINUSCON:
     SUBB A,@R1
     JC MINUSFIX
     MOV @R0,A          ;�ݴ�ʮ��������λ��ӽ��
                   ;���CY��Ϊ���������ж���׼��
     LJMP MINUSFIN
MINUSFIX:
     ADD A,#10
     MOV @R0,A
     LJMP MINUSFIN
;SUBB A, #10H
     ;JNC MINUSFIX        ;�ж���λ����Ƿ���ڵ���10���������������λ���������CY��־λ
     ;CLR C
     ;LJMP MINUSFIN
MINUSFIN:
     INC R0             ;ѭ������
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
     SJMP $
MEM2:
     MOV 17H,#17
     SJMP $             ;�������
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
     JNZ GET2
     DEC R0
     DEC R1
     DJNZ R2,GETLOOP
     LJMP GET2
   GETFINAL:
     MOV RS1,#1B
     MOV RS0,#0B
     MOV R0,#10H
     MOV R1,#20H
     MOV R4,#01H
     RET
   GET2:
     MOV RS1,#1B
     MOV RS0,#0B
     MOV R0,#20H
     MOV R1,#10H
     MOV R4,#00H
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