bits 64
default rel
section .text

global main
extern puts
extern getchar
extern putchar
extern printf

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
main_end:

section .rodata
str_0 db 72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 33, 46, 32, 69, 110, 116, 101, 114, 32, 115, 111, 109, 101, 32, 115, 121, 109, 98, 111, 108, 0
str_1 db 37, 100, 0

section .debug_str
dbg_str_main db 'main', 0
dbg_str_t1_0 db 't1', 0
dbg_str_s_1 db 's', 0
dbg_str_t1_2 db 't1', 0
dbg_str_s_3 db 's', 0
dbg_str_t2_4 db 't2', 0
dbg_str_t5_5 db 't5', 0
dbg_str_c_6 db 'c', 0
dbg_str_t5_7 db 't5', 0
dbg_str_c_8 db 'c', 0
dbg_str_t6_9 db 't6', 0
dbg_str_t9_10 db 't9', 0
dbg_str_t8_11 db 't8', 0
dbg_str_t11_12 db 't11', 0
dbg_str_t12_13 db 't12', 0
dbg_str_t10_14 db 't10', 0
dbg_str_t14_15 db 't14', 0
dbg_str_t13_16 db 't13', 0
dbg_str_t15_17 db 't15', 0

section .debug_info
    ; === Функция main ===
    dq dbg_str_main                 ; указатель на имя
    dq main                         ; старт
    dq main_end                     ; конец
    dd 0                          ; параметров: 0
    dd 18                         ; локальных: 18
    ; Переменная t1
    dq dbg_str_t1_0                    ; имя
    dd 1                            ; тип: string
    dd -8                           ; смещение
    ; Переменная s
    dq dbg_str_s_1                    ; имя
    dd 1                            ; тип: string
    dd -16                           ; смещение
    ; Переменная t1
    dq dbg_str_t1_2                    ; имя
    dd 1                            ; тип: string
    dd -24                           ; смещение
    ; Переменная s
    dq dbg_str_s_3                    ; имя
    dd 1                            ; тип: string
    dd -32                           ; смещение
    ; Переменная t2
    dq dbg_str_t2_4                    ; имя
    dd 0                            ; тип: int
    dd -40                           ; смещение
    ; Переменная t5
    dq dbg_str_t5_5                    ; имя
    dd 0                            ; тип: int
    dd -48                           ; смещение
    ; Переменная c
    dq dbg_str_c_6                    ; имя
    dd 0                            ; тип: int
    dd -56                           ; смещение
    ; Переменная t5
    dq dbg_str_t5_7                    ; имя
    dd 0                            ; тип: int
    dd -64                           ; смещение
    ; Переменная c
    dq dbg_str_c_8                    ; имя
    dd 0                            ; тип: int
    dd -72                           ; смещение
    ; Переменная t6
    dq dbg_str_t6_9                    ; имя
    dd 0                            ; тип: int
    dd -80                           ; смещение
    ; Переменная t9
    dq dbg_str_t9_10                    ; имя
    dd 0                            ; тип: int
    dd -88                           ; смещение
    ; Переменная t8
    dq dbg_str_t8_11                    ; имя
    dd 0                            ; тип: int
    dd -96                           ; смещение
    ; Переменная t11
    dq dbg_str_t11_12                    ; имя
    dd 1                            ; тип: string
    dd -104                           ; смещение
    ; Переменная t12
    dq dbg_str_t12_13                    ; имя
    dd 0                            ; тип: int
    dd -112                           ; смещение
    ; Переменная t10
    dq dbg_str_t10_14                    ; имя
    dd 0                            ; тип: int
    dd -120                           ; смещение
    ; Переменная t14
    dq dbg_str_t14_15                    ; имя
    dd 0                            ; тип: int
    dd -128                           ; смещение
    ; Переменная t13
    dq dbg_str_t13_16                    ; имя
    dd 0                            ; тип: int
    dd -136                           ; смещение
    ; Переменная t15
    dq dbg_str_t15_17                    ; имя
    dd 0                            ; тип: int
    dd -144                           ; смещение
