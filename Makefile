# Makefile
# Сборка симулятора: .c компилируем через gcc, .asm через nasm,
# после линковки собираем всё в один исполняемый файл vmsim

CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude   # предупреждения, отладки, путь к заголовкам
ASM = nasm
ASMFLAGS = -f elf64 -g -F dwarf

all: vmsim gen_trace

# линкуем объектные файлы в программу
vmsim: build/main.o build/addr.o build/memory.o build/utils.o build/trace.o build/view.o
	$(CC) $(CFLAGS) -o $@ $^

# сборка для C-файлов: src/x.c -> build/x.o
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# сборка для asm-файлов: src/x.asm -> build/x.o
build/%.o: src/%.asm | build
	$(ASM) $(ASMFLAGS) $< -o $@

# как собирать тесты: tests/x.c -> build/x.o
build/%.o: tests/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# тесты: всё, кроме main.o и view.o
TEST_OBJS = build/test_main.o build/addr.o build/memory.o build/utils.o build/trace.o

run_tests: $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# make test - собрать и запустить тесты
test: run_tests
	./run_tests

# как собирать утилиты: tools/x.c -> build/x.o
build/%.o: tools/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# генератор трасс
gen_trace: build/gen_trace.o
	$(CC) $(CFLAGS) -o $@ $^

# создаём папку для объектных файлов
build:
	mkdir -p build

clean:
	rm -rf build vmsim run_tests gen_trace

.PHONY: all clean test