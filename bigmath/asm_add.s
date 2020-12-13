.intel_syntax noprefix
.global _ZN7bigmath8internal8add_wordEPmmm, _ZN7bigmath8internal7add_natEPmmPKmm

#ifdef BIGMATH_ASM_X86_64

/*
  uint64_t add_word(uint64_t* nat, uint64_t nat_size, uint64_t word)

  rdi: nat
  rsi: nat_size
  rdx: word
*/
_ZN7bigmath8internal8add_wordEPmmm:
  mov rax, rsi
  ret

/*
  uint64_t add_nat(uint64_t* nat, uint64_t nat_size, const uint64_t* other,
                   uint64_t other_size)
  rdi: nat
  rsi: nat_size
  rdx: other
  rcx: other_size
*/
_ZN7bigmath8internal7add_natEPmmPKmm:
  mov rax, rsi
  ret

#endif
