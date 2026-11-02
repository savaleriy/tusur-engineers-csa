# Шаблон проекта на C

Минимальный проект для заданий курса: библиотека `core`, программа `app`,
тест `test_core` и две системы сборки - Makefile и CMake.

## Сборка

```bash
make            # build/app
make test       # build/test_core и запуск
make run        # build/app и запуск
make cmake      # CMake-сборка в build/cmake + ctest
make clean
```

То же через CMake напрямую:

```bash
cmake -S . -B build/cmake
cmake --build build/cmake
ctest --test-dir build/cmake --output-on-failure
```

## Структура

```text
include/core.h      интерфейс библиотеки
src/core.c          реализация
src/main.c          демонстрационная программа
tests/test_core.c   проверка
CMakeLists.txt      CMake
Makefile            обычная сборка
```

## C++

Если задание на C++, замените `.c` на `.cpp`, в `CMakeLists.txt`
укажите `LANGUAGES CXX`, а в Makefile - `CXX` и `-std=c++20`.
Интерфейс `core.h` можно оставить общим.
