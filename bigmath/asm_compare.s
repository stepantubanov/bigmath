.intel_syntax noprefix

#ifdef BIGMATH_ASM_X86_64

#
#  s32 compare_nat(const place_t* a, const place_t* b, u64 size);
#
#  rdi: a
#  rsi: b
#  rdx: size (< 32 bit)
#
.p2align 4
.global __ZN7bigmath8internal11compare_natEPKNS_7place_tES3_m
__ZN7bigmath8internal11compare_natEPKNS_7place_tES3_m:
  shl edx, 5
  add rdi, rdx
  add rsi, rdx
  neg rdx
  xor eax, eax

.p2align 4
L_compare_nat_loop:
  vmovups ymm0, [rdi+rdx]
  vpcmpeqb ymm1, ymm0, [rsi+rdx]
  vpmovmskb ecx, ymm1
  xor ecx, -1
  jnz L_compare_nat_finish

  add rdx, 32
  jnz L_compare_nat_loop

  vzeroupper
  ret

L_compare_nat_finish:
  tzcnt ecx, ecx
  add rdx, rcx
  movzx eax, byte ptr [rdi+rdx]
  movzx ecx, byte ptr [rsi+rdx]
  sub eax, ecx
  vzeroupper
  ret

#endif
