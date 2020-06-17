# AlFractal
Программа для рисования фракталов над алгебрами. Для вычислений используется длинная арифметика [GNU Multi-Precision Library (GMP)](https://gmplib.org/).

## Документация
В разработке.

### Сочетания клавиш
Сочетание | Описание 
---|---
`U` | Включить/выключить оверлей
`R` | Перерисовать фрактал
`i +` / `i -` | Увеличить/уменьшить число итераций
`b +` / `b -` | Увеличить/уменьшить число бит на число
`Wheel+` / `Wheel-` | Приблизить/отдалить камеру

## Начало работы
### Построение
Для построения проекта требуется CMake версии не ниже 3.7.2. Листинг команд, используемых для построения проекта из директории, находящейся на два уровня ниже файла CMakeLists.txt:
```
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
```

### Справка
Полноценная справка в разработке.

## Запланировано к реализации
### Документация
- [ ] Составление файла документации.
- [ ] Реализация вывода справки по ключу `--help` или `-h`.

### Вычисление
- [x] Распараллеливание вычислений (пока не так удобно, как хотелось бы).
- [ ] Перенос вычислений на GPU.
- [ ] Реализация метода обратных итераций.
- [ ] Задание алгебры и итеративной функции на этапе выполнения.
