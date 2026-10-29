# Команды

Все команды запуска выполняются из папки `lab_03/build`.

Аргументы с клавиатуры, данные из файла, вывести в консоль
`cat - ../data/in.txt | ./median_filter`

Ввод из файла и вывод результата в файл
`./median_filter < ../data/input_10x10.txt > ../data/out.txt`

Оба потока в терминал
`./median_filter`

Результат в файл, диагностика в терминал
`./median_filter > out.txt`

Результат в терминал, диагностика в файл
`./median_filter 2> err.txt`

Каждый поток в свой файл
`./median_filter > out.txt 2> err.txt`

Оба потока в один файл
`./median_filter > all.txt 2>&1`

Скрыть результат, оставить диагностику
`./median_filter > /dev/null`

Оставить результат, скрыть диагностику
`./median_filter 2> /dev/null`

Скрыть оба потока
`./median_filter > /dev/null 2>&1`

Результат в файл, диагностику скрыть
`./median_filter > out.txt 2> /dev/null`

Результат скрыть, диагностику в файл
`./median_filter > /dev/null 2> err.txt`

## strace

Базовый запуск с вводом с клавиатуры:
```bash
strace -f ./median_filter
```

Все системные вызовы в лог:
```bash
strace -f -o ../report/strace.log ./median_filter < ../data/input_10x10.txt > ../data/out.txt
```

Развёрнутые аргументы и пути дескрипторов в лог:
```bash
strace -f -v -s 65535 -yy -o ../report/strace.log ./median_filter < ../data/input_10x10.txt > ../data/out.txt
```

- `-f` — следить за потоками.
- `-v` — раскрывать структуры и массивы.
- `-s 65535` — показывать строки длиной до 65535 байт.
- `-yy` — показывать сведения и пути дескрипторов.
- `-o` — сохранять лог трассировки в файл.

## Исследование

Пять запусков на конфигурацию, медиана и запись в measurements_simple.csv (Release-программа уже собрана):
```bash
python3 ../report/benchmark.py
```

Один замер real/user/sys с параметрами из входного файла:
```bash
/usr/bin/time -p ./benchmark/median_filter < ../data/input_1000x1000.txt > benchmark/manual_out.txt
```

Графики ускорения и эффективности из measurements_simple.csv:
```bash
../.venv/bin/python ../report/plot_results.py
```
