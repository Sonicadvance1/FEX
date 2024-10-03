%ifdef CONFIG
{
  "RegData": {
    "RAX": "0x41582d3bc0000000",
    "RBX": "0x41582d3bc0000000",
    "RCX": "0xc1582d3bc0000000",
    "RDX": "0xc1582d3bc0000000"
  }
}
%endif

; FEX-Emu had a bug in the fprem implementation where it was behaving like fprem1
; Do a handful of large fprem operations to ensure it works correctly.

; 64-bit float memory locations
; doremainder <result>, <src1>, <src2>
%macro doremainder 3
  ; Load big number and divisor
  fld qword %3
  fld qword %2

  ; For large remainders, x86 fprem computes partial remainders and needs to run multiple times.
  %%again:
    ; Get the remainder
    fprem
    ; Check if we need to run again
    fnstsw ax
    test ah, 0x4
    jne %%again

  ; Pop one value
  fstp st1

  ; Store the result
  fstp qword %1
%endmacro

; Do a handful of remainder checks with different sign combinations.
doremainder [rel .data_result + (8 * 0)], [rel .data_big], [rel .data_divisor]
doremainder [rel .data_result + (8 * 1)], [rel .data_big], [rel .data_divisor_negative]
doremainder [rel .data_result + (8 * 2)], [rel .data_big_negative], [rel .data_divisor]
doremainder [rel .data_result + (8 * 3)], [rel .data_big_negative], [rel .data_divisor_negative]

; Load the results in to registers
mov rax, qword [rel .data_result + (8 * 0)]
mov rbx, qword [rel .data_result + (8 * 1)]
mov rcx, qword [rel .data_result + (8 * 2)]
mov rdx, qword [rel .data_result + (8 * 3)]

hlt

.data_big:
dq 83403126337775.0

.data_big_negative:
dq -83403126337775.0

.data_divisor:
dq 10000000.0

.data_divisor_negative:
dq -10000000.0

.data_result:
dq 0, 0, 0, 0
