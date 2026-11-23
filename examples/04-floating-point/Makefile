CC       = gcc
CFLAGS   := -std=gnu17 -O2 -Wall -Wextra -Wpedantic
LDLIBS   := -lm
PYTHON   ?= python3
RVCC     ?= riscv64-linux-gnu-gcc
QEMU     ?= qemu-riscv64

BUILD    := build
SRCS     := $(wildcard src/*.c)
BINS     := $(patsubst src/%.c,$(BUILD)/%,$(SRCS))

.PHONY: all run python fma subnormal riscv run-riscv clean help

all: $(BINS)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) $< $(LDLIBS) -o $@

# clock_gettime требует POSIX-макрос
$(BUILD)/07-subnormal: src/07-subnormal.c | $(BUILD)
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L $< $(LDLIBS) -o $@

# та же программа без векторизации: классические 115x
$(BUILD)/07-subnormal-scalar: src/07-subnormal.c | $(BUILD)
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -fno-tree-vectorize $< $(LDLIBS) -o $@

# FMA-вариант: сравнить с базовым 06-fma
$(BUILD)/06-fma-mfma: src/06-fma.c | $(BUILD)
	$(CC) $(CFLAGS) -mfma -mavx2 $< $(LDLIBS) -o $@

fma: $(BUILD)/06-fma $(BUILD)/06-fma-mfma
subnormal: $(BUILD)/07-subnormal $(BUILD)/07-subnormal-scalar

run: $(BINS)
	@for b in $(BINS); do \
		echo "===== $$b ====="; \
		./$$b; \
	done

run-fma: fma
	@echo "base:"; ./$(BUILD)/06-fma
	@echo "-mfma -mavx2:"; ./$(BUILD)/06-fma-mfma

run-subnormal: subnormal
	@echo "vectorized:"; ./$(BUILD)/07-subnormal
	@echo "scalar:"; ./$(BUILD)/07-subnormal-scalar

riscv: | $(BUILD)
	@if command -v $(RVCC) >/dev/null 2>&1; then \
		$(RVCC) -O2 -march=rv64gc -static src/08-fp-examples.c $(LDLIBS) -o $(BUILD)/08-fp-examples-rv; \
		$(RVCC) -O2 -march=rv64gc -static src/06-fma.c $(LDLIBS) -o $(BUILD)/06-fma-rv; \
	else \
		echo "skip RISC-V: $(RVCC) not found"; \
	fi

run-riscv: riscv
	@if command -v $(QEMU) >/dev/null 2>&1; then \
		echo "fp_examples:"; $(QEMU) ./$(BUILD)/08-fp-examples-rv; \
		echo "fma:"; $(QEMU) ./$(BUILD)/06-fma-rv; \
	else \
		echo "skip QEMU: $(QEMU) not found"; \
	fi

python:
	$(PYTHON) python/fp_fields.py
	$(PYTHON) python/fp_exact.py
	$(PYTHON) python/fp_float32.py
	$(PYTHON) python/fp_limits.py

clean:
	rm -rf $(BUILD)

help:
	@echo "make          - собрать все C-программы"
	@echo "make run      - запустить все C-программы"
	@echo "make fma      - собрать базовый и -mfma варианты 06-fma"
	@echo "make run-fma  - запустить оба варианта FMA"
	@echo "make subnormal - собрать векторный и скалярный варианты"
	@echo "make run-subnormal - запустить оба варианта"
	@echo "make python   - запустить Python-скрипты"
	@echo "make riscv    - собрать статические бинарники для RISC-V"
	@echo "make run-riscv - запустить их в qemu-riscv64"
	@echo "make clean    - удалить build/"
