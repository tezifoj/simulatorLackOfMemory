# Makefile
# Сборка симулятора: .c компилируем через gcc, .asm через nasm,
# после линковки собираем всё в один исполняемый файл vmsim

CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude   # предупреждения, отладки, путь к заголовкам
ASM = nasm
ASMFLAGS = -f elf64 -g -F dwarf

all: vmsim

# линкуем объектные файлы в программу
vmsim: build/main.o build/addr.o
	$(CC) $(CFLAGS) -o $@ $^

# сборка для C-файлов: src/x.c -> build/x.o
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# сборка для asm-файлов: src/x.asm -> build/x.o
build/%.o: src/%.asm | build
	$(ASM) $(ASMFLAGS) $< -o $@

# создаём папку для объектных файлов
build:
	mkdir -p build

clean:
	rm -rf build vmsim