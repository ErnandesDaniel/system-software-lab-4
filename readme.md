

Настройка CLion:

Edit Configurations

program arguments - Конфигурация запуска основного файла программы:
input.mylang ast.mmd cfg assembler-code

Working directory - рабочая директория (корень проекта):
C:/Users/DN3672/CLionProjects/system-software-lab-4


Использование стандартного компилятора под Windows:

Установить LLVM, в него будет включен Clang компилятор

Установить Windows SDK

https://developer.microsoft.com/ru-ru/windows/downloads/windows-sdk/

Добавить cdb в PATH:

# Путь к x64-отладчикам
$debuggerPath = "C:\Program Files (x86)\Windows Kits\10\Debuggers\x64"

# Добавить в PATH текущего пользователя
[Environment]::SetEnvironmentVariable(
    "Path",
    [Environment]::GetEnvironmentVariable("Path", "User") + ";$debuggerPath",
    "User"
)

Проверить, что установлен:

cdb -version

Скомпилировать главный файл в объектный файл windows с отладочной информацией в формате CodeView (CV) можно через команду:
nasm -g -F cv8 -f win64 main.asm -o main.obj

Для линковки и получения исполняемого файла можно использовать:
clang -g -gcodeview -o main.exe main.obj

Запустить программу можно через
.\main.exe

.\program.exe


Посмотреть результат можно через:
echo $LASTEXITCODE

Провести деббагинг можно через официальный инструмент:
cdb -lines main.exe

Выполянем:
x main!*main*

найти строку вида:
00007ff7`7bce7390 main!main (main)

устанавливаем brackpoint вида:
bp 00007ff7`7bce7390

Проверяем, что точка установлена:
bl

ожидаем что-то вида:
0 e 00007ff7`b76c7390     0001 (0001)  0:**** main!main

запускаем программу
g

Ожидаем что-то вроде:
Breakpoint 0 hit
main!main:
00007ff7`b76c7390 55              push    rbp
0:000>

Можно увидеть весь код в виде машинных инструкций:
u . L20

Выполнить шаг вперед (если функция, то зайти внутрь ее потока выполения):
t

Выполнить шаг вперед (если функция, то выполнить польностью):
p


Найти строку вида (str_0):
00007ff7`b76c7398 488d05615c0900  lea     rax,[main!str_0 (00007ff7`b775d000)]

Показать содержимое переменной:
da 00007ff7`b775d000

Посмотреть регистры можно через:
r





мне нужно для каждой инструкции из исходников получать номер строки инстркции

и потом ее записывать в .debug_line в ассемблере

нужно следовать правилу:

Для каждой исходной строки, содержащей исполняемый код, укажите адрес первой инструкции, сгенерированной для этой строки.

создавайте метки в ассемблере для начала каждой строки:


main:
push rbp
mov rbp, rsp
sub rsp, 168

; --- строка 7: s = "..." ---
.line_7:
lea rax, [str_0]
mov [rbp + -8], rax
; ... (копии и т.д.)

; --- строка 9: puts(s) ---
.line_9:
mov rcx, [rbp + -16]
sub rsp, 32
call puts
add rsp, 32

; --- строка 11: c = getchar() ---
.line_11:
sub rsp, 32
call getchar
add rsp, 32
mov [rbp + -48], eax

; --- строка 12: putchar(c) ---
.line_12:
mov eax, [rbp + -48]
mov [rbp + -56], eax
mov ecx, [rbp + -56]
sub rsp, 32
call putchar
add rsp, 32

; ... и так далее

section .debug_line
dq .line_7,   dd 7
dq .line_9,   dd 9
dq .line_11,  dd 11
dq .line_12,  dd 12
dq .line_13,  dd 13
dq .line_15,  dd 15
dq .line_17,  dd 17
dq .line_19,  dd 19



