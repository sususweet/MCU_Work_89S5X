ORG 0000H
;LJMP START
MOV R0,#20H
MOV RS1,#0B
MOV RS0,#0B
LCALL CLEAR
KEY_SCAN: ;����ɨ��ʹ��P1��,P1.4-P1.7Ϊ��ɨ��,P1.0-P1.3Ϊ��ɨ��
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
    LCALL JUDGELIE
LIE2:
    MOV R4,#02H
    LCALL JUDGELIE
LIE3:
    MOV R4,#03H
    LCALL JUDGELIE
LIE4:
    MOV R4,#04H
    LCALL JUDGELIE
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
    MOV A,R5
    DEC A
    MOV B,#04H
    MUL AB
    ADD A,R4
    DEC A
    MOV DPTR,#KEY_TABLE
    MOVC A,@A+DPTR
    MOV @R0,A
    INC R0
    LCALL RLCBIT
    CJNE R0,#29H,KEY_SCAN
    MOV R0,#20H
    LCALL CLEAR
    LJMP KEY_SCAN
KEY_TABLE:    ;0AH:+  0BH:-  0CH:x  0DH:/  0EH:*  0FH:# 
    DB 01H,02H,03H,0AH,04H,05H,06H,0BH,07H,08H,09H,0CH,0EH,00H,0FH,0DH
CLEAR:
MOV R1,#20H
MOV R2,#09H
CLOOP:MOV @R1,#10H
INC R1
DJNZ R2,CLOOP
RET

DISP_LED:
;;MOV 20H,#01H    ;��20H��27H�Ĵ洢�ռ丳��ֵ
;MOV 21H,#02H
;MOV 22H,#03H
;MOV 23H,#04H
;MOV 24H,#05H
;MOV 25H,#06H
;MOV 26H,#07H
;MOV 27H,#08H
MOV R1,#21H
MOV R3,#00H
MOV R2,#09H     ;��ֵ��ֵ����
LOOP:
    MOV DPTR,#TND       ;��ȡ��Ҫ����������ܶ�
    MOV A,@R1
    MOVC A,@A+DPTR
    MOV P0,A

    MOV DPTR,#TWEID      ;��ȡ��Ҫ�����������λ��Ϣ
    MOV A,R3
    MOVC A,@A+DPTR
    MOV P2,A

    INC R1              ;����ܶκ�λƫ��������
    INC R3
    DJNZ R2,GOON        ;�ж��Ƿ���ɶ�8λ���������ɨ��,���δ��������GOON

    RET
    ;MOV R0,#20H         ;����Ѿ����������ɨ��,�����¿�ʼɨ��
    ;;MOV R1,#00H
    ;MOV R2,#08H
GOON:
    LCALL DELAY         ;����DELAY��������Ӿ�����,ʵ�ֶ�λ�����ͬʱ��ʾ
    AJMP LOOP
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
       MOV R0,#28H
       MOV R1,#27H
RLCLOOP:
       MOV A,@R1
       MOV @R0,A
       DEC R0
       DEC R1
       CJNE R1,#1FH,RLCLOOP
       MOV RS1,#0B
       MOV RS0,#0B
       RET
;AT89C51�����0-9,A-F�����,����С����,P0�����
TND: DB 28H,7EH,0A2H,62H,74H,61H,21H,7AH,20H,60H,30H,25H,0A9H,26H,0A1H,0B1H,0FFH
;AT89C51�����λѡ�����
TWEI: DB 0FEH,0FDH,0FBH,0F7H,0EFH,0DFH,0BFH,07FH
TWEID: DB 07FH,0BFH,0DFH,0EFH,0F7H,0FBH,0FDH,0FEH
;AT89C51�����0-9,A-F�����,��С����,P0�����
TD: DB 08H,5EH,82H,42H,54H,41H,01H,5AH,00H,40H,10H,05H,89H,06H,81H,91H