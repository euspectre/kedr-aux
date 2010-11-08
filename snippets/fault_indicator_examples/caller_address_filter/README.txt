Пример индикатора (indicator.data), который помимо стандартной работы индикатора __kmalloc, делает возможным использование сценария на основе
адреса, по которому был осуществлен вызов replacement функции.

Этот адрес доступен из выражения под именем "caller".

Индикатор может быть использован для точки, которую создает fault simulation payload (payload_fsim.data).

Проверено на шаблонах KEDR из ревизии 7e1573c2db19.

Use case:
1. Из KEDR копируем себе пример custom_indicator_fsim.
2. Заменяем файл indicator.data из примера на indicator.data (из этого snippet'а).
3. Заменяем имя модуля в makefile и Kbuild на indicator_fsim_kmalloc.
4. Собираем (make). Warning: "assignment makes integer from pointer without cast" - это нормально (в будушем должны быть доработаны шаблоны).

5. Из KEDR копируем себе пример custom_payload_fsim.
6. Заменяем файл payload.data из примера на payload_fsim.data (из этого snippet'а).
7. Заменяем имя модуля в makefile и Kbuild на payload_fsim_kmalloc.
8. Собираем (make).

9. Из KEDR копируем себе пример sample_target, собираем (make).

Дальнейшие шаги могут быть в некоторой степени автоматизированы.

10. Загружаем KEDR с использованием собранного fault simulation payload(+ разрешаем tracepoint'ы для payload).
11. Загружаем собранный индикатор.
12. Устанавливаем индикатор для точки: echo "kmalloc" > /sys/kernel/debug/kedr_fault_simulation/__kmalloc/current_indicator

13. Определяем адрес инструкции, которая следует непосредственно за вызовом kmalloc в cfake_open.
Выйдет что-то похожее на (.text+0x60d).

14. Загружаем target (./kedr_sample_target load).
15. Складываем адрес .text секции загруженного target модуля (/sys/module/kedr_sample_target/sections/.text) и относительный адрес
инструкции с предыдущего шага, переводим результат в десятичную систему счисления.

16. Устанавливаем выражение для индикатора caller=<decimal-abs-address>:
echo "caller=<decimal-abs-address>" > /sys/kernel/debug/kedr_fault_simulation/__kmalloc/expression

17. После этого попытки записи в устройство 
echo 1 > /dev/cfake
будут возвращать ошибку.