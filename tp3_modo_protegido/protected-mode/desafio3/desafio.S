.code16
.global _start

_start:
    cli
    lgdt gdt_descriptor

    mov %cr0, %eax
    or $1, %eax
    mov %eax, %cr0

    ljmp $0x08, $protected_mode

.code32
protected_mode:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    movl $0xDEADBEEF, (0x0)

hang:
    jmp hang

.align 4
gdt_start:
    .quad 0x0000000000000000

    .word 0xffff, 0x0000, 0x9a00, 0x00cf

    .word 0xffff, 0x0000, 0x9000, 0x00cf
gdt_end:

gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .long gdt_start
