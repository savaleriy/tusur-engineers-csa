# Задание 12. Базовые АТД на C

!!! warning "WIP"
    Задание в работе: условия и код могут меняться.

> Модуль 12: [Базовые абстрактные типы данных](../modules/12-adt.md). Нужно: `gcc`, `make` не обязателен; для проверки утечек - ASan или valgrind.

## Цель

Реализовать четыре базовых АТД на C и убедиться, что они корректны, не текут и имеют обещанную сложность. Это подготовка к следующему модулю: там те же задачи решаются с оглядкой на кэш.

## Что сделать

### 1. Динамический массив

```c
typedef struct { int *data; size_t size, capacity; } vec;

void vec_init(vec *v);
void vec_free(vec *v);
int  vec_push(vec *v, int x);
int  vec_pop(vec *v, int *x);
int  vec_get(const vec *v, size_t i, int *x);
```

Требования: рост вдвое, `vec_free` освобождает память, `vec_get` проверяет границы.

### 2. Связный список

```c
typedef struct node { int value; struct node *next; } node;
typedef struct { node *head; size_t size; } list;

int  list_push_front(list *l, int x);
int  list_pop_front(list *l, int *x);
void list_free(list *l);
```

Дополнительно: функция `list_reverse`, разворачивающая список за один проход и без новой памяти.

### 3. Очередь на кольцевом буфере

```c
typedef struct { int *data; size_t capacity, head, size; } ring;

int ring_init(ring *q, size_t capacity);   /* capacity - степень двойки */
int ring_push(ring *q, int x);
int ring_pop(ring *q, int *x);
void ring_free(ring *q);
```

Проверьте, что после $10^6$ операций `push`/`pop` очередь не теряет элементы и не растет.

### 4. Хеш-таблица с цепочками

```c
typedef struct hmap hmap;   /* непрозрачный тип */

hmap *hmap_new(void);
void  hmap_free(hmap *m);
int   hmap_put(hmap *m, uint64_t key, int value);
int   hmap_get(const hmap *m, uint64_t key, int *value);
int   hmap_remove(hmap *m, uint64_t key);
```

Требования: перестройка при коэффициенте заполнения 0.7, корректное удаление, отсутствие утечек.

## Как проверить

1. Напишите `main` с проверками на граничных случаях: пустой массив, один элемент, много элементов, повторяющиеся ключи.
2. Соберите с санитайзерами:

```bash
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all \
    test.c vec.c list.c ring.c hmap.c -o test
./test
```

3. Сравните массив и список: сумма $10^6$ элементов, вставка в начало. Замерьте время и объясните разницу.
4. Проверьте хеш-таблицу на 100000 ключей: время вставки и поиска, число коллизий при разных `nbuckets`.

## Вопросы

1. Почему рост массива вдвое дает амортизированно $O(1)$, а рост на 1 - нет?
2. В каком случае список быстрее массива?
3. Зачем в кольцевом буфере степень двойки?
4. Почему при коэффициенте заполнения 0.9 хеш-таблица с цепочками замедляется и как это лечится?
5. Что в вашей реализации может утечь и как вы это проверили?
