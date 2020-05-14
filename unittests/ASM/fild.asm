%ifdef CONFIG
{
  "Match": "All",
  "RegData": {
    "MM2": ["0xeceef0f2f4f6f8fc", "0xc03d"],
    "MM3": ["0x80889098a0a8b0b8", "0x403b"],
    "MM4": ["0xf424000000000000", "0xc012"],
    "MM5": ["0xf120000000000000", "0x400f"],
    "MM6": ["0x8000000000000000", "0xbfff"],
    "MM7": ["0xa800000000000000", "0x4004"]
  }
}
%endif

fild word [rel wordint]
fild word [rel wordint + 2]
fild dword [rel dwordint]
fild dword [rel dwordint + 4]
fild qword [rel qwordint]
fild qword [rel qwordint + 8]

hlt

align 8
wordint:    dw 42
            dw -1
dwordint:   dd 123456
            dd -1000000
qwordint:   dq 0x1011121314151617
            dq 0x8988878685848382