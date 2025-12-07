bits 64
default rel
section .text

global main
extern puts
extern getchar
extern putchar
extern putchar
extern printf
extern putchar

main:
    push rbp
    mov rbp, rsp
    sub rsp, 88
BB_0:
    lea rax, [str_0]
    mov [rbp + -8], rax
    mov rax, [rbp + -8]
    mov [rbp + -12], rax
    mov rcx, [rbp + -12]
    sub rsp, 32
    call puts
    add rsp, 32
    mov [rbp + -24], eax
    sub rsp, 32
    call getchar
    add rsp, 32
    mov [rbp + -28], eax
    mov eax, [rbp + -28]
    mov [rbp + -32], eax
    mov ecx, [rbp + -32]
    sub rsp, 32
    call putchar
    add rsp, 32
    mov [rbp + -44], eax
    mov eax, 10
    mov [rbp + -48], eax
    mov ecx, [rbp + -48]
    sub rsp, 32
    call putchar
    add rsp, 32
    mov [rbp + -52], eax
    lea rax, [str_1]
    mov [rbp + -56], rax
    mov eax, 10
    mov [rbp + -60], eax
    mov rcx, [rbp + -56]
    mov edx, [rbp + -60]
    sub rsp, 32
    call printf
    add rsp, 32
    mov [rbp + -64], eax
    mov eax, 10
    mov [rbp + -68], eax
    mov ecx, [rbp + -68]
    sub rsp, 32
    call putchar
    add rsp, 32
    mov [rbp + -72], eax
    mov eax, 27
    mov [rbp + -76], eax
    mov eax, [rbp + -76]
; Очистка стека и возврат
    leave       ; эквивалент: mov rsp, rbp; pop rbp
    ret         ; возвращаем eax как результат

section .data
str_0 db 72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 33, 46, 32, 69, 110, 116, 101, 114, 32, 115, 111, 109, 101, 32, 115, 121, 109, 98, 111, 108, 0
str_1 db 37, 100, 0
