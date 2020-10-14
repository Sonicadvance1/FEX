%ifdef CONFIG
{
  "RegData": {
    "RBX": "0x80000000",
    "RDI": "0x00000000",
    "RDX": "0xC0000000",
    "RSI": "0x40000000",
    "R8":  "0x1",
    "R9":  "0x0",
    "R10": "0x0",
    "R11": "0x1"
  }
}
%endif

mov rbx, 0x00000001
mov rdi, 0x00000001
mov rdx, 0x80000000
mov rsi, 0x80000000
mov r15, 1
mov rcx, 1

stc
rcr ebx, cl
mov r8, 0
cmovo r8, r15 ; We only care about OF here

clc
rcr edi, cl
mov r9, 0
cmovo r9, r15 ; We only care about OF here

stc
rcr edx, cl
mov r10, 0
cmovo r10, r15 ; We only care about OF here

clc
rcr esi, cl
mov r11, 0
cmovo r11, r15 ; We only care about OF here

hlt
