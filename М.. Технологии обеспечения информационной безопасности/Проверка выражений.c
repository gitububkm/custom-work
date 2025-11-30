/*
 * validator.c - Синтаксический анализатор арифметических выражений.
 *
 * Специализация: Безопасное программирование для критически важных систем.
 * Стандарт: Строго ANSI C (C89/C90).
 *
 * Автор: Старший разработчик / Эксперт по ИБ.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* --- Константы и определения --- */

#define MAX_EXPR_LEN 1024
#define BUFFER_SIZE (MAX_EXPR_LEN + 2) /* +1 для \n и +1 для \0 */

#define TRUE 1
#define FALSE 0

/*
 * Состояния конечного автомата для синтаксического анализа.
 * ENUM - стандартный и безопасный способ представления состояний.
 */
typedef enum {
    STATE_EXPECT_OPERAND, /* Ожидается операнд (число, переменная, унарный знак, открывающая скобка) */
    STATE_EXPECT_OPERATOR /* Ожидается бинарный оператор или закрывающая скобка */
} State;


/* --- Прототипы функций --- */

/*
 * Главная функция валидации выражения.
 * Принимает строку и возвращает TRUE (1) если она корректна, иначе FALSE (0).
 * Использование 'const char *' гарантирует, что функция не изменяет входные данные.
 */
int isValidExpression(const char *expr);


/* --- Основная логика --- */

int main(void)
{
    /*
     * ANSI C (C89/C90) требует объявления всех переменных в начале блока.
     */
    char buffer[BUFFER_SIZE];
    char *newline_pos;

    /*
     * Безопасное чтение ввода.
     * fgets читает не более BUFFER_SIZE - 1 символов, предотвращая переполнение буфера.
     * Всегда проверяем результат вызова системных функций.
     */
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        /* Ошибка чтения или достигнут конец файла при пустом вводе */
        printf("incorrect\n");
        return 0;
    }

    /*
     * fgets сохраняет символ новой строки '\n', если он помещается в буфер.
     * Его необходимо удалить для корректной обработки выражения.
     */
    newline_pos = strchr(buffer, '\n');
    if (newline_pos != NULL) {
        *newline_pos = '\0';
    }

    /*
     * Вызов основной функции-валидатора и печать результата.
     */
    if (isValidExpression(buffer)) {
        printf("correct\n");
    } else {
        printf("incorrect\n");
    }

    return 0;
}

/* --- Реализация функций --- */

int isValidExpression(const char *expr)
{
    /* Объявление всех переменных в начале функции, как того требует ANSI C. */
    int i;
    int len;
    char current_char;
    int parenthesis_balance = 0; /* Счетчик для проверки баланса скобок */
    State current_state = STATE_EXPECT_OPERAND;

    len = strlen(expr);

    /* Пустая строка или строка, состоящая только из пробелов, не является корректным выражением. */
    /* Этот случай будет корректно обработан проверкой конечного состояния в конце функции. */

    for (i = 0; i < len; ++i) {
        current_char = expr[i];

        if (isspace(current_char)) {
            /* Пробельные символы игнорируются */
            continue;
        }

        if (current_state == STATE_EXPECT_OPERAND) {
            if (isdigit(current_char)) {
                /* Нашли начало числа. Пропускаем все последующие цифры. */
                while (i + 1 < len && isdigit(expr[i + 1])) {
                    i++;
                }
                current_state = STATE_EXPECT_OPERATOR;
            } else if (islower(current_char)) {
                /* Нашли переменную */
                current_state = STATE_EXPECT_OPERATOR;
            } else if (current_char == '(') {
                parenthesis_balance++;
                /* После открывающей скобки мы снова ожидаем операнд */
                current_state = STATE_EXPECT_OPERAND;
            } else if (current_char == '+' || current_char == '-') {
                /* Это унарный плюс или минус. После него все еще ожидается операнд. */
                current_state = STATE_EXPECT_OPERAND;
            } else {
                /* В состоянии ожидания операнда встречен недопустимый символ */
                return FALSE;
            }
        } else { /* current_state == STATE_EXPECT_OPERATOR */
            if (strchr("+-*/%", current_char) != NULL) {
                /* Нашли бинарный оператор. Далее ожидаем операнд. */
                current_state = STATE_EXPECT_OPERAND;
            } else if (current_char == ')') {
                parenthesis_balance--;
                /* После закрывающей скобки мы все еще ожидаем оператор, e.g. "(a+b)*c" */
                current_state = STATE_EXPECT_OPERATOR;
            } else {
                /*
                 * Ошибка: два операнда подряд без оператора.
                 * Примеры: "7a", "(a+b)(c-d)"
                 */
                return FALSE;
            }
        }

        /*
         * Анализ безопасности: Немедленная проверка дисбаланса скобок.
         * Если счетчик стал отрицательным, значит закрывающая скобка встретилась раньше открывающей.
         * Это предотвращает дальнейшую обработку заведомо некорректной строки.
         */
        if (parenthesis_balance < 0) {
            return FALSE;
        }
    }

    /*
     * Финальные проверки после прохода по всей строке:
     * 1. Баланс скобок должен быть равен нулю.
     * 2. Выражение не может заканчиваться на бинарный оператор (это означает, что
     *    конечное состояние должно быть STATE_EXPECT_OPERATOR). Если оно STATE_EXPECT_OPERAND,
     *    значит выражение оборвалось после оператора, например "a+".
     */
    if (parenthesis_balance == 0 && current_state == STATE_EXPECT_OPERATOR) {
        return TRUE;
    }

    return FALSE;
}