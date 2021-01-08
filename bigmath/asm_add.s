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
#  rsi: nat_size (< 32 bit)
#  rdx: word
#
BIGMATH_INTERNAL_ADD_WORD:
  xor eax, eax
  add qword ptr [rdi], rdx
  jc .L_add_word_carry
  ret

.L_add_word_carry:
  adc qword ptr [rdi+8], 0
  lea esi, [esi+esi]
  jc .L_add_word_carry_loop
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
  mov ecx, 1
  movd xmm0, ecx
  pxor xmm1, xmm1
  movaps [rdi], xmm0
  movaps [rdi+16], xmm1
  inc eax
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
#  upper bound for size vars is (32-5) bits or 134 Mb
#
BIGMATH_INTERNAL_ADD_NAT:
  mov eax, esi
  sub eax, ecx      # ESI = nat_size - other_size
  cmovb ecx, esi    # ECX = min(nat_size, other_size)

  xor esi, esi

  # Iterate through the common amount of places (ESI) preserving
  # the carry flag between iterations.
.align 4
.L_add_nat_min_loop:
  mov rsi, [rdi]
  adc rsi, [rdx]    # NOTE: "adc [rdi], rsi" would be more concise, but
  mov [rdi], rsi    # is significantly (30-40%) slower (tested on i7-9750h).

  mov rsi, [rdi+8]
  adc rsi, [rdx+8]
  mov [rdi+8], rsi

  mov rsi, [rdi+16]
  adc rsi, [rdx+16]
  mov [rdi+16], rsi

  mov rsi, [rdi+24]
  adc rsi, [rdx+24]
  mov [rdi+24], rsi

  lea rdi, [rdi+32]
  lea rdx, [rdx+32]

  dec ecx
  jnz .L_add_nat_min_loop

  setc cl

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
.L_add_nat_ret:
  ret

.L_add_nat_lb:
  neg eax             # EAX is initially negative, convert to positive.
  shl eax             # EAX = number of remaining 16-byte blocks.
  sub rdx, rdi

  test ecx, ecx       # ECX contains CF flag.
  jnz .L_add_nat_lb_carry_loop

.L_add_nat_lb_copy:
  xor esi, esi
  test edi, 16
  setnz sil             # ESI = 1 when RDI is not 32-byte aligned.

  test eax, eax
  setnz r8b             # R8 = 1 when EAX != 0
  and esi, r8d          # RDI is not-aligned AND EAX != 0
  jz .L_add_nat_lb_copy_64

  vmovups xmm0, [rdi+rdx]  # XMM copy to align data (or if only one block).
  vmovaps [rdi], xmm0
  add rdi, 16
  dec eax

.align 4
.L_add_nat_lb_copy_64:        # Copies 64 byte of data using aligned stores.
  cmp eax, 4                  # Alignment plays a big role here. Accounts for
  jb .L_add_nat_lb_copy_32    # about 40% gain in copy speed.

                              # Comparison for i7-9750h on the same test:
                              #
                              #   rep movsq              = 230 ns
                              #   movups YMM, unroll x 2 = 220 ns (aligned 16)
                              #   movups YMM, unroll x 2 = 157 ns (aligned 32)
                              #   movups XMM, unroll x 2 = 300 ns (aligned 16)

  vmovups ymm0, [rdi+rdx]
  vmovups ymm1, [rdi+rdx+32]
  vmovaps [rdi], ymm0
  vmovaps [rdi+32], ymm1
  add rdi, 64
  sub eax, 4
  jmp .L_add_nat_lb_copy_64

.L_add_nat_lb_copy_32:
  cmp eax, 2
  jb .L_add_nat_lb_copy_16
  vmovups ymm0, [rdi+rdx]
  vmovaps [rdi], ymm0
  add rdi, 32

.L_add_nat_lb_copy_16:
  test eax, eax
  jz .L_add_nat_ret_2
  vmovups xmm0, [rdi+rdx]
  vmovups [rdi], xmm0
.L_add_nat_ret_2:
  vzeroupper
  xor eax, eax
  ret

.L_add_nat_lb_carry_loop:
  test eax, eax
  jz .L_add_nat_carry_finish

  mov rsi, 1
  add rsi, [rdi+rdx]
  mov [rdi], rsi

  mov rsi, [rdi+rdx+8]
  adc rsi, 0
  mov [rdi+8], rsi

  lea rdi, [rdi+16]
  dec eax

  jnc .L_add_nat_lb_copy
  jmp .L_add_nat_lb_carry_loop

.L_add_nat_carry_finish:
  mov eax, 1
  movd xmm0, eax
  pxor xmm1, xmm1
  movups [rdi], xmm0
  movups [rdi+16], xmm1
  ret

#endif
