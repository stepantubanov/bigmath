.intel_syntax noprefix

#ifdef BIGMATH_ASM_X86_64

#
#  bool add_word(place_t* nat, u64 nat_size, u64 word);
#
#  rdi: nat
#  rsi: nat_size (< 32 bit)
#  rdx: word
#
.p2align 4
.global __ZN7bigmath8internal8add_wordEPNS_7place_tEmm
__ZN7bigmath8internal8add_wordEPNS_7place_tEmm:
  xor eax, eax
  add [rdi], rdx
  jc .L_add_word_carry
  ret
.L_add_word_carry:
  add esi, esi
  inc qword ptr [rdi+8]
  jz .L_add_word_carry_loop
  ret
.L_add_word_carry_loop:
  add rdi, 16
  dec esi
  jz .L_add_word_finalize

  mov rcx, [rdi]
  mov rdx, [rdi+8]
  add rcx, 1
  adc rdx, 0
  mov [rdi], rcx
  mov [rdi+8], rdx
  jc .L_add_word_carry_loop
  ret

.L_add_word_finalize:
  # At this point ESI and EAX are zero.
  inc eax
  mov qword ptr [rdi], rax
  mov qword ptr [rdi+8], rsi
  mov qword ptr [rdi+16], rsi
  mov qword ptr [rdi+24], rsi
  ret

#
#  bool add_nat(place_t* nat, u64 nat_size, const place_t* other, u64 other_size);
#
#  rdi: nat
#  rsi: nat_size (< 32 bit)
#  rdx: other
#  rcx: other_size (< 32 bit)
#
.p2align 4
.global __ZN7bigmath8internal7add_natEPNS_7place_tEmPKS1_m
__ZN7bigmath8internal7add_natEPNS_7place_tEmPKS1_m:
  mov eax, esi
  sub eax, ecx      # ESI = nat_size - other_size
  cmovb ecx, esi    # ECX = min(nat_size, other_size)

  xor esi, esi

.p2align 5
.L_add_nat_min_loop:
  # Iterate through the common amount of places (ESI) preserving
  # the carry flag between iterations.
  #
  # Experiments:
  # "adc [rdi], rsi" is significantly (30-40%) slower
  # for some reason. IDQ.ALL_MITE_CYCLES_ANY_UOPS is high and DSB is close
  # to zero. Adding alignment nops between "adc [mem], r64" makes DSB high
  # and MITE close to zero, but only ~10% perf. improvement.

  mov rsi, [rdx]
  mov r8, [rdx+8]
  adc rsi, [rdi]
  adc r8, [rdi+8]
  mov [rdi], rsi
  mov [rdi+8], r8
  mov rsi, [rdx+16]
  mov r8, [rdx+24]
  adc rsi, [rdi+16]
  adc r8, [rdi+24]
  mov [rdi+16], rsi
  mov [rdi+24], r8

  lea rdi, [rdi+32]
  lea rdx, [rdx+32]

  dec ecx
  jnz .L_add_nat_min_loop

  setc cl

.byte 0x66, 0x90    # 2-byte nop to avoid having "js" branch on 32-byte
                    # boundary.
  test eax, eax     # EAX < 0 indicates that "nat" is larger than "other".
  js .L_add_nat_lb

  test ecx, ecx     # ECX contains CF
  jnz .L_add_nat_carry_loop

  xor eax, eax
  ret

  # This is a basic one-per-iteration loop. It's not likely to
  # execute more than once.
.L_add_nat_carry_loop:
  test eax, eax
  jz .L_add_nat_carry_finish
  dec eax
  add rdi, 8
  inc qword ptr [rdi-8]
  jz .L_add_nat_carry_loop
  xor eax, eax
  ret
.L_add_nat_carry_finish:
  xor esi, esi
  inc eax       # EAX was zero, inc to make it 1.
  mov qword ptr [rdi], rax
  mov qword ptr [rdi+8], rsi
  mov qword ptr [rdi+16], rsi
  mov qword ptr [rdi+24], rsi
  ret

.L_add_nat_lb:
  add eax, eax        # EAX = negative count of 16-byte blocks left to process.
  sub rdx, rdi

  test ecx, ecx       # ECX contains CF flag.
  jnz .L_add_nat_lb_carry_loop

.L_add_nat_lb_copy:
  # EAX is guaranteed to be non zero here.
  cmp eax, -8
  jle .L_add_nat_lb_large_copy

.L_add_nat_lb_basic_copy_loop:
  vmovaps xmm0, [rdi+rdx]
  vmovaps [rdi], xmm0
  add rdi, 16
  inc eax
  jnz .L_add_nat_lb_basic_copy_loop
  vzeroupper
  ret

.L_add_nat_lb_large_copy:
  movsxd rax, eax
  neg rax

  shl eax, 4
  lea rcx, [rdi+rax-64]

  vmovaps xmm0, [rdi+rdx]
  vmovaps [rdi], xmm0
  add rdi, 16
  and rdi, -32

.p2align 5
.L_add_nat_lb_copy_64:
  vmovups ymm0, [rdi+rdx]
  vmovups ymm1, [rdi+rdx+32]
  vmovaps [rdi], ymm0
  vmovaps [rdi+32], ymm1
  add rdi, 64
  cmp rdi, rcx
  jbe .L_add_nat_lb_copy_64

  add rdx, rcx

  vmovaps xmm0, [rdx+16]
  vmovaps xmm1, [rdx+32]
  vmovaps xmm2, [rdx+48]
  vmovaps [rcx+16], xmm0
  vmovaps [rcx+32], xmm1
  vmovaps [rcx+48], xmm2
  vzeroupper
  xor eax, eax
  ret

.L_add_nat_lb_carry_loop: # Unlikely to loop more than once.
  test eax, eax
  jz .L_add_nat_carry_finish

  add rdi, 16
  inc eax

  mov rsi, [rdi+rdx-16]
  add rsi, 1
  mov [rdi-16], rsi

  mov rsi, [rdi+rdx-8]
  adc rsi, 0
  mov [rdi-8], rsi

  jc .L_add_nat_lb_carry_loop

  test eax, eax
  jnz .L_add_nat_lb_copy
  ret

#endif
