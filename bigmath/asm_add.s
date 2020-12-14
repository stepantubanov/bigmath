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
  push rbp
  mov rbp, rsp
  mov rax, rsi
  add [rdi], rdx
  jb .add_word_carry_loop
  pop rbp
  ret
.add_word_carry_loop:
  dec rsi
  jz .add_word_finalize
  lea rdi, [rdi+8]
  inc qword ptr [rdi]
  jz .add_word_carry_loop
  pop rbp
  ret
.add_word_finalize:
  mov qword ptr [rdi+8], 1
  inc rax
  pop rbp
  ret

/*
  u64 add_nat(u64* nat, u64 nat_size, const u64* other, u64 other_size)

  rdi: nat
  rsi: nat_size (< 2**32)
  rdx: other
  rcx: other_size (< 2**32)
*/
BIGMATH_INTERNAL_ADD_NAT:
  push rbp
  mov rbp, rsp
  mov [rsp-8], rbx

  cmp rcx, rsi
  mov r8, rsi
  mov r9, rcx
  mov r10, rdx
  cmovb r8, rcx       # r8 = min(nat_size, other_size)
  cmovb r9, rsi       # r9 = max(nat_size, other_size)
  cmovb r10, rdi
  mov [rsp-16], r9    # store max size on stack
  mov [rsp-24], r10   # store max ptr on stack
  sub r9, r8          # r9 = max_size - min_size

  mov rsi, r8
  and r8d, 3          # r8 = min_size % 4
  shr rsi, 2          # rsi = min_size / 4

  xor rax, rax

  mov rcx, r8
  test rsi, rsi          # min_size/4 == 0 and set CF=0
  jz .add_nat_min_loop1
  mov rcx, rsi

.add_nat_min_loop4_body:
  mov rsi, [rdx+rax*8]
  mov rbx, [rdx+rax*8+8]
  mov r10, [rdx+rax*8+16]
  mov r11, [rdx+rax*8+24]
  adc [rdi+rax*8], rsi
  adc [rdi+rax*8+8], rbx
  adc [rdi+rax*8+16], r10
  adc [rdi+rax*8+24], r11
  lea rax, [rax+4]
  dec rcx
  jnz .add_nat_min_loop4_body

  mov rcx, r8
  dec rcx
  js .add_nat_max_loop
  inc rcx

.add_nat_min_loop1:
  mov rsi, [rdx+rax*8]
  adc [rdi+rax*8], rsi
  inc rax
  dec rcx
  jnz .add_nat_min_loop1

.add_nat_max_loop:
  mov rsi, [rsp-24]    # rsi = max ptr
  mov rcx, r9          # rcx = max_size - min_size
  jnc .add_nat_max_loop_entry

.add_nat_carry_loop:
  test rcx, rcx
  jz .add_nat_finish_with_carry
  dec rcx
  inc rax

  mov rbx, [rsi+rax*8-8]
  inc rbx
  mov qword ptr [rdi+rax*8-8], rbx
  jz .add_nat_carry_loop

.add_nat_max_loop_entry:
  test rcx, rcx
  jz .add_nat_finish   # no more places to copy

  cmp rsi, rdi
  je .add_nat_finish   # max ptr == destination, nothing to copy

  lea rsi, [rsi+rax*8]
  lea rdi, [rdi+rax*8]
  xor rax, rax

  cmp ecx, 8               # use movaps for size <= 64 bytes
  jle .add_nat_copy_loop

  rep movsq
  jmp .add_nat_finish

.add_nat_copy_loop:
  vmovups xmm0, [rsi+rax*8]
  vmovups [rdi+rax*8], xmm0
  lea rax, [rax+2]
  sub ecx, 2
  jg .add_nat_copy_loop

.add_nat_finish:
  mov rax, [rsp-16]
  mov rbx, [rsp-8]
  pop rbp
  ret

.add_nat_finish_with_carry:
  mov qword ptr [rdi+rax*8], 1
  mov rbx, [rsp-8]
  inc rax
  pop rbp
  ret

#endif
