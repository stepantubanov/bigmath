.intel_syntax noprefix

#ifdef BIGMATH_ASM_X86_64

#ifdef __GNUC__

#ifdef __clang__
#define BIGMATH_INTERNAL_ADD_WORD __ZN7bigmath8internal8add_wordEPvmm
#define BIGMATH_INTERNAL_ADD_NAT  __ZN7bigmath8internal7add_natEPvmPKvm
#else
#define BIGMATH_INTERNAL_ADD_WORD _ZN7bigmath8internal8add_wordEPvmm
#define BIGMATH_INTERNAL_ADD_NAT  _ZN7bigmath8internal7add_natEPvmPKvm
#endif

#endif

.global BIGMATH_INTERNAL_ADD_WORD, BIGMATH_INTERNAL_ADD_NAT

#
#  u64 add_word(void* nat, u64 nat_size, u64 word)
#
#  rdi: nat
#  rsi: nat_size
#  rdx: word
#
BIGMATH_INTERNAL_ADD_WORD:
  push rbp
  mov rbp, rsp
  mov rax, rsi
  add qword ptr [rdi], rdx
  jc .add_word_continue
  pop rbp
  ret
.add_word_continue:
  adc qword ptr [rdi+8], 0
  adc qword ptr [rdi+16], 0
  adc qword ptr [rdi+24], 0
  jc .add_word_carry_loop
  pop rbp
  ret
.add_word_carry_loop:
  lea rdi, [rdi+32]
  dec rsi
  jz .add_word_finalize
  add qword ptr [rdi], 1
  adc qword ptr [rdi+8], 0
  adc qword ptr [rdi+16], 0
  adc qword ptr [rdi+24], 0
  jc .add_word_carry_loop
  pop rbp
  ret
.add_word_finalize:
  mov qword ptr [rdi], 1
  mov qword ptr [rdi+8], 0
  mov qword ptr [rdi+16], 0
  mov qword ptr [rdi+24], 0
  inc rax
  pop rbp
  ret

#
#  u64 add_nat(void* nat, u64 nat_size,
#              const void* other, u64 other_size)
#
#  rdi: nat
#  rsi: nat_size (< 32 bit)
#  rdx: other
#  rcx: other_size (< 32 bit)
#
BIGMATH_INTERNAL_ADD_NAT:
  push rbp
  mov rbp, rsp

  sub rdx, rdi

  xor r8, r8
  mov rax, rsi
  cmp rcx, rsi
  cmovb rax, rcx      # rax = min_size
  setbe r8b           # big+small or equal size
  mov [rsp-8], r8

  xor r8, r8
  mov r8, [rdi+rdx]
  mov r9, [rdi+rdx+8]
  mov r10, [rdi+rdx+16]
  mov r11, [rdi+rdx+24]

.add_nat_min_loop:
  adc [rdi], r8
  adc [rdi+8], r9
  adc [rdi+16], r10
  adc [rdi+24], r11
  mov r8, [rdi+rdx+32]
  mov r9, [rdi+rdx+40]
  mov r10, [rdi+rdx+48]
  mov r11, [rdi+rdx+56]
  lea rdi, [rdi+32]
  dec rax
  jnz .add_nat_min_loop

.add_nat_after_min_loop:
  dec qword ptr [rsp-8]
  jnz .add_nat_small_big

# --- Big + Small

  mov rax, rsi
  jc .add_nat_big_small_carry
  pop rbp
  ret

.add_nat_big_small_carry:
  sub rsi, rcx
.add_nat_big_small_carry_loop:
  dec rsi
  js .add_nat_carry_exit
  add qword ptr [rdi], 1
  adc qword ptr [rdi+8], 0
  adc qword ptr [rdi+16], 0
  adc qword ptr [rdi+24], 0
  lea rdi, [rdi+32]
  jc .add_nat_big_small_carry_loop
  pop rbp
  ret
.add_nat_carry_exit:
  mov qword ptr [rdi], 1
  mov qword ptr [rdi+8], 0
  mov qword ptr [rdi+16], 0
  mov qword ptr [rdi+24], 0
  inc rax
  pop rbp
  ret

# --- Small + Big

.add_nat_small_big:
  mov rax, rcx
  jc .add_nat_small_big_carry
  sub rcx, rsi

.add_nat_small_big_copy:
  lea rsi, [rdi+rdx]

  test rcx, rcx
  jle .add_nat_skip_rep_mov

  shl rcx, 2
  rep movsq
.add_nat_skip_rep_mov:
  pop rbp
  ret

.add_nat_small_big_carry:
  sub rcx, rsi
.add_nat_small_big_carry_loop:
  dec rcx
  js .add_nat_carry_exit
  add r8, 1
  adc r9, 0
  adc r10, 0
  adc r11, 0
  mov [rdi], r8
  mov [rdi+8], r9
  mov [rdi+16], r10
  mov [rdi+24], r11
  mov r8, [rdi+rdx+32]
  mov r9, [rdi+rdx+40]
  mov r10, [rdi+rdx+48]
  mov r11, [rdi+rdx+56]
  lea rdi, [rdi+32]
  jc .add_nat_small_big_carry_loop
  jmp .add_nat_small_big_copy

#endif
