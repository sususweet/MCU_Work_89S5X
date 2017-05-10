;;位空间定义
        B_CURSOR BIT     00H
;;内部RAM空间
        LED_BUF  EQU     28H          ;;LED显示缓存28H~2FH
        
        DELH     EQU     30H
        DELL     EQU     31H
;;按键相关寄存器
        KEYCODE  EQU     32H
        LASTCODE EQU     33H
        N_SAME   EQU     34H
        TEMP     EQU     35H
        N_DISP   EQU     36H
;;光标位置
        CURSOR   EQU     37H
;;时钟相关寄存器
        T_2MS    EQU    40H             ;;2ms定时计数
        T_100MS  EQU    41H             ;;100ms定时计数

        SECOND  EQU     42H
        MINUTE  EQU     43H
        HOUR    EQU     44H

        ORG     0000H
        LJMP    BEGIN

        ORG     000BH
        LJMP    TIMER0_INT



        ORG     0100H
BEGIN:
        MOV     SP,#70H
;;光标初始化位置
        MOV     CURSOR,#0
        CLR     B_CURSOR
;;按键初始化
        MOV     KEYCODE,#0FFH
        MOV     LASTCODE,#0FFH
        MOV     N_SAME,#0
;;LED显示初始化
        MOV     R1,#8
        MOV     R0,#LED_BUF
CLRLOOP:
        MOV     @R0,#0FH
        INC     R0
        DJNZ    R1,CLRLOOP
;;显示时间值初始化
        LCALL   INI_TIMER


LOOP:
        LJMP    LOOP
;;定时器T0初始化
INI_TIMER:
        MOV     T_2MS,#0
        MOV     T_100MS,#0
        MOV     SECOND,#0
        MOV     MINUTE,#59
        MOV     HOUR,#11
        MOV     TMOD,#01H               ;;TIMER0 MODE1
        MOV     TCON,#0
        MOV     TH0,#0F8H
        MOV     TL0,#30H               ;;2mS
        SETB    TR0
        SETB    ET0
        SETB    EA
        RET
;;定时器中断
TIMER0_INT:
        PUSH    ACC
        PUSH    PSW
        LCALL TIMER              ;;时钟运行
        LCALL   DISP_TIMER       ;;时钟放入显示缓冲
        LCALL   KEY_SCAN         ;;键盘扫描
        LCALL   DO_KEY           ;;根据键码值运行相应操作
        LCALL   SET_CURSOR       ;;显示光标

        LCALL   DISP_LED         ;;LED显示
        POP     PSW
        POP     ACC
        RETI

;;内部时钟运行
;;INPUT:NONE
;;OUTPUT:TIP,SECOND,MINUTE,HOUR
;;INFLUENCED:ACC

TIMER:
        MOV     TH0,#0F8H
        MOV     TL0,#30H               ;;2mS
;;2ms计数加1
        INC     T_2MS
        MOV     A,T_2MS
        CLR     C
        SUBB    A,#50
        JC      TIMER0_RET
        MOV     T_2MS,A
;;100ms计数加1
        INC     T_100MS
        MOV     A,T_100MS
        CLR     C
        SUBB    A,#10
        JC      TIMER0_RET
        MOV     T_100MS,A
;;秒计数加1
        INC     SECOND
        MOV     A,SECOND
        CLR     C
        SUBB    A,#60
        JC      TIMER0_RET
        MOV     SECOND,A
;;分钟计数加1
        INC     MINUTE
        MOV     A,MINUTE
        CLR     C
        SUBB    A,#60
        JC      TIMER0_RET
        MOV     MINUTE,A
;;小时计数加1
        INC     HOUR
        MOV     A,HOUR
        SUBB    A,#12
        JC      TIMER0_RET
        MOV     HOUR,A
TIMER0_RET:
        RET

;;设置光标
;;INPUT:B_CURSOR,CURSOR
;;OUTPUT:LED_BUF
;;INFLUENCED:ACC
SET_CURSOR:
        JNB     B_CURSOR,CURSOR_OUT
        MOV     A,T_100MS
        CLR     C
        SUBB    A,#5
        JC      CURSOR_OUT
        MOV     A,CURSOR
        ANL     A,#7
        ADD     A,#LED_BUF
        MOV     R0,A
        MOV     @R0,#0FH
        
CURSOR_OUT:
        RET

;;显示时间，将显示数据放入显示缓冲区
;;INPUT:SECOND,MINUTE,HOUR
;;OUTPUT:LED_BUF
;;INFLUENCED:ACC，B，R0
DISP_TIMER:
        MOV     R0,#LED_BUF
        MOV     A,HOUR
        MOV     B,#10
        DIV     AB
        MOV     @R0,A
        INC     R0
        MOV     @R0,B
        INC     R0              ;;DISPLAY HOUR

        MOV     @R0,#0EH        ;;-
        INC     R0

        MOV     A,MINUTE
        MOV     B,#10
        DIV     AB
        MOV     @R0,A
        INC     R0
        MOV     @R0,B
        INC     R0               ;;DISPLAY MINUTE

        MOV     @R0,#0EH        ;;-
        INC     R0

        MOV     A,SECOND
        MOV     B,#10
        DIV     AB
        MOV     @R0,A
        INC     R0
        MOV     @R0,B            ;;DISPLAY SECOND


        RET



;;DELAY 2ms
DELAY2MS:
        MOV     DELH,#2
DELAY2:
        MOV     DELL,#250
DELAY1:
        NOP
        NOP
        DJNZ    DELL,DELAY1
        DJNZ    DELH,DELAY2
        RET
        
;;按键处理
;;INPUT:KEYCODE
;;OUTPUT:
;;INFLUENCED:ACC
;;           
DO_KEY:
        MOV     A,KEYCODE
        CJNE    A,#0FFH,KEY_0
        RET
KEY_0:
        CJNE    A,#0FH,KEY_1
        CPL     B_CURSOR
        SJMP    KEY_RET
        
KEY_1:

      NOP

KEY_RET:
        MOV     KEYCODE,#0FFH                
        RET

;;键盘扫描
;;INPUT:P1，P2
;;OUTPUT:KEYCODE
;;INFLUENCED:LASTCODE-上次采样的键码
;;           N_SAME -采样到相同键码的次数
;;DPTR
KEY_SCAN:
        MOV     P1,#0F0H
        NOP
        MOV     A,P1
        ANL     A,#0F0H
        MOV     B,A
        MOV     P1,#0FH
        NOP
        MOV     A,P1
        ANL     A,#0FH
        ORL     A,B
        MOV     DPTR,#TAB_KEY
        MOVC    A,@A+DPTR
        CJNE    A,LASTCODE,GET_NEW_KEY
        MOV     A,N_SAME
        CJNE    A,#3,GET_NEXT
;;新键码
        MOV     KEYCODE,LASTCODE
KEY_EXIT:
        INC     N_SAME
        RET
GET_NEXT:
        JC      KEY_EXIT
        RET
GET_NEW_KEY:
        MOV     LASTCODE,A
        MOV     N_SAME,#0
        RET

TAB_KEY:
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH ;0-10H
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH ;10-1FH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH ;20-2FH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH ;30-3FH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH ;40-4FH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH ;50-5FH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH ;60-6FH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0CH,0FFH,0FFH,0FFH,0DH,0FFH,0EH,0FH,0FFH ;70-7FH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH  ;80-8FH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH  ;90-9FH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH  ;A0-AFH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0AH,0FFH,0FFH,0FFH,09H,0FFH,00H,0BH,0FFH  ;B0-BFH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH  ;C0-CFH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,05H,0FFH,0FFH,0FFH,06H,0FFH,07H,08H,0FFH  ;D0-DFH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,01H,0FFH,0FFH,0FFH,02H,0FFH,03H,04H,0FFH  ;E0-EFH
        DB      0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH,0FFH  ;F0-FFH



;;LED动态扫描
;;INPUT:LED_BUF
;;OUTPUT:KEYCODE
;;INFLUENCED:Acc，DPTR
DISP_LED:
        MOV     A,#LED_BUF
        ADD     A,N_DISP
        MOV     R0,A
        MOV     A,@R0
        MOV     DPTR,#TAB_LED
        ANL     A,#0FH
        MOVC    A,@A+DPTR
        MOV     P0,A

        MOV     A,#0FFH
        CLR     C
        INC     N_DISP
        MOV     R0,N_DISP
LOOP_D1:
        RLC     A
        DJNZ    R0,LOOP_D1
        MOV     P2,A
        ANL     N_DISP,#07H
        RET




TAB_LED:
        DB      28H,7EH,0A2H,62H,74H
        DB      61H,21H,7AH,20H,60H
        DB      0FFH,0FFH,0FFH,0FFH,0F7H,0EFH


