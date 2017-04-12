org 00h
jmp start
org 03h
jmp ext0
start: mov ie,#10000001b
       mov ip,#00000001b
       mov tcon,#00000000b
       mov sp,#70h
org 0043h
main: mov p1,#00h
      mov P2,#00h
      lcall delay
      mov P2,#0FFh
      mov a,#0feh
loop: mov p1,a
      lcall delay
      rl a
      ajmp loop
ext0: push acc
      push psw
      mov p1,#00h
      cpl p3.3
      call del
      pop psw
      pop acc
      reti
delay: 
       mov r5,#20
       d1:mov r6,#20
       d2:mov r7,#250
       djnz r7,$
       djnz r6,d2
       djnz r5,d1
       ret
del: mov r4, #250
     djnz r4,$
     ret
     end
