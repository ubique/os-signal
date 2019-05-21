### Usage:

+ `./run-tests.sh` - compile and run tests
+ `make clean` - clean the mess

##### Extra usage just in case:

+ `make` - compile handler sources
+ `make tests` - compile tests

---

# Обработка сигналов

Необходимо написать обработчик сигнала SIGSEGV. Обработчик должен:
 * Дампить значения general purpose регистров, соответствуюших моменту падения
 * Дампить память поблизости от адреса, по которому произошло нарушение защиты памяти

Стоит быть готовым, что:
 * Адрес, по которому был сгенерирован SIGSEGV - NULL
 * Адрес, по которому был сгенерирован SIGSEGV - находится на границе валидной памяти и нет

## Что может помочь при выполнении задания?
 * man 2 sigaction
 * man 2 getcontext
