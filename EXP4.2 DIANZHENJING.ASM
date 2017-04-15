ORG 0000H

MOV R0,#00H ;行选标志位
MOV R1,#00H ;当前查表标志位

MOV DPTR,#WORD  ;初始化字模数据表地址
;MOV P1,R0    ;点阵LED测试代码
;MOV P0,#01H
;MOV P2,#0FFH
;SJMP $
LOOP:
     MOV P1,R0  ;行选输出

     MOV A,R1   ;列选左半部分查表输出
     MOVC A,@A+DPTR
     MOV P2,A
     INC R1     ;列选左半部分偏离自增

     MOV A,R1   ;列选右半部分查表输出
     MOVC A,@A+DPTR
     MOV P0,A
     INC R1     ;列选右半部分偏离自增
     INC R0     ;行选自增

     CJNE R0,#10H,GOON  ;判断行选是否已经进行到一个周期,没有完成则跳转GOON
     MOV R0,#00H        ;行选一个周期后行选标志位和列选偏移量置位,等待下一次行选周期
     MOV R1,#00H
     LCALL DELAY        ;调用DELAY函数配合视觉暂留,实现多行点阵同时显示
     AJMP LOOP
GOON:
    LCALL DELAY
    AJMP LOOP           ;继续循环
WORD: DB 080H,000H,000H,001H,0FCH,07FH,004H,001H,0F4H,01FH,004H,011H,0FCH,07FH,004H,011H,0F4H,01FH,004H,001H,0F4H,01FH,014H,010H;
      DB 012H,010H,012H,010H,0F1H,01FH,010H,010H ;汉字字模编码："唐"
DELAY:                  ;延时DELAY函数
       mov r5,#20
       d1:mov r6,#5
       d2:mov r7,#5
       djnz r7,$
       djnz r6,d2
       djnz r5,d1
       ret
