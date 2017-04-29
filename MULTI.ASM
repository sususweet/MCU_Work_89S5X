ORG 0000H
;使用01位寄存器R0,R1,R3,将20H,10H开始的数码相减,结果存入20H
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
