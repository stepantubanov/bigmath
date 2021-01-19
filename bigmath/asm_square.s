.intel_syntax noprefix

#ifdef BIGMATH_ASM_X86_64

#
#  void square_nat(place_t* res, const place_t* nat, u64 nat_size);
#
#  rdi: res
#  rsi: nat
#  rdx: nat_size (< 32 bit)
#
.p2align 4
.global __ZN7bigmath8internal10square_natEPNS_7place_tEPKS1_m
__ZN7bigmath8internal10square_natEPNS_7place_tEPKS1_m:
  push rbp
  push rbx
  push r12
  push r13
  push r14

  mov rbx, rdi
  mov eax, edx

  lea ecx, [4*edx-2]        # ECX = nat_size in qwords - 2.
  lea rdi, [rdi+8*rcx-8]    # RDI = destination address (for inner loop).

  vpxor xmm0, xmm0, xmm0
  vmovaps [rbx], xmm0

  dec eax
  jnz .L_square_nat_zero_start_loop

  vmovaps [rbx+16], xmm0
  vmovups [rbx+32], ymm0

  jmp .L_square_nat_after_zero

.L_square_nat_zero_start_loop:
  lea r8, [rdi+8*rcx+24]    # R8 = destination end address.
  add rbx, 31
  and rbx, -32              # Align RBX on 32-bytes.
  mov r9, 64

.p2align 3
.L_square_nat_zero:
  vmovaps [rbx], ymm0
  vmovaps [rbx+32], ymm0
  add rbx, r9
  dec eax
  jnz .L_square_nat_zero

  vmovaps [rbx], ymm0
  vmovaps [r8-32], xmm0   # May overlap with the prev.store.
  vmovaps [r8-16], xmm0

.L_square_nat_after_zero:
  # Process the top of the pyramid (A[0] * A[n-1])

  mov rdx, [rsi]            # RDX = a[0]
  mulx rdx, rax, [rsi+8*rcx+8]    # a[0] * a[n-1]
  mov [rdi+16], rax
  mov [rdi+24], rdx

  mov r8d, ecx              # R8 = Outer loop counter (nat_size - 2);
  xor r9d, r9d
  inc r9d                   # R9D = Inner loop iteration count - 1.
  xor r13d, r13d            # R13D = Used as zero constant for ADCX/ADOX.
  mov rbp, rdi              # RBP = Current destination address.

.p2align 4
.L_square_nat_outer_loop:
  mov rdx, [rsi+8*r8-8]
  lea r14, [rsi+8*r8]

  mov rbx, rsi
  mov ecx, r9d
  xor eax, eax

.p2align 4
.L_square_nat_inner_loop:
  # R8 = outer loop index
  # ECX = inner loop index [-n to -2]
  # RAX, CF = carry
  # R13 = zero
  # RBX, RDX, R10, R11, R12 = clobbers

  mov r10, [rbx]            # a[j+0] ~ a[0]
  mulx r12, r11, r10

  adcx rax, [rdi]
  adox rax, r11
  mov [rdi], rax

  mov rdx, [r14]          # a[j+i+2] ~ a[2]
  mulx rax, r11, r10

  adcx r12, [rdi+8]
  adox r12, r11
  mov [rdi+8], r12

  mov r10, [rbx+8]          # a[j+1] ~ a[1]
  mulx r12, r11, r10

  adcx rax, [rdi+16]
  adox rax, r11
  mov [rdi+16], rax

  mov rdx, [r14+8]          # a[j+i+3] ~ a[3]
  mulx rax, r11, r10

  adcx r12, [rdi+24]
  adox r12, r11
  mov [rdi+24], r12

  adox rax, r13           # Guaranteed not to overflow.
                          # Max value of hi part is (~0 - 1), which
                          # can accomodate addition of OF.

  lea rbx, [rbx+16]
  lea r14, [r14+16]
  lea rdi, [rdi+32]

  dec ecx
  jnz .L_square_nat_inner_loop

  mulx r12, r11, [rbx]      # Compute the last product.

  adc r11, rax              # Consolidate carries (RAX, CF).
  adc r12, 0
  mov [rdi], r11
  mov [rdi+8], r12

  sub rbp, 16
  mov rdi, rbp

  inc r9d
  sub r8d, 2
  jnz .L_square_nat_outer_loop

  add rdi, 8      # RDI to the starting position

  mov r10d, -1    # R10D used for overflow flag restore.
  xor r8d, r8d

.p2align 4
.L_square_nat_square_loop:
  mov rdx, [rsi]
  mov rbx, [rdi]
  mov rcx, [rdi+8]

  adox r8d, r10d # Restore overflow flag.

  mulx rdx, rax, rdx

  adcx rbx, rbx
  adox rbx, rax
  adcx rcx, rcx
  adox rcx, rdx

  mov [rdi], rbx
  mov [rdi+8], rcx

  mov rdx, [rsi+8]
  mov rbx, [rdi+16]
  mov rcx, [rdi+24]

  mulx rdx, rax, rdx

  adcx rbx, rbx
  adox rbx, rax
  adcx rcx, rcx
  adox rcx, rdx

  mov r10d, 0
  mov [rdi+16], rbx
  mov [rdi+24], rcx

  seto r10b

  lea rsi, [rsi+16]
  lea rdi, [rdi+32]

  dec r9d
  jnz .L_square_nat_square_loop

  vzeroupper
  pop r14
  pop r13
  pop r12
  pop rbx
  pop rbp
  ret

#endif
