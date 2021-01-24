.intel_syntax noprefix

#ifdef BIGMATH_ASM_X86_64

#
#  bool mul_word(place_t* res, const place_t* nat, u64 nat_size, u64 word);
#
#  rdi: res
#  rsi: nat
#  rdx: nat_size (< 2 ** 32)
#  rcx: word
#
.p2align 4
.global __ZN7bigmath8internal8mul_wordEPNS_7place_tEPKS1_mm
__ZN7bigmath8internal8mul_wordEPNS_7place_tEPKS1_mm:
  mov r8d, edx
  mov rdx, rcx
  mov ecx, r8d
  xor eax, eax

.p2align 4
L_mul_word_loop:
  mulx r9, r8, [rsi]
  add r8, rax
  mov [rdi], r8
  mulx rax, r8, [rsi+8]
  adc r9, r8
  mov [rdi+8], r9

  mulx r9, r8, [rsi+16]
  adc r8, rax
  mov [rdi+16], r8
  mulx rax, r8, [rsi+24]
  adc r9, r8
  mov [rdi+24], r9

  adc rax, 0
  add rdi, 32
  add rsi, 32

  dec ecx
  jnz L_mul_word_loop

  test eax, eax
  jnz L_mul_word_carry
  ret

L_mul_word_carry:
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
.p2align 4
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
  jae L_mul_nat_after_swap
  xchg edx, r8d
  xchg rsi, rcx
L_mul_nat_after_swap:
  mov ebx, r8d
  shl r8d, 5            # R8D = positive bytes in "b"
  shl edx, 5            # EDX = positive bytes in "a"
  mov rbp, rsi          # RBP = "a" start ptr
  add rsi, rdx          # RSI = "a" end ptr
  add rcx, r8           # RCX = "b" end ptr

  # Zero "b" sized part of RDI.

  vpxor xmm0, xmm0, xmm0

  vmovups [rdi], ymm0
  lea r9, [rdi+16]
  and r9, -32

  shr ebx
  jz L_mul_nat_after_zero

.p2align 3
L_mul_nat_zero_64:
  vmovaps [r9], ymm0
  vmovaps [r9+32], ymm0
  add r9, 64
  dec ebx
  jnz L_mul_nat_zero_64

L_mul_nat_after_zero:
  # Main loop.

  add rdi, r8           # RDI = dst at "b" end
  neg r8
  mov [rsp-8], rsi      # [RSP-8] = "a" end ptr
  mov [rsp-16], r8      # [RSP-16] = negative bytes in "b"
  mov rbx, r8           # RBX = negative bytes in "b"

L_mul_nat_outer_loop:
  mov r12, [rbp]
  mov r13, [rbp+8]
  mov r14, [rbp+16]
  mov r15, [rbp+24]

  xor r8d, r8d
  xor r9d, r9d
  xor r10d, r10d
  xor r11d, r11d

L_mul_nat_inner_loop:
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
  adc r8, rsi

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

  adox rax, r8
  mov r9d, 0
  adox r9, r9
  adc rsi, r9

  mov r8, r10
  mov r9, r11
  mov r10, rax
  mov r11, rsi

  add rbx, 16
  jnz L_mul_nat_inner_loop

  mov rsi, [rsp-8]
  mov rbx, [rsp-16]

  mov [rdi], r8
  mov [rdi+8], r9
  mov [rdi+16], r10
  mov [rdi+24], r11

  add rbp, 32
  add rdi, 32

  cmp ebp, esi
  jne L_mul_nat_outer_loop

  vzeroupper
  xor eax, eax
  pop r15
  pop r14
  pop r13
  pop r12
  pop rbx
  pop rbp
  ret

#endif
