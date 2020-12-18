## Описание
Реализация алгоритма CYK, проверяющего, выводимо ли слово в данной КС-грамматике.  
## Сборка
##### Обычная сборка
```
mkdir CMake && cd CMake
cmake ..
make
```
##### Code coverage
```
mkdir CMake && cd CMake
cmake -DCOVERAGE=true ..
make CoverageReport
```
Результат сохраняется в `Build`.
