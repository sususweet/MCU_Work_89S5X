ORG 0000H
BEGIN:
      MOV R0,#40H       ;十进制数存放地址初始化
      MOV R1,#50H
      MOV R3,30H        ;字节长度初始化
      CLR C
LOOP:                   ;从40H开始循环遍历十进制数的各个数位
     MOV A,@R0
     ADDC A,@R1
     DA A               ;十进制数数位相加，进行二-十进制调整
     MOV @R0,A          ;暂存十进制数数位相加结果
     CLR C              ;清空CY，为接下来的判断做准备
     SUBB A, #10H       
     JNC MINUS          ;判断数位相加是否大于等于10，若条件满足则进位，否则清空CY标志位
     CLR C
     LCALL FINAL
MINUS:
     MOV @R0,A          ;CY置位，为进位做准备
     SETB C
FINAL:
     INC R0             ;循环遍历
     INC R1             
     DJNZ R3,LOOP
     SJMP $             ;程序结束
