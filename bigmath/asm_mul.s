.intel_syntax noprefix

#ifdef BIGMATH_ASM_X86_64

#ifdef __GNUC__

#ifdef __clang__
#define BIGMATH_INTERNAL_MUL_WORD __ZN7bigmath8internal8mul_wordEPvPKvmm
#define BIGMATH_INTERNAL_MUL_NAT __ZN7bigmath8internal7mul_natEPvPKvmS3_m
#else
#define BIGMATH_INTERNAL_MUL_WORD _ZN7bigmath8internal8mul_wordEPvPKvmm
#define BIGMATH_INTERNAL_MUL_NAT _ZN7bigmath8internal7mul_natEPvPKvmS3_m
#endif

#endif

.global BIGMATH_INTERNAL_MUL_WORD, BIGMATH_INTERNAL_MUL_NAT

#
#  u64 mul_word(void* res, const void* nat, u64 nat_size, u64 word)
#
#  rdi: res
#  rsi: nat
#  rdx: nat_size
#  rcx: word
#
BIGMATH_INTERNAL_MUL_WORD:
  push rbp
  mov rbp, rsp

  mov rax, rdx
  xchg rdx, rcx

  xor r11, r11

.mul_word_loop:
  mulx r9, r8, [rsi]
  adc r8, r11
  mov [rdi], r8

  mulx r11, r10, [rsi+8]
  adc r9, r10
  mov [rdi+8], r9

  mulx r9, r8, [rsi+16]
  adc r11, r8
  mov [rdi+16], r11

  mulx r11, r10, [rsi+24]
  adc r9, r10
  mov [rdi+24], r9

  lea rsi, [rsi+32]
  lea rdi, [rdi+32]
  dec rcx
  jnz .mul_word_loop

  test r11, r11
  jnz .mul_word_carry
  pop rbp
  ret

.mul_word_carry:
  mov qword ptr [rdi], r11
  mov qword ptr [rdi+8], 0
  mov qword ptr [rdi+16], 0
  mov qword ptr [rdi+24], 0
  inc rax
  pop rbp
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
BIGMATH_INTERNAL_MUL_NAT:
  push rbp
  mov rbp, rsp

  push rbx
  push r12
  push r13
  push r14
  push r15

  cmp r8, rdx
  jbe .L_mul_nat_skip_swap
  xchg rsi, rcx
  xchg rdx, r8

.L_mul_nat_skip_swap:
  shl r8, 2
  shl rdx, 2

  mov [rsp-8], rsi    # [rsp-8] = a
  mov [rsp-16], rcx   # [rsp-16] = b
  mov [rsp-24], r8    # [rsp-24] = b-size
  mov [rsp-32], rdx   # [rsp-32] = a-size

  #  column-index <= other_size
  #    down
  #
  #    rsi = rsi-begin + column-index
  #    rsi >= rsi-begin
  #      2 x mul
  #      --rsi
  #      ++rcx
  #    1 x mul

  mov r15, rsi

  xor r13, r13       # carry0 = 0
  xor r14, r14       # carry1 = 0
  xor r8, r8

  # TODO: consider removing r8 usage as loop counter
  # pre-calculate loop bounds and use rdi instead.

.L_mul_nat_outer1:
  mov rdx, [rcx]
  mov r11, [rsi+8]

  mov rax, r13
  mov rbx, r14
  xor r13, r13       # carry0 = 0
  xor r14, r14       # carry1 = 0

  # LLVM-MCA-BEGIN
.align 4
.L_mul_nat_inner1:
  # rcx = current "b", changes +8 bytes
  # rsi = current "a", changes -8 bytes
  # rax = d0
  # rbx = d1
  # r13 = carry0
  # r14 = carry1
  # r15 = a
  # loop executes r8+1 times

  mulx r10, r9, [rsi]         # a[i] * b[i]
  mulx r12, r11, r11          # a[i+1] * b[i]
  lea rcx, [rcx+8]
  lea rsi, [rsi-8]
  mov rdx, [rcx]

  add rax, r9                 # d0 += lo(a[i] * b[i])
  adc rbx, r10                # d1 += hi(a[i] * b[i])
  adc r13, 0                  # carry0 += CF

  add rbx, r11                # d1 += lo(a[i+1] * b[i])
  adc r13, r12                # carry0 += hi(a[i+1] * b[i])
  adc r14, 0                  # carry1 += CF

  mov r11, [rsi+8]

  cmp rsi, r15
  jae .L_mul_nat_inner1
  # LLVM-MCA-END

  mulx r10, r9, r11

  lea rsi, [r15+8*r8+16]
  mov rcx, [rsp-16]

  add rbx, r9                 # d1 += lo(last)
  adc r13, r10                # carry0 += hi(last)
  adc r14, 0                  # carry1 += CF
  
  lea r8, [r8+2]              # column-index
  mov [rdi], rax
  mov [rdi+8], rbx
  lea rdi, [rdi+16]

  cmp r8, [rsp-24]
  jne .L_mul_nat_outer1

  #  column-index < nat_size
  #    down
  #
  #    rsi = rsi-begin + column-index
  #    rcx < rcx-end
  #      2 x mul
  #      --rsi
  #      ++rcx
  #

  #mov rcx, [rsp-16]     # b
  mov r15, [rsp-24]     # b-size
  lea r15, [rcx+8*r15]  # b[b-size]

.L_mul_nat_outer2:
  cmp r8, [rsp-32]
  jae .L_mul_nat_skip2

  mov r11, [rsi+8]

  mov rax, r13
  mov rbx, r14
  xor r13, r13       # carry0 = 0
  xor r14, r14       # carry1 = 0

.align 4
.L_mul_nat_inner2:
  # rcx = current "b", changes +8 bytes
  # rsi = current "a", changes -8 bytes
  # rax = d0
  # rbx = d1
  # r13 = carry0
  # r14 = carry1
  # r15 = b[b-size] (one past last element)
  # loop executes r8+1 times

  mov rdx, [rcx]
  mulx r10, r9, [rsi]         # a[i] * b[i]
  mulx r12, r11, r11          # a[i+1] * b[i]
  lea rcx, [rcx+8]
  lea rsi, [rsi-8]

  add rax, r9                 # d0 += lo(a[i] * b[i])
  adc rbx, r10                # d1 += hi(a[i] * b[i])
  adc r13, 0                  # carry0 += CF

  add rbx, r11                # d1 += lo(a[i+1] * b[i])
  adc r13, r12                # carry0 += hi(a[i+1] * b[i])
  adc r14, 0                  # carry1 += CF

  mov r11, [rsi+8] # TODO: same address as line 214 (first mulx)

  cmp rcx, r15
  jb .L_mul_nat_inner2

  mov r9, [rsp-16]            # r9 = b
  sub rcx, r9                 # rcx = b-size in bytes
  lea rsi, [rsi+rcx+16]
  mov rcx, r9
  
  lea r8, [r8+2]              # column-index
  mov [rdi], rax
  mov [rdi+8], rbx
  lea rdi, [rdi+16]

  jmp .L_mul_nat_outer2

  #  nat_size <= column-index < nat_size + other_size - 1
  #    down
  #
  #    rsi = rsi-end - 1
  #    1 x mul
  #    rcx < rcx-end
  #      2 x mul
  #      --rsi
  #      ++rcx
  #

.L_mul_nat_skip2:
  mov rsi, [rsp-8]          # a
  mov r8, [rsp-32]          # a-size
  lea rsi, [rsi+8*r8-8]     # rsi = a[a-size-1]
  mov [rsp-8], rsi          # TODO: precalculate before all the loops

  mov rcx, [rsp-16]
  lea rcx, [rcx+8]          # rcx = b[1]

  xor r8, r8

.L_mul_nat_outer3:
  mov rdx, [rcx]
  mov r11, [rsi]
  mulx r10, r9, r11

  mov rax, r13
  mov rbx, r14
  xor r13, r13       # carry0 = 0
  xor r14, r14       # carry1 = 0

  add rax, r9        # d0 += lo
  adc rbx, r10       # d1 += hi
  adc r13, 0         # carry0 += CF

  lea rsi, [rsi-8]
  lea rcx, [rcx+8]

.align 4
.L_mul_nat_inner3:
  cmp rcx, r15
  jae .L_mul_nat_skip_inner3

  mov rdx, [rcx]
  mulx r10, r9, [rsi]
  mulx r12, r11, r11
  lea rcx, [rcx+8]
  lea rsi, [rsi-8]

  add rax, r9                 # d0 += lo(a[i] * b[i])
  adc rbx, r10                # d1 += hi(a[i] * b[i])
  adc r13, 0                  # carry0 += CF

  add rbx, r11                # d1 += lo(a[i+1] * b[i])
  adc r13, r12                # carry0 += hi(a[i+1] * b[i])
  adc r14, 0                  # carry1 += CF

  mov r11, [rsi+8] # TODO: same address as first mulx

  jmp .L_mul_nat_inner3

.L_mul_nat_skip_inner3:
  lea r8, [r8+2]

  mov [rdi], rax
  mov [rdi+8], rbx
  lea rdi, [rdi+16]

  mov rsi, [rsp-8]
  mov rcx, [rsp-16]
  lea rcx, [rcx+8*r8+8]   # rcx = next b

  cmp rcx, r15
  jb .L_mul_nat_outer3

  mov rax, [rsp-24]
  add rax, [rsp-32]
  shr rax, 2

  pop r15
  pop r14
  pop r13
  pop r12
  pop rbx

  pop rbp
  ret

#endif
