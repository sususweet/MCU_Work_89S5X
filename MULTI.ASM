ORG 0000H
;ʹ��01λ�Ĵ���R0,R1,R3,��20H,10H��ʼ���������,�������20H
MULTI:
      MOV RS1,#1B
      MOV RS0,#0B
      MOV R0,#20H
      MOV R1,#10H
      MOV R3,#08H

      MOV RS1,#0B
      MOV RS0,#1B
      MOV R0,#10H
      MOV R1,#40H
      LCALL MEMMOVE
      MOV R0,#20H
      MOV R1,#10H
      LCALL MEMMOVE
      MOV RS1,#1B
      MOV RS0,#0B

      CLR C
      SJMP $
MULTILOOP:
     MOV A,@R0

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