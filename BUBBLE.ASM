ORG      0000H
        LJMP     BEGIN
       
        ORG     1000H
BEGIN: 
        MOV     R0,#80H
        MOV     R1,#15
LOOP1:
        MOV     R2,#15
LOOP2:
        LCALL   BUBBLE
        DJNZ    R2,LOOP2
        MOV     R0,#80H
        DJNZ    R1,LOOP1
        SJMP    $

;;input：R0—当前地址指针
;;output：R0=R0+1
;;influenced：A，B
BUBBLE:
        MOV     A,@R0
        INC     R0
        MOV     B,@R0
        CJNE    A,B,NEXT1
NEXT1:
        JNC     NEXT2
        MOV     @R0,A
        DEC     R0
        MOV     @R0,B
        INC     R0
NEXT2:        
        RET
 
