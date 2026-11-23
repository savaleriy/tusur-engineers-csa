CC      = gcc
CFLAGS  := -O2 -g -std=c17 -Wall -Wextra -Wpedantic
LDLIBS  := -lm
PYTHON  ?= python3
BUILD   := build

.PHONY: all check clean help

all: heat

heat: heat.c
	$(CC) $(CFLAGS) heat.c -o $@ $(LDLIBS)

check: heat | $(BUILD)
	@status=0; \
	./heat --n 512 --steps 100 --out $(BUILD)/512x100.txt; \
	./heat --n 2048 --steps 200 --out $(BUILD)/2048x200.txt; \
	$(PYTHON) check.py expected/512x100.txt $(BUILD)/512x100.txt || status=1; \
	$(PYTHON) check.py expected/2048x200.txt $(BUILD)/2048x200.txt || status=1; \
	exit $$status

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf heat $(BUILD)

help:
	@echo "make         - собрать heat"
	@echo "make check   - сравнить оба прогона с expected/ (исходная версия не проходит)"
	@echo "make clean   - удалить heat и build/"
