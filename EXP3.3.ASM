ORG 0000H
BEGIN:
  MOV R1,#63H           ;变量初始化赋值
  MOV R2,#82H           ;R1:A  R2:B R3:C  R4:D
  MOV R3,#0C5H
  MOV R4,#36H
  MOV A,R1
  XRL A,R2              ;A与B异或运算
  MOV R0,A
  MOV A,R3
  CPL A                 ;C取反
  ANL A,R0              ;C取反的结果与A与B异或运算求与
  CPL A                 ;求与结果取反
  MOV R0,A              ;结果存入R0
  MOV A,R1
  ORL A,R4              ;A与D求或
  CPL A                 ;A与D求或结果取反
  ANL A,R0              ;最后求与
  MOV R0,A              ;结果存入R0

  MOV A,R0              ;对结果低位进行译码转换为数码管显示码
  ANL A,#0FH
  MOV DPTR,#TABLE
  MOVC A,@A+DPTR
  MOV R5,A  ;Low

  MOV A,R0              ;对结果高位进行译码转换为数码管显示码
  ANL A,#0F0H
  SWAP A                ;高位字节与低位字节交换下移
  MOV DPTR,#TABLE
  MOVC A,@A+DPTR
  MOV R6,A   ;High
  SJMP $
  ;AT89C51数码管0-9,A-F编码表
TABLE: DB 14H,07EH,45H,46H,2EH,86H,84H,5EH,04H,06H,0CH,0A4H,95H,64H,85H,8DH

