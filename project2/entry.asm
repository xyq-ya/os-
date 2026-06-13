BITS 32;32位保护模式

global _start;函数入口点

global stack_top;栈顶

extern kmain

section .text
_start:
    mov esp, stack_top;	设置 C 语言运行需要的栈指针
    call kmain
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 8192;留出8192字节空间
stack_top:
