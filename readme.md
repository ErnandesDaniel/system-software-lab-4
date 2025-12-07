
Установить Visual Studio Build Tools (только инструменты, без IDE)
https://visualstudio.microsoft.com/ru/visual-cpp-build-tools/

Разработка классических приложений на С++

Узнайте точный путь к vcvars64.bat

Обычно путь вида

C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat


Настроить терминал в CLion:

Шаг 1: Откройте настройки CLion
Нажмите File → Settings (или Ctrl+Alt+S)

Шаг 2: Перейдите в настройки терминала
В меню слева: Tools → Terminal

Шаг 3: Замените Shell path на эту строку

cmd /k "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

Настройка CLion:

Edit Configurations

program arguments - Конфигурация запуска основного файла программы:
input.mylang ast.mmd cfg assembler-code

Working directory - рабочая директория (корень проекта):
C:/Users/DN3672/CLionProjects/system-software-lab-4

Скомпилировать главный файл в объектный файл windows с отладочной информацией в формате CodeView (CV) можно через команду:
nasm -g -F cv8 -f win64 main.asm -o main.obj

Для линковки и получения исполняемого файла можно использовать:
link /debug /subsystem:console /entry:main main.obj ucrt.lib

/debug — создать .pdb-файл
/subsystem:console — консольное приложение
/entry:main — точка входа (ваша метка main)
ucrt.lib — библиотека для printf, puts и т.д.

Запустить программу можно через
.\main.exe

Посмотреть результат можно через:
echo $LASTEXITCODE