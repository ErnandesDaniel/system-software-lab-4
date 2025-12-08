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
    sub rsp, 168
BB_0:
    lea rax, [str_0]
    mov [rbp + -8], rax
    mov rax, [rbp + -8]
    mov [rbp + -16], rax
    mov rcx, [rbp + -16]
    sub rsp, 32
    call puts
    add rsp, 32
    mov [rbp + -40], eax
    sub rsp, 32
    call getchar
    add rsp, 32
    mov [rbp + -48], eax
    mov eax, [rbp + -48]
    mov [rbp + -56], eax
    mov ecx, [rbp + -56]
    sub rsp, 32
    call putchar
    add rsp, 32
    mov [rbp + -80], eax
    mov eax, 10
    mov [rbp + -88], eax
    mov ecx, [rbp + -88]
    sub rsp, 32
    call putchar
    add rsp, 32
    mov [rbp + -96], eax
    lea rax, [str_1]
    mov [rbp + -104], rax
    mov eax, 10
    mov [rbp + -112], eax
    mov rcx, [rbp + -104]
    mov edx, [rbp + -112]
    sub rsp, 32
    call printf
    add rsp, 32
    mov [rbp + -120], eax
    mov eax, 10
    mov [rbp + -128], eax
    mov ecx, [rbp + -128]
    sub rsp, 32
    call putchar
    add rsp, 32
    mov [rbp + -136], eax
    mov eax, 27
    mov [rbp + -144], eax
    mov eax, [rbp + -144]
; Очистка стека и возврат
    leave       ; эквивалент: mov rsp, rbp; pop rbp
    ret         ; возвращаем eax как результат

section .data
str_0 db 72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 33, 46, 32, 69, 110, 116, 101, 114, 32, 115, 111, 109, 101, 32, 115, 121, 109, 98, 111, 108, 0
str_1 db 37, 100, 0
