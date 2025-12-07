
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

Настройка CLion:

Edit Configurations

program arguments - Конфигурация запуска основного файла программы:
input.mylang ast.mmd cfg assembler-code

Working directory - рабочая директория (корень проекта):
C:/Users/DN3672/CLionProjects/system-software-lab-4

Скомпилировать главный файл в объектный файл windows с отладочной информацией в формате CodeView (CV) можно через команду:
nasm -g -F cv8 -f win64 main.asm -o main.obj

Для линковки и получения исполняемого файла можно использовать:
clang -g -gcodeview -o main.exe main.obj

Запустить программу можно через
.\main.exe

Провести деббагинг можно через официальный инструмент:
cdb -lines main.exe

Выполянем:
x main!*main*

найти строку вида:
00007ff7`7bce7390 main!main (main)

устанавливаем brackpoint вида:
bp 00007ff7`7bce7390

запускаем программу
g


Посмотреть результат можно через:
echo $LASTEXITCODE