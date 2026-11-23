CC       = gcc
RVCC     = riscv64-linux-gnu-gcc
CLANG    = clang

BUILD    := build
ASM_DIR  := $(BUILD)/asm
IR_DIR   := $(BUILD)/ir
CFLAGS   := -std=c17 -O2 -Wall -Wextra -Wpedantic

.PHONY: all asm heap ir run-ir clean help

all: asm heap

$(BUILD):
	mkdir -p $(BUILD)

$(ASM_DIR): | $(BUILD)
	mkdir -p $(ASM_DIR)

asm: $(ASM_DIR)
	$(CC) $(CFLAGS) -S -masm=intel -fno-asynchronous-unwind-tables src/asm03.c -o $(ASM_DIR)/asm03-x86-O2.s
	$(CC) $(CFLAGS) -O1 -S -masm=intel -fno-asynchronous-unwind-tables src/asm03.c -o $(ASM_DIR)/asm03-x86-O1.s
	$(CC) $(CFLAGS) -O0 -S -masm=intel -fno-asynchronous-unwind-tables src/asm03.c -o $(ASM_DIR)/asm03-x86-O0.s
	@if command -v $(RVCC) >/dev/null 2>&1; then \
		$(RVCC) -O2 -march=rv64gc -S -fno-asynchronous-unwind-tables src/asm03.c -o $(ASM_DIR)/asm03-rv64gc-O2.s; \
		$(RVCC) -O1 -march=rv64gc -S -fno-asynchronous-unwind-tables src/asm03.c -o $(ASM_DIR)/asm03-rv64gc-O1.s; \
	else \
		echo "skip RISC-V: $(RVCC) not found"; \
	fi

heap: | $(BUILD)
	$(CC) $(CFLAGS) -c src/heap.c -o $(BUILD)/heap.o

ir:
	bash scripts/ir.sh

run-ir: ir
	@echo "GCC GIMPLE:"; sed -n '1,60p' $(IR_DIR)/ir_demo.c.*.gimple
	@echo "GCC optimized:"; sed -n '1,80p' $(IR_DIR)/ir_demo.c.*.optimized
	@echo "LLVM IR:"; sed -n '1,40p' $(IR_DIR)/ir_demo.ll

clean:
	rm -rf $(BUILD)

help:
	@echo "make asm    - сгенерировать asm03-x86-*.s и asm03-rv64gc-*.s"
	@echo "make heap   - собрать heap.c в объектный файл"
	@echo "make ir     - дампы GIMPLE/optimized и LLVM IR для ir_demo.c"
	@echo "make run-ir - показать начало дампов"
	@echo "make clean  - удалить build/"
