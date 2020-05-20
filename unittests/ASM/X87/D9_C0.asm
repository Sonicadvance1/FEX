%ifdef CONFIG
{
  "RegData": {
    "MM5":  ["0x8000000000000000", "0x4000"],
    "MM6":  ["0x8000000000000000", "0x4000"],
    "MM7":  ["0x8000000000000000", "0x3FFF"]
  },
  "MemoryRegions": {
    "0x100000000": "4096"
  }
}
%endif

lea rdx, [rel data]
fld tword [rdx + 8 * 0]

lea rdx, [rel data2]
fld tword [rdx + 8 * 0]

fld st1

hlt

align 8
data:
  dt 1.0
  dq 0
data2:
  dt 2.0
  dq 0
