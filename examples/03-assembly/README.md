# Примеры к занятию 3: чтение ассемблера

Код к модулю 3: чтение листингов x86-64 и RISC-V, стек, ABI, рекурсия и IR.

## Быстрый старт

```bash
cd examples/03-assembly
make asm      # листинги x86-64 и RISC-V
make heap     # объектный файл для примера со стеком и кучей
make ir       # GIMPLE, optimized и LLVM IR
make run-ir   # показать начало дампов
make clean
```

Нужны `gcc`, `binutils` и, для RISC-V, `riscv64-linux-gnu-gcc`. `clang` нужен только для LLVM IR; без него шаг с LLVM пропускается.

## Файлы

| Файл | Что показывает |
|---|---|
| `src/asm03.c` | `sum`, `max2`, `max3`, `frame`, `fact`, `tail`, `apply`, `seven`, `leaf` - те же функции, что в заметке |
| `src/heap.c` | стек и куча в одной функции: `malloc`/`free`, локальный массив |
| `src/ir_demo.c` | `sum` и `fact` для дампов GIMPLE/optimized и LLVM IR |
| `scripts/ir.sh` | команды `gcc -fdump-tree-*` и `clang -emit-llvm` |

## Ассемблер

```bash
make asm
ls build/asm
```

Появятся:

- `asm03-x86-O2.s`, `asm03-x86-O1.s`, `asm03-x86-O0.s`;
- `asm03-rv64gc-O2.s`, `asm03-rv64gc-O1.s`, если установлен кросс-компилятор.

Ручные команды:

```bash
gcc -std=c17 -O2 -S -masm=intel -fno-asynchronous-unwind-tables src/asm03.c -o build/asm/asm03-x86-O2.s
gcc -std=c17 -O2 -S -masm=intel -fno-asynchronous-unwind-tables src/asm03.c -o - | sed -n '/^max2:/,/ret/p'
riscv64-linux-gnu-gcc -O2 -march=rv64gc -S src/asm03.c -o build/asm/asm03-rv64gc-O2.s
```

На GCC 16 символы в листинге могут быть в кавычках: `"max2":`. Это не ошибка, а способ защитить имя.

Что искать:

- `max2`: `cmovge` на x86 и `bge` на RISC-V;
- `fact`: на `-O2` нет `call fact`, на `-O0` есть;
- `seven`: седьмой аргумент в `8[rsp]` на x86 и в `a6` на RISC-V;
- `frame`: пролог, сохранение `rbp`/`rbx` или `ra`/`s0`/`s1`;
- `sum`: проверка `n <= 0` до цикла и указатель вместо индекса.

## IR

```bash
make ir
ls build/ir
```

GCC создаст:

- `ir_demo.c.007t.gimple` - GIMPLE до оптимизаций;
- `ir_demo.c.273t.optimized` - SSA после оптимизаций.

Номер прохода зависит от версии GCC, поэтому смотрите по маске:

```bash
ls build/ir/ir_demo.c.*t.*
sed -n '1,80p' build/ir/ir_demo.c.*.gimple
sed -n '1,100p' build/ir/ir_demo.c.*.optimized
```

В `.gimple` факториал еще рекурсивный:

```text
_1 = n + -1;
_2 = fact (_1);
D.2974 = n * _2;
```

В `.optimized` он уже цикл с `PHI` и `mult_acc`.

LLVM IR:

```bash
clang -O2 -S -emit-llvm src/ir_demo.c -o build/ir/ir_demo.ll
sed -n '1,60p' build/ir/ir_demo.ll
```

Ручные команды GCC:

```bash
gcc -O2 -fdump-tree-gimple src/ir_demo.c -c
gcc -O2 -fdump-tree-optimized src/ir_demo.c -c
gcc -O2 -fdump-tree-all src/ir_demo.c -c      # много файлов
gcc -O2 -fdump-rtl-all src/ir_demo.c -c       # низкий уровень, RTL
gcc -O2 -fdump-tree-optimized=/dev/stdout src/ir_demo.c -c
```

## Связь с заметкой

- `asm03.c` - разделы 4-14 заметки: `max2`, `fact`, `seven`, `sum`, `frame`, `apply`.
- `heap.c` - раздел 13 *Стек и куча*.
- `ir_demo.c` и `scripts/ir.sh` - дополнительный материал к разделу 9 *Читаем вывод компилятора*: IR показывает, что компилятор сделал до ассемблера.

## Ожидаемо

- `make asm` не должен падать; RISC-V-файлы появятся только при установленном кросс-компиляторе.
- `make heap` создает `build/heap.o`.
- `make ir` создает `.gimple`, `.optimized` и `ir_demo.ll`.
- В `.optimized` у `fact` нет рекурсивного вызова, у `sum` есть `PHI` и `ivtmp`.
