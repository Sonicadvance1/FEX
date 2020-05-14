%ifdef CONFIG
{
  "Match": "All",
  "RegData": {
    "RAX": "0x00000000",
    "MM0": ["0x0000000000000000", "0x0000"],
    "MM1": ["0x0000000000000000", "0x0000"],
    "MM2": ["0x0000000000000000", "0x0000"],
    "MM3": ["0x0000000000000000", "0x0000"],
    "MM4": ["0x0000000000000000", "0x0000"],
    "MM5": ["0x0000000000000000", "0x0000"],
    "MM6": ["0xc90fdb0000000000", "0x4000"],
    "MM7": ["0x9de9e6e165590000", "0x4002"]
  },
  "MemoryRegions": {
    "0x100000000": "4096"
  }
}
%endif

; calcuate (1 * pi) + pi
fld dword [rel pi]
fld dword [rel one]
fmulp
fld dword [rel pi]
fmulp

lea rbp, [rsp - 4]
fst dword [rbp]
mov eax, dword [rbp]

hlt

align 8
pi:     dd 0x40490fdb ; 3.14...
one:    dd 0x3f800000 ; 1.0
ptone:  dd 0x3dcccccd ; 0.1
dst:    dd 0x00000000 ;