Здесь лежат тесты.

converter-ism-tests.sh - скрипт конвертации тестов из формата, какой дан в https://github.com/contactless/rfm69-linux (011000010101010...) в формат, который использует rfsniffer (+1020 -500 +1600 .....)

ismTestConverter.cpp - код конвертера, который использует скрипт converter-ism-tests.sh.

showTest.cpp - самодостаточная программа для представления тестов формата rfsniffer в human-readable виде.

wb-homa-rfsniffer-test, который получается после компиляции осуществляет, собственно работу по тестированию. Опции:
	-a эквивалентно -l -p -r -s -m
	-l тест логгирования
	-p тест парсера (тест распознавания протоколов)
	-r тест связи с радиомодулем
	-s тест основной части
	-m тест связи с mqtt
	-f директория в которой лежат образцы пришедших сигналов, которые нужно просмотреть и вывести результат распознавания для каждого образка
	


