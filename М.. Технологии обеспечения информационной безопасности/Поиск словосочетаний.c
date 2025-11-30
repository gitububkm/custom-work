/*
 * search.c - Поиск словосочетаний в тексте (ANSI C / C89).
 *
 * Задача: Найти вхождения фразы в тексте.
 * Особенности:
 * 1. Гибкие разделители (пробел, tab, \n, \r).
 * 2. Частичные совпадения слов разрешены (согласно примеру 4).
 * 3. Поддержка Win-1251 (русский текст).
 *
 * Автор: Старший разработчик / Эксперт по ИБ.
 */

#include <stdio.h>
#include <string.h>

/* --- Константы и Макросы --- */

/* Максимальный размер фразы и текста согласно заданию */
#define MAX_PHRASE_LEN 105
#define MAX_TEXT_LEN   2005

/* Логические константы для ANSI C */
#define TRUE  1
#define FALSE 0

/* Имена файлов */
#define INPUT_FILE  "input.txt"
#define OUTPUT_FILE "output.txt"

/* --- Прототипы функций --- */

/* Проверяет, является ли символ разделителем */
int isSeparator(int c);

/* Сравнивает фразу с текстом в данной позиции */
int matchPhrase(const char* text_ptr, const char* phrase_ptr);

/* --- Основная программа --- */

int main(void)
{
    FILE* fin;
    FILE* fout;
    
    /* Буферы данных */
    char phrase[MAX_PHRASE_LEN];
    char text[MAX_TEXT_LEN];
    
    /* Массив флагов: 1, если в позиции i начинается совпадение */
    char match_flags[MAX_TEXT_LEN];
    
    /* Переменные циклов и счетчики */
    int i;
    int text_len = 0;
    int ch;
    char* newline_pos;

    /* 1. Инициализация памяти */
    for (i = 0; i < MAX_TEXT_LEN; i++) {
        text[i] = '\0';
        match_flags[i] = 0;
    }
    for (i = 0; i < MAX_PHRASE_LEN; i++) {
        phrase[i] = '\0';
    }

    /* 2. Чтение входных данных */
    fin = fopen(INPUT_FILE, "r");
    if (fin == NULL) {
        /* Ошибка открытия входного файла */
        return 1;
    }

    /* Чтение искомой фразы (первая строка) */
    if (fgets(phrase, sizeof(phrase), fin) == NULL) {
        fclose(fin);
        /* Пустой файл - создаем пустой выходной файл */
        fout = fopen(OUTPUT_FILE, "w");
        if (fout != NULL) fclose(fout);
        return 0;
    }

    /* Удаление символов перевода строки из фразы */
    newline_pos = strchr(phrase, '\n');
    if (newline_pos != NULL) *newline_pos = '\0';
    newline_pos = strchr(phrase, '\r');
    if (newline_pos != NULL) *newline_pos = '\0';

    /* Чтение остального текста посимвольно */
    while ((ch = fgetc(fin)) != EOF && text_len < MAX_TEXT_LEN - 1) {
        text[text_len++] = (char)ch;
    }
    text[text_len] = '\0';
    fclose(fin);

    /* 3. Поиск совпадений */
    /* Если фраза пустая, совпадений нет */
    if (phrase[0] != '\0') {
        for (i = 0; i < text_len; i++) {
            if (matchPhrase(&text[i], phrase)) {
                match_flags[i] = 1;
            }
        }
    }

    /* 4. Запись результата */
    fout = fopen(OUTPUT_FILE, "w");
    if (fout == NULL) {
        return 1;
    }

    for (i = 0; i < text_len; i++) {
        /* Если в этой позиции начинается совпадение, ставим '@' */
        if (match_flags[i]) {
            fputc('@', fout);
        }
        fputc(text[i], fout);
    }

    fclose(fout);
    return 0;
}

/* --- Реализация функций --- */

int isSeparator(int c)
{
    /* 
     * Приводим к unsigned char для безопасности 
     * (защита от отрицательных значений char в Win-1251)
     */
    unsigned char uc = (unsigned char)c;
    return (uc == ' ' || uc == '\t' || uc == '\n' || uc == '\r');
}

/*
 * matchPhrase:
 * Проверяет, совпадает ли phrase с началом text_ptr.
 * Учитывает гибкие разделители.
 */
int matchPhrase(const char* text_ptr, const char* phrase_ptr)
{
    const char* t = text_ptr;
    const char* p = phrase_ptr;

    while (*p != '\0') {
        /* Если достигли конца текста, но фраза не закончилась - не совпадение */
        if (*t == '\0') {
            return FALSE;
        }

        if (isSeparator(*p)) {
            /* 
             * Если во фразе разделитель, в тексте ТОЖЕ должен быть разделитель.
             * Иначе это не совпадение (например "AB" не совпадает с "A B").
             */
            if (!isSeparator(*t)) {
                return FALSE;
            }

            /* 
             * "Схлопывание" разделителей:
             * Пропускаем все подряд идущие разделители и во фразе, и в тексте.
             * Группа пробелов равна любой другой группе пробелов/табов.
             */
            while (isSeparator(*p)) p++;
            while (isSeparator(*t)) t++;
        } else {
            /* Обычный символ: строгое сравнение */
            if ((unsigned char)*p != (unsigned char)*t) {
                return FALSE;
            }
            p++;
            t++;
        }
    }

    /* 
     * Если мы вышли из цикла, значит вся фраза совпала.
     * Мы НЕ проверяем, что идет после фразы (согласно примеру 4).
     */
    return TRUE;
}