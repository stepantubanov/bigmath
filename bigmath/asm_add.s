.intel_syntax noprefix

#ifdef __GNUC__

#ifdef __clang__
#define BIGMATH_INTERNAL_ADD_WORD __ZN7bigmath8internal8add_wordEPmmm
#define BIGMATH_INTERNAL_ADD_NAT  __ZN7bigmath8internal7add_natEPmmPKmm
#else
#define BIGMATH_INTERNAL_ADD_WORD _ZN7bigmath8internal8add_wordEPmmm
#define BIGMATH_INTERNAL_ADD_NAT  _ZN7bigmath8internal7add_natEPmmPKmm
#endif

#endif

.global BIGMATH_INTERNAL_ADD_WORD, BIGMATH_INTERNAL_ADD_NAT


#ifdef BIGMATH_ASM_X86_64

/*
  u64 add_word(u64* nat, u64 nat_size, u64 word)

  rdi: nat
  rsi: nat_size
  rdx: word
*/
BIGMATH_INTERNAL_ADD_WORD:
  mov rax, rsi
  ret

/*
  u64 add_nat(u64* nat, u64 nat_size, const u64* other, u64 other_size)

  rdi: nat
  rsi: nat_size
  rdx: other
  rcx: other_size
*/
BIGMATH_INTERNAL_ADD_NAT:
  mov rax, rsi
  ret

#endif
