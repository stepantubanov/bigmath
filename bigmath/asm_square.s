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

  mov rbx, rdi
  mov eax, edx

  shl edx, 5                # EDX = nat_size in bytes.
  mov ecx, edx              # ECX = nat_size in bytes.
  lea rdi, [rdi+rdx-24]     # RDI = destination address (for inner loop).
  #lea r8, [rdi+rcx+24]     # R8 = destination end address.
  lea r8, [rdi+rcx-40]      # R8 = destination end address.
  mov r12, r8
  mov rdx, [rsi]            # RDX = a[0]

  vpxor xmm0, xmm0, xmm0
  vmovaps [rbx], xmm0

  lea eax, [ecx+ecx-64]
  test eax, eax
  jz .L_square_nat_skip_zero_loop

  neg rax
  add r8, 31
  and r8, -32

.p2align 4
.L_square_nat_zero_loop:
  vmovaps [r8+rax], ymm0
  vmovaps [r8+rax+32], ymm0
  add rax, 64
  jnz .L_square_nat_zero_loop

.L_square_nat_skip_zero_loop:
  vmovaps [r12], xmm0
  vmovaps [r12+16], xmm0
  vmovups [r12+32], ymm0

.L_square_nat_after_zero:
  # Process the top of the pyramid (A[0] * A[n-1])

  mulx rdx, rax, [rsi+rcx-8]    # a[0] * a[n-1]
  mov [rdi+16], rax
  mov [rdi+24], rdx

  lea r8d, [ecx-16]         # R8 = Outer loop counter (nat_size - 2) in bytes.
  xor r9d, r9d
  inc r9d                   # R9D = Inner loop iteration count - 1.
  xor r13d, r13d            # R13D = Used as zero constant for ADCX/ADOX.
  mov rbp, rdi              # RBP = Current destination address.

.p2align 4
.L_square_nat_outer_loop:
  mov rdx, [rsi+r8-8]

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
  mov rdx, [rbx+r8]          # a[j+i+2] ~ a[2]
  mov [rdi], rax

  mulx rax, r11, r10

  adcx r12, [rdi+8]
  adox r12, r11
  mov r10, [rbx+8]          # a[j+1] ~ a[1]
  mov [rdi+8], r12

  mulx r12, r11, r10

  adcx rax, [rdi+16]
  adox rax, r11
  mov rdx, [rbx+r8+8]          # a[j+i+3] ~ a[3]
  mov [rdi+16], rax

  mulx rax, r11, r10

  lea rbx, [rbx+16]

  adcx r12, [rdi+24]
  adox r12, r11
  mov [rdi+24], r12

  lea rdi, [rdi+32]

  adox rax, r13           # Guaranteed not to overflow.
                          # Max value of hi part is (~0 - 1), which
                          # can accomodate addition of OF.


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
  sub r8d, 16
  jnz .L_square_nat_outer_loop

  add rdi, 8      # RDI to the starting position

  xor r11d, r11d
  xor r10d, r10d
  not r10d        # R10D used for overflow flag restore.
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

  mov [rdi+16], rbx
  mov [rdi+24], rcx

  cmovno r8d, r11d

  lea rsi, [rsi+16]
  lea rdi, [rdi+32]

  dec r9d
  jnz .L_square_nat_square_loop

  vzeroupper
  pop r13
  pop r12
  pop rbx
  pop rbp
  ret

#endif
