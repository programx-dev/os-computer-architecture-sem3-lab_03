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