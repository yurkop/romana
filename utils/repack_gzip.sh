#!/bin/bash

# Скрипт для пережатия gzip файла с уровнем 0 на уровень 9 с прогресс-баром
# Использование: ./repack_gzip.sh input.gz output.gz

set -e  # Прекратить выполнение при любой ошибке

# Проверка количества аргументов
if [ $# -ne 3 ]; then
    echo "Ошибка: Неверное количество аргументов"
    echo "Использование: $0 <входной_файл> <выходной_файл> <сжатие>[0..9]"
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_FILE="$2"
LEVEL="$3"

# Проверка существования входного файла
if [ ! -f "$INPUT_FILE" ]; then
    echo "Ошибка: Входной файл '$INPUT_FILE' не существует"
    exit 1
fi

# Проверка что входной файл читается
if [ ! -r "$INPUT_FILE" ]; then
    echo "Ошибка: Входной файл '$INPUT_FILE' недоступен для чтения"
    exit 1
fi

# Проверка что выходной файл не существует
if [ -e "$OUTPUT_FILE" ]; then
    echo "Ошибка: Выходной файл '$OUTPUT_FILE' уже существует"
    exit 1
fi

# Проверка наличия утилит
if ! command -v pigz &> /dev/null; then
    echo "Ошибка: pigz не установлен или не найден в PATH"
    exit 1
fi

if ! command -v pv &> /dev/null; then
    echo "Ошибка: pv не установлен или не найден в PATH"
    exit 1
fi

pigz -d -c "$INPUT_FILE" | pv | pigz -"$LEVEL" -c > "$OUTPUT_FILE"

# Проверяем успешность выполнения
if [ $? -eq 0 ] && [ -f "$OUTPUT_FILE" ]; then
    echo ""
    echo "Пережатие успешно завершено!"
    
    # Показываем информацию о размерах
    INPUT_SIZE=$(stat -c%s "$INPUT_FILE")
    OUTPUT_SIZE=$(stat -c%s "$OUTPUT_FILE")
    RATIO=$(echo "scale=2; $OUTPUT_SIZE * 100 / $INPUT_SIZE" | bc)
    
    echo "Размер исходного файла: $INPUT_SIZE байт"
    echo "Размер нового файла:    $OUTPUT_SIZE байт"
    echo "Коэффициент сжатия:     $RATIO%"
else
    echo ""
    echo "Ошибка: Процесс пережатия завершился с ошибкой"
    # Удаляем частично созданный выходной файл
    [ -f "$OUTPUT_FILE" ] && rm -f "$OUTPUT_FILE"
    exit 1
fi
