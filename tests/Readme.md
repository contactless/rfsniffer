Здесь находятся тесты на rfsniffer

Запускать wb-homa-rfsniffer-test. Опции:
-l - тест логгирования
-p - тест парсера
-r - тест взаимодействия с радиомодулем
-s - тест центральной части программы
-m - тест взаимодействия с mqtt
-f path - указать директорию с тестами
-a - всё вышеперечисленное кроме -f

Немного отдельно идут ismTestConverter.cpp и convert-ism-tests.sh.
Редактирование и запуск convert-ism-tests.sh позволяют сконвертировать тесты заданные в формате как в https://github.com/contactless/rfm69-linux/blob/master/tests.py в формат принятый в rfsniffer.
