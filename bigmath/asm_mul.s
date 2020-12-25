.intel_syntax noprefix

#ifdef BIGMATH_ASM_X86_64

#ifdef __GNUC__

#ifdef __clang__
#define BIGMATH_INTERNAL_MUL_WORD __ZN7bigmath8internal8mul_wordEPvPKvmm
#define BIGMATH_INTERNLA_MUL_NAT __ZN7bigmath8internal7mul_natEPvPKvmS3_m
#else
#define BIGMATH_INTERNAL_MUL_WORD _ZN7bigmath8internal8mul_wordEPvPKvmm
#define BIGMATH_INTERNLA_MUL_NAT _ZN7bigmath8internal7mul_natEPvPKvmS3_m
#endif

#endif

.global BIGMATH_INTERNAL_MUL_WORD, BIGMATH_INTERNAL_MUL_NAT

#
#  u64 mul_word(void* _res, const void* _nat, u64 nat_size, u64 word)
#
#  rdi: _res
#  rsi: _nat
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

BIGMATH_INTERNAL_MUL_NAT:
  push rbp
  mov rbp, rsp

  pop rbp
  ret

#endif
