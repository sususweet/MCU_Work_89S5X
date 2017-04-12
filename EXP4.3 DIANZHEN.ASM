ORG 0000H
MOV R0,#00H ;列选标志位
MOV R1,#00H ;当前查表标志位
MOV R2,#00H ;滚动偏移标志位
MOV R3,#00H ;滚动延时标志位

MOV DPTR,#WORD
;MOV P1,R0
;MOV P0,#0FFH
;MOV P2,#0FFH
;SJMP $
LOOP1:
     CJNE R3,#04H,JUDGER2
     MOV R3,#00H
JUDGER2:
     CJNE R2,#40H,LOOP
     MOV DPTR,#WORD
     MOV R2,#00H
LOOP:
     MOV P1,R0
     
     MOV A,R1
     ADD A,R2
     MOV R1,A
     
     MOV A,R1
     MOVC A,@A+DPTR
     MOV P2,A
     INC R1
     ;MOV A,#0FFH
     ;MOV P2,A
     
     MOV A,R1
     MOVC A,@A+DPTR
     MOV P0,A
     INC R1
     INC R0
     ;MOV A,#01H
     ;MOV P0,A
     CJNE R0,#10H,GOON
     INC DPTR
     INC DPTR
     INC R3
     INC R2
     INC R2
     MOV R0,#00H
     MOV R1,#00H
     AJMP LOOP1
GOON:
    LCALL DELAY
    AJMP LOOP
WORD: DB 040H,000H,080H,000H,0FCH,01FH,010H,004H,020H,002H,0FFH,07FH,000H,000H,0F8H,00FH,008H,008H,0F8H,00FH,008H,008H,0F8H,00FH
      DB 080H,000H,0FFH,07FH,080H,000H,080H,000H
      DB 004H,004H,008H,004H,008H,004H,09FH,03FH,090H,024H,088H,024H,088H,024H,09CH,03FH,0AAH,024H,0A9H,024H,088H,024H,088H,03FH
      DB 088H,024H,008H,004H,008H,004H,008H,004H
DELAY:
       mov r5,#20
       d1:mov r6,#5
       d2:mov r7,#5
       djnz r7,$
       djnz r6,d2
       djnz r5,d1
       ret
DELAY1:
       mov r5,#20
       d3:mov r6,#10
       d4:mov r7,#10
       djnz r7,$
       djnz r6,d4
       djnz r5,d3
       ret

