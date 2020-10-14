%ifdef CONFIG
{
  "RegData": {
    "XMM0": ["0xD5D56A6A6AD5D56A", "0x6A6AD5D5D56A6AD5"],
    "XMM1": ["0x2A2A9595952A2A95", "0x95952A2A2A95952A"],
    "XMM2": ["0xD5D56A6B6AD5D56B", "0x6A6AD5D4D56A6AD4"],
    "XMM3": ["0x2A2A95956AD5D56A", "0x6A6AD5D42A95952A"]
  }
}
%endif

mov rdx, 0xe0000000

mov rax, 0x0000000000000000
mov [rdx + 8 * 0], rax
mov [rdx + 8 * 1], rax

mov rax, 0xFFFFFFFFFFFFFFFF
mov [rdx + 8 * 2], rax
mov [rdx + 8 * 3], rax

mov rax, 0x0000000100000001
mov [rdx + 8 * 4], rax
mov [rdx + 8 * 5], rax

mov rax, 0xFFFFFFFF00000000
mov [rdx + 8 * 6], rax
mov rax, 0x00000001FFFFFFFF
mov [rdx + 8 * 7], rax

mov rax, 0x0202020202020202
mov [rdx + 8 * 8], rax
mov rax, 0x0303030303030303
mov [rdx + 8 * 9], rax

movaps xmm0, [rdx + 8 * 8]
movaps xmm1, [rdx + 8 * 8]
movaps xmm2, [rdx + 8 * 8]
movaps xmm3, [rdx + 8 * 8]

aesdeclast xmm0, [rdx + 8 * 0]

aesdeclast xmm1, [rdx + 8 * 2]

aesdeclast xmm2, [rdx + 8 * 4]

aesdeclast xmm3, [rdx + 8 * 6]

hlt
