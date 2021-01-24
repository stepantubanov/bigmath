.intel_syntax noprefix

#ifdef BIGMATH_ASM_X86_64

#
#  bool add_word(place_t* nat, u64 nat_size, u64 word);
#
#  rdi: nat
#  rsi: nat_size (< 2 ** 31)
#  rdx: word
#
.p2align 4
.global __ZN7bigmath8internal8add_wordEPNS_7place_tEmm
__ZN7bigmath8internal8add_wordEPNS_7place_tEmm:
  xor eax, eax
  add rdx, [rdi]            # Add given word with the first word in nat.
  mov [rdi], rdx
  jc L_add_word_carry       # Unlikely branch.
  ret
L_add_word_carry:
  add esi, esi              # ESI is count of 32-byte blocks. We want count
                            # of 16-byte blocks for the loop below.
  mov rdx, [rdi+8]
  inc rdx
  mov [rdi+8], rdx
  jz L_add_word_carry_loop  # Unlikely branch. If result is zero (after adding
                            # 1) that means there was an overflow (carry).
  ret

L_add_word_carry_loop:
  add rdi, 16
  dec esi
  jz L_add_word_finalize    # Reached the end of the nat while there is still a
                            # carry. Append the carry to the end of nat.

  mov rcx, [rdi]
  mov rdx, [rdi+8]
  add rcx, 1
  adc rdx, 0
  mov [rdi], rcx
  mov [rdi+8], rdx
  jc L_add_word_carry_loop  # Unlikely branch.
  ret

L_add_word_finalize:
  inc eax                       # At this point ESI and EAX are zero.
  mov qword ptr [rdi], rax      # Use RAX, RSI instead of imm32 for smaller
  mov qword ptr [rdi+8], rsi    # code size.
  mov qword ptr [rdi+16], rsi
  mov qword ptr [rdi+24], rsi
  ret

#
#  bool add_nat(place_t* nat, u64 nat_size, const place_t* other, u64 other_size);
#
#  rdi: nat
#  rsi: nat_size (< 2 ** 27)
#  rdx: other
#  rcx: other_size (< 2 ** 27)
#
.p2align 4
.global __ZN7bigmath8internal7add_natEPNS_7place_tEmPKS1_m
__ZN7bigmath8internal7add_natEPNS_7place_tEmPKS1_m:
  mov eax, esi
  sub eax, ecx      # ESI = nat_size - other_size
  cmovb ecx, esi    # ECX = min(nat_size, other_size)

  xor esi, esi

.p2align 4
L_add_nat_min_loop:
  # Iterate through the common amount of places (ECX) preserving
  # the carry flag between iterations.
  #
  # NOTE:
  # "adc [rdi], rsi" is significantly (30-40%) slower than the version
  # below. For dense (adc [rdi], rsi) addition MITE uops is high and DSB
  # is close to zero. Adding multibyte nop makes MITE go away, but only
  # helps about 10% and the dense addition is still significantly slower.

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
  jnz L_add_nat_min_loop

  setc cl                 # ECX = CF for use in one of two carry loops below.

  test eax, eax           # EAX < 0 indicates that "nat" is larger than "other".
  js L_add_nat_lb

  test ecx, ecx             # ECX != 0 means there is a carry to process.
  jnz L_add_nat_carry_loop  # Unlikely branch.

  xor eax, eax
  ret

  # This is a basic one-per-iteration loop. It's not likely to
  # execute more than once.
L_add_nat_carry_loop:
  test eax, eax
  jz L_add_nat_carry_finish   # Reached the end of "nat" while adding
                              # carry. Append carry to the end.
  dec eax
  add rdi, 8
  inc qword ptr [rdi-8]
  jz L_add_nat_carry_loop     # Unlikely branch. Continue with the carry.
  xor eax, eax
  ret
L_add_nat_carry_finish:
  xor esi, esi
  inc eax                     # EAX was zero, inc to make it 1.
  mov qword ptr [rdi], rax    # Use RAX, RSI instead of imm32 for
  mov qword ptr [rdi+8], rsi  # smaller code size.
  mov qword ptr [rdi+16], rsi
  mov qword ptr [rdi+24], rsi
  ret

L_add_nat_lb:
  add eax, eax                  # EAX = negative count of 16-byte blocks
                                # left to process.
  sub rdx, rdi                  # RDX = "other" ptr relative to "nat".

  test ecx, ecx                 # ECX contains CF flag.
  jnz L_add_nat_lb_carry_loop   # Unlikely branch.

L_add_nat_lb_copy:
  cmp eax, -8                     # EAX is guaranteed to be non zero here due to
                                  # "js L_add_nat_lb" branch above.
  jbe L_add_nat_lb_large_copy

L_add_nat_lb_basic_copy_loop:
  vmovaps xmm0, [rdi+rdx]
  vmovaps [rdi], xmm0
  add rdi, 16
  inc eax
  jnz L_add_nat_lb_basic_copy_loop
  vzeroupper
  ret

L_add_nat_lb_large_copy:
  neg eax
  shl eax, 4

  lea rcx, [rdi+rax-48]

  vmovaps xmm0, [rdi+rdx]       # Copy first 16 bytes. May overlap with loop
  vmovaps [rdi], xmm0           # below. We use overlapping stores to avoid
                                # a branch on aligned/unaligned RDI ptr.

  add rdi, 16
  and rdi, -32

.p2align 4
L_add_nat_lb_copy_64:
  vmovups ymm0, [rdi+rdx]
  vmovups ymm1, [rdi+rdx+32]
  vmovaps [rdi], ymm0
  vmovaps [rdi+32], ymm1
  add rdi, 64
  cmp rdi, rcx
  jb L_add_nat_lb_copy_64

  vmovaps xmm0, [rcx+rdx]       # After the loop is done we have 0 bytes left to
  vmovups ymm1, [rcx+rdx+16]    # to copy if RDI was aligned. And we have 48
  vmovaps [rcx], xmm0           # bytes left ot copy if it wasn't aligned. We
  vmovups [rcx+16], ymm1        # unconditionally copy 48.

  vzeroupper
  xor eax, eax
  ret

L_add_nat_lb_carry_loop:
  test eax, eax
  jz L_add_nat_carry_finish     # Reached the end of "nat" while there is still
                                # a carry. Append the carry to the end.

  add rdi, 16
  inc eax

  mov rsi, [rdi+rdx-16]
  add rsi, 1
  mov [rdi-16], rsi

  mov rsi, [rdi+rdx-8]
  adc rsi, 0
  mov [rdi-8], rsi

  jc L_add_nat_lb_carry_loop    # Unlikely branch.

  test eax, eax
  jnz L_add_nat_lb_copy
  ret

#endif
