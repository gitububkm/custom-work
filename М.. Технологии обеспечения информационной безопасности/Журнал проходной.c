/*
 * journal.c - Анализатор журнала проходной для определения максимальной загруженности.
 *
 * Специализация: Безопасное программирование для критически важных систем.
 * Стандарт: Строго ANSI C (C89/C90).
 *
 * Автор: Старший разработчик / Эксперт по ИБ.
 * Версия: 2.0 - Исправлена логика определения временного интервала.
 */

#include <stdio.h>
#include <stdlib.h>

/* --- Константы и определения --- */

#define MAX_RECORDS 10000
#define MAX_EVENTS (MAX_RECORDS * 2)

#define EVENT_ENTER 1
#define EVENT_LEAVE (-1)

#define INPUT_FILE "input.txt"
#define OUTPUT_FILE "output.txt"

/*
 * Структура для представления одного события: время и тип (вход/выход).
 * Время хранится в минутах от полуночи для удобства и эффективности сравнения.
 */
typedef struct {
    int time_in_minutes;
    int type;
} Event;


/* --- Прототипы функций --- */

/*
 * Функция сравнения для qsort.
 * Сортирует события по времени. Если время совпадает,
 * события входа (EVENT_ENTER) ставятся перед событиями выхода (EVENT_LEAVE).
 * Это критически важно для корректного подсчета на границах интервалов.
 */
int compareEvents(const void* a, const void* b);

/*
 * Функция для форматированного вывода времени.
 * Принимает минуты, выводит в файл в формате ЧЧ:ММ.
 */
void printTime(FILE* file, int minutes);


/* --- Основная логика --- */

int main(void)
{
    /*
     * ANSI C (C89/C90) требует объявления всех переменных в начале блока.
     */
    FILE* fin;
    FILE* fout;

    Event events[MAX_EVENTS];
    int n, i;
    int h1, m1, h2, m2;

    int current_people = 0;
    int max_people = 0;

    int current_max_period_start_time = 0;
    int max_period_duration = -1;
    int result_start_time = 0;
    int result_end_time = 0;

    /*
     * БЕЗОПАСНОСТЬ: Открытие файла с обязательной проверкой на ошибку.
     */
    fin = fopen(INPUT_FILE, "r");
    if (fin == NULL) {
        return 1;
    }

    /*
     * БЕЗОПАСНОСТЬ: Проверка результата fscanf и корректности значения N.
     */
    if (fscanf(fin, "%d", &n) != 1 || n < 0 || n > MAX_RECORDS) {
        fclose(fin);
        return 1;
    }
    
    /* Обработка случая с пустым журналом */
    if (n == 0) {
        fclose(fin);
        fout = fopen(OUTPUT_FILE, "w");
        if (fout != NULL) {
            fprintf(fout, "0\n00:00 00:00\n");
            fclose(fout);
        }
        return 0;
    }

    for (i = 0; i < n; ++i) {
        if (fscanf(fin, "%d:%d %d:%d", &h1, &m1, &h2, &m2) != 4) {
             fclose(fin);
             return 1;
        }
        
        events[2 * i].time_in_minutes = h1 * 60 + m1;
        events[2 * i].type = EVENT_ENTER;
        events[2 * i + 1].time_in_minutes = h2 * 60 + m2;
        events[2 * i + 1].type = EVENT_LEAVE;
    }
    
    fclose(fin);

    qsort(events, 2 * n, sizeof(Event), compareEvents);

    /*
     * Усовершенствованный алгоритм "сканирующей прямой".
     * Эта логика корректно обрабатывает множественные, несвязанные
     * периоды максимальной загруженности.
     */
    for (i = 0; i < 2 * n; ++i) {
        int prev_people = current_people;
        int current_time = events[i].time_in_minutes;
        
        current_people += events[i].type;

        /*
         * Состояние 1: Достигнут НОВЫЙ, более высокий максимум людей.
         * С этого момента начинается новый потенциально лучший интервал.
         * Все предыдущие расчеты интервалов для старого максимума становятся неважны.
         */
        if (current_people > max_people) {
            max_people = current_people;
            current_max_period_start_time = current_time;
            /* Сбрасываем длительность, т.к. ищем интервал для нового максимума */
            max_period_duration = -1; 
        }
        /*
         * Состояние 2: Количество людей упало С максимального уровня.
         * Это означает, что период пиковой нагрузки только что завершился.
         * Вычисляем его длительность и сравниваем с лучшей найденной.
         */
        else if (prev_people == max_people && current_people < max_people) {
            int current_duration = current_time - current_max_period_start_time;
            
            /*
             * Условие СТРОГО '>', чтобы при равной длине сохранялся самый ранний интервал.
             */
            if (current_duration > max_period_duration) {
                max_period_duration = current_duration;
                result_start_time = current_max_period_start_time;
                result_end_time = current_time;
            }
        }
        /*
         * Состояние 3: Количество людей вернулось К максимальному уровню (после спада).
         * Это означает начало нового периода пиковой нагрузки.
         * Фиксируем время начала этого нового периода.
         */
        else if (prev_people < max_people && current_people == max_people) {
            current_max_period_start_time = current_time;
        }
    }

    fout = fopen(OUTPUT_FILE, "w");
    if (fout == NULL) {
        return 1;
    }

    fprintf(fout, "%d\n", max_people);
    printTime(fout, result_start_time);
    fprintf(fout, " ");
    printTime(fout, result_end_time);
    fprintf(fout, "\n");

    fclose(fout);

    return 0;
}

/* --- Реализация функций --- */

int compareEvents(const void* a, const void* b)
{
    Event* eventA = (Event*)a;
    Event* eventB = (Event*)b;
    
    int time_diff = eventA->time_in_minutes - eventB->time_in_minutes;
    if (time_diff != 0) {
        return time_diff;
    }
    
    return eventB->type - eventA->type;
}

void printTime(FILE* file, int minutes)
{
    fprintf(file, "%02d:%02d", minutes / 60, minutes % 60);
}