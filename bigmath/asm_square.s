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
  mov rcx, rsi
  mov r8d, edx
  jmp __ZN7bigmath8internal7mul_natEPNS_7place_tEPKS1_mS4_m

#endif
