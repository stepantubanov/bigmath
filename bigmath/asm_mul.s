.intel_syntax noprefix

#ifdef BIGMATH_ASM_X86_64

#
#  bool mul_word(place_t* res, const place_t* nat, u64 nat_size, u64 word);
#
#  rdi: res
#  rsi: nat
#  rdx: nat_size (< 32 bit)
#  rcx: word
#
.p2align 4
.global __ZN7bigmath8internal8mul_wordEPNS_7place_tEPKS1_mm
__ZN7bigmath8internal8mul_wordEPNS_7place_tEPKS1_mm:
  xchg rcx, rdx
  xor eax, eax

.p2align 4
.L_mul_word_loop:
  mulx r9, r8, [rsi]
  adc r8, rax
  mulx rax, r10, [rsi+8]
  adc r9, r10
  mov [rdi], r8
  mov [rdi+8], r9

  mulx r9, r8, [rsi+16]
  adc r8, rax
  mulx rax, r10, [rsi+24]
  adc r9, r10
  mov [rdi+16], r8
  mov [rdi+24], r9

  lea rdi, [rdi+32]
  lea rsi, [rsi+32]

  dec ecx
  jnz .L_mul_word_loop

  adc rax, 0
  jnz .L_mul_word_carry
  ret

.L_mul_word_carry:
  xor esi, esi
  mov qword ptr [rdi], rax
  mov qword ptr [rdi+8], rsi
  mov qword ptr [rdi+16], rsi
  mov qword ptr [rdi+24], rsi
  ret

#  u64 mul_nat(void* res, const void* nat, u64 nat_size, const void* other,
#              u64 other_size) {
#
#  rdi: res
#  rsi: nat
#  rdx: nat_size
#  rcx: other
#  r8: other_size
#
.p2align 2
.global __ZN7bigmath8internal7mul_natEPNS_7place_tEPKS1_mS4_m
__ZN7bigmath8internal7mul_natEPNS_7place_tEPKS1_mS4_m:
  prefetchw [rdi]
  prefetcht0 [rsi]
  prefetcht0 [rcx]

  push rbp
  push rbx
  push r12
  push r13
  push r14
  push r15

  cmp r8d, edx
  jae .L_mul_nat_after_swap
  xchg rdx, r8
  xchg rsi, rcx
.L_mul_nat_after_swap:
  mov ebx, r8d
  shl r8d, 5            # R8D = positive bytes in "b"
  shl edx, 5            # EDX = positive bytes in "a"

  mov rbp, rsi
  mov r9, rdi

  add rcx, r8           # RCX = "b" end ptr
  add rdi, r8           # RDI = dst at "b" end
  add rsi, rdx          # RSI = "a" end ptr

  neg r8
  mov [rsp-8], r8       # [RSP-8] = negative bytes in "b"
  mov [rsp-16], rsi     # [RSP-24] = "a" end ptr

  # Zero "b" sized part of RDI.

  vpxor xmm0, xmm0, xmm0

  vmovaps [r9], xmm0
  add r9, 16
  and r9, -32

  shr ebx
  jz .L_mul_nat_zero_32

.p2align 3
.L_mul_nat_zero_64:
  vmovaps [r9], ymm0
  vmovaps [r9+32], ymm0
  add r9, 64
  dec ebx
  jnz .L_mul_nat_zero_64

  vmovaps [rdi-32], xmm0
.L_mul_nat_zero_32:
  vmovaps [rdi-16], xmm0

  # Main loop.

  # RBP => points to a[0]
  # RBX => negative bytes in "b"
  mov rbx, r8

.p2align 4
.L_mul_nat_outer_loop:
  mov r12, [rbp]
  mov r13, [rbp+8]
  mov r14, [rbp+16]
  mov r15, [rbp+24]

  xor r8d, r8d
  xor r9d, r9d
  xor r10d, r10d
  xor r11d, r11d

.p2align 4
.L_mul_nat_inner_loop:
  # Unroll [0]

  mov rdx, [rcx+rbx]

  xor esi, esi
  mulx rsi, rax, r12

  adcx r8, [rdi+rbx]
  adox r8, rax
  adcx r9, rsi

  mulx rsi, rax, r13

  mov [rdi+rbx], r8
  adox r9, rax
  adcx r10, rsi

  mulx rsi, rax, r14

  adox r10, rax
  adcx r11, rsi

  mulx rsi, rax, r15

  adox r11, rax
  mov r8d, 0
  adox r8, r8
  adcx r8, rsi

  # Unroll [1]

  mov rdx, [rcx+rbx+8]
  xor esi, esi

  mulx rsi, rax, r12

  adcx r9, [rdi+rbx+8]
  adox r9, rax
  adcx r10, rsi

  mulx rsi, rax, r13

  mov [rdi+rbx+8], r9
  adox r10, rax
  adcx r11, rsi

  mulx rsi, rax, r14

  adox r11, rax
  adcx r8, rsi

  mulx rsi, rax, r15

  mov r9d, 0
  adox rax, r8
  adcx rsi, r9
  adox rsi, r9

  mov r8, r10
  mov r9, r11
  mov r10, rax
  mov r11, rsi

  add rbx, 16
  jnz .L_mul_nat_inner_loop

  mov rbx, [rsp-8]
  mov rsi, [rsp-16]

  mov [rdi], r8
  mov [rdi+8], r9
  mov [rdi+16], r10
  mov [rdi+24], r11

  add rbp, 32
  add rdi, 32

  xor r8d, r8d
  xor r9d, r9d
  xor r10d, r10d
  xor r11d, r11d

  cmp rbp, rsi
  jne .L_mul_nat_outer_loop

  xor eax, eax

  pop r15
  pop r14
  pop r13
  pop r12
  pop rbx
  pop rbp
  ret

#endif
