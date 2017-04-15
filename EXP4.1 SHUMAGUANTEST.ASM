ORG 0000H
MOV 20H,#02H    ;给20H到27H的存储空间赋初值
MOV 21H,#00H
MOV 22H,#01H
MOV 23H,#07H
MOV 24H,#00H
MOV 25H,#04H
MOV 26H,#01H
MOV 27H,#01H
MOV R0,#20H
MOV R1,#00H
MOV R2,#08H     ;初值赋值结束
LOOP:
    MOV DPTR,#TND       ;获取将要点亮的数码管单位内容（段信息）
    MOV A,@R0
    MOVC A,@A+DPTR
    MOV P0,A

    MOV DPTR,#TWEI      ;获取将要点亮的数码管位信息
    MOV A,R1
    MOVC A,@A+DPTR
    MOV P2,A

    INC R0              ;数码管段和位偏移量自增
    INC R1
    DJNZ R2,GOON        ;判断是否完成对8位数码的完整扫描,如果未完成则继续GOON

    MOV R0,#20H         ;如果已经完成了完整扫描,则重新开始扫描
    MOV R1,#00H
    MOV R2,#08H
GOON:
    LCALL DELAY         ;调用DELAY函数配合视觉暂留,实现多位数码管同时显示
    AJMP LOOP
DELAY:
       mov r5,#20
       d1:mov r6,#10
       d2:mov r7,#5
       djnz r7,$
       djnz r6,d2
       djnz r5,d1
       ret
;AT89C51数码管0-9,A-F编码表,不含小数点,P0口输出
TND: DB 28H,7EH,0A2H,62H,74H,61H,21H,7AH,20H,60H,30H,25H,0A9H,26H,0A1H,0B1H
;AT89C51数码管位选编码表
TWEI: DB 0FEH,0FDH,0FBH,0F7H,0EFH,0DFH,0BFH,07FH
;AT89C51数码管0-9,A-F编码表,含小数点,P0口输出
TD: DB 08H,5EH,82H,42H,54H,41H,01H,5AH,00H,40H,10H,05H,89H,06H,81H,91H
