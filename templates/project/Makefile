# Простой Makefile для проекта. CMake-сборка - в CMakeLists.txt.
#
#   make            собрать app
#   make test       собрать и запустить тест
#   make run        собрать и запустить app
#   make cmake      собрать через CMake в build/cmake
#   make clean      удалить build

CC      ?= cc
CFLAGS  ?= -O2 -g -Wall -Wextra -Wpedantic -std=c11 -Iinclude
BUILD   := build

CORE    := $(BUILD)/core.o
APP     := $(BUILD)/app
TEST    := $(BUILD)/test_core

.PHONY: all test run cmake clean

all: $(APP)

$(BUILD):
	mkdir -p $@

$(CORE): src/core.c include/core.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(APP): src/main.c $(CORE) | $(BUILD)
	$(CC) $(CFLAGS) src/main.c $(CORE) -o $@

$(TEST): tests/test_core.c $(CORE) | $(BUILD)
	$(CC) $(CFLAGS) tests/test_core.c $(CORE) -o $@

test: $(TEST)
	./$(TEST)

run: $(APP)
	./$(APP)

cmake:
	cmake -S . -B $(BUILD)/cmake
	cmake --build $(BUILD)/cmake
	ctest --test-dir $(BUILD)/cmake --output-on-failure

clean:
	rm -rf $(BUILD)
