# \arg %rsi code_tickle_address
#             L -> bit 0: whether to write to data address
#             L -> bit 4: whether to add cycle delay
# \arg %rdx data_tickle_address
# \arg %rbp stack1_tickle_address
# \arg %rbx stack2_tickle_address
# \arg %rdi cr3_code_byte_address
.ct_enable_aexnotify:
    mov     RSVD_AEXNOTIFY_ADDRESS_OFFSET(%rsp), %rax
    movb    $1, (%rax)
__ct_mitigation_begin:
    lfence
.ct_check_write:
    movl    $63, %ecx
    shlx    %rcx, %rsi, %rcx
    jrcxz   .ct_clear_low_bits_of_rdx
    lea     -1(%rsi), %rsi
    movb    (%rdx), %al
    movb    %al, (%rdx)
.ct_clear_low_bits_of_rdx:
    movl    $12, %ecx
    shrx    %rcx, %rdx, %rdx
    shlx    %rcx, %rdx, %rdx
.ct_check_execute:
    call    *%rdi
    mov     $0x1000, %ecx
.ct_warm_caches_and_tlbs:
    lea     -0x40(%ecx), %ecx
    mov     (%rsi, %rcx), %eax
    mov     (%rbp, %rcx), %eax
    mov     (%rbx, %rcx), %eax
    mov     (%rdx, %rcx), %eax
    jrcxz   .ct_restore_state
    jmp     .ct_warm_caches_and_tlbs
.ct_restore_state:
    movzx   %sil, %ecx
    mov     RSVD_REDZONE_WORD_OFFSET(%rsp), %rdi
    mov     %rdi, -SE_WORDSIZE(%rsp)
    mov     RSVD_RDI_OFFSET(%rsp), %rdi
    mov     RSVD_RSI_OFFSET(%rsp), %rsi
    mov     RSVD_RBP_OFFSET(%rsp), %rbp
    mov     RSVD_RBX_OFFSET(%rsp), %rbx
    mov     RSVD_RDX_OFFSET(%rsp), %rdx
    mov     RSVD_RAX_OFFSET(%rsp), %rax
    jrcxz  .ct_restore_rcx
    CYCLE_DELAY 20, %rsp
.ct_restore_rcx:
    # Cache attack simulation
    mov     cache_access_counter(%rip), %rcx
    jrcxz   .is_null_pointer
    incl    (%rcx)
.is_null_pointer:
    mov     RSVD_RCX_OFFSET(%rsp), %rcx
    jmp     *RSVD_RIP_OFFSET(%rsp)
