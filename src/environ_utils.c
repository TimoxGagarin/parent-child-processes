#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "headers/environ_utils.h"

/**
 * @brief Сравнивает две строки с учетом текущей локали (используется для сортировки).
 *
 * @param a Указатель на первую строку для сравнения.
 * @param b Указатель на вторую строку для сравнения.
 * @return Результат сравнения строк с учетом текущей локали (LC_COLLATE).
 */
int compareStrings(const void *a, const void *b) {
    return strcoll(*(const char **)a, *(const char **)b);
}

/**
 * @brief Получает значение переменной среды по ее имени из массива envp.
 *
 * @param envp Массив строк envp (переменные окружения).
 * @param variable Имя переменной среды, значение которой необходимо получить.
 * @return Указатель на значение переменной среды, если найдено, иначе NULL.
 */
const char* get_env_value(char **envp, char *variable) {
    // Длина имени переменной
    size_t variable_length = strlen(variable);
    
    // Поиск переменной среды в массиве
    for (int i = 0; envp[i] != NULL; i++)
        // Проверяем, начинается ли текущая строка с искомого имени переменной
        // и следующий символ после имени - знак "="
        if (strncmp(envp[i], variable, variable_length) == 0 && envp[i][variable_length] == '=')
            // Найдено значение переменной среды. Возвращаем указатель на начало значения
            return &envp[i][variable_length + 1];

    // Переменная среды не найдена
    return NULL;
}

/**
 * @brief Выводит и сортирует переменные окружения.
 *
 * @details Использует глобальную переменную environ.
 */
void printEnviron() {
    // Получаем переменные среды
    extern char **environ;

    // Подсчитываем количество переменных среды
    int env_count = 0;
    for (; environ[env_count] != NULL; env_count++);

    // Выделяем память под массив строк переменных среды
    char **env_array = (char**)malloc(env_count * sizeof(char*));
    if (env_array == NULL) {
        perror("Ошибка выделения памяти");
        exit(EXIT_FAILURE);
    }

    // Копируем переменные среды в массив
    for (int i = 0; i < env_count; i++) {
        env_array[i] = (char*)malloc(strlen(environ[i]) + 1);
        strcpy(env_array[i], environ[i]);

        if (env_array[i] == NULL) {
            perror("Ошибка выделения памяти");
            exit(EXIT_FAILURE);
        }
    }

    // Сортируем массив строк переменных среды
    qsort(env_array, env_count, sizeof(char*), compareStrings);

    // Выводим и освобождаем выделенную память
    for (int i = 0; i < env_count; i++) {
        printf("%s\n", env_array[i]);
        free(env_array[i]);
    }
    free(env_array);
}

/**
 * @brief Выводит значения переменных окружения, считанных из файла.
 *
 * @param filename Имя файла с переменными окружения.
 * @param envp Массив переменных окружения.
 */
void print_custom_environ(char* filename, char* envp[]) {
    // Открыть файл с переменными
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fclose(file);

    char variable_name[256];

    // Заполнить массив переменными окружения
    file = fopen(filename, "r");
    int i = 0;
    while (fscanf(file, "%s", variable_name) != EOF) {
        char *variable_value;
        if(envp != NULL)
            variable_value = get_env_value(envp, variable_name);
        else
            variable_value = getenv(variable_name);

        if(!variable_value) {
            fprintf(stderr, "Variable %s is not set in the environment\n", variable_name);
            continue;
        }
        printf("%s=%s\n", variable_name, variable_value);
    }

    // Закрыть файл
    fclose(file);
}

/**
 * @brief Создает массив переменных окружения на основе содержимого файла.
 *
 * @param filename Имя файла с переменными окружения.
 * @param env Указатель на массив переменных окружения.
 */
void createCustomEnviroment(char* filename, char*** env) {
    // Открыть файл с переменными
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fclose(file);

    char variable_name[256];

    // Заполнить массив переменными окружения
    file = fopen(filename, "r");
    int i = 0;
    for (; fscanf(file, "%s", variable_name) != EOF; i++) {
        char *variable_value = getenv(variable_name);

        // Пропуск переменных, которые не установлены и не являются HOSTNAME и LC_COLLATE
        if (!variable_value && strcmp(variable_name, "HOSTNAME") && strcmp(variable_name, "LC_COLLATE")) {
            i--;
            continue;
        }
        // Если переменная уже установлена, выделить память под новый элемент массива
        else if (variable_value) {
            *env = (char**)realloc(*env, (i + 1) * sizeof(char*));
            if (*env == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }
        // Обработка переменной HOSTNAME
        else if (!strcmp(variable_name, "HOSTNAME")) {
            variable_value = (char*)realloc(variable_value, 256);

            // Попытка получить имя хоста
            if (gethostname(variable_value, sizeof(char*)) != 0) {
                if (errno == ENAMETOOLONG) {
                    variable_value = (char*)realloc(variable_value, strlen(variable_value) + 1);
                } else {
                    perror("gethostname");
                    continue;
                }
            }

            // Создание записи для переменной HOSTNAME и добавление в массив
            char *variable_entry = (char *)malloc(strlen(variable_name) + strlen(variable_value) + 2);
            if (variable_entry == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            sprintf(variable_entry, "%s=%s", variable_name, variable_value);
            (*env)[i] = variable_entry;
            continue;
        }
        // Обработка переменной LC_COLLATE
        else if (!strcmp(variable_name, "LC_COLLATE")) {
            // Создание записи для переменной LC_COLLATE и добавление в массив
            (*env)[i] = (char*)malloc(12);
            strcpy((*env)[i], "LC_COLLATE=C");
            continue;
        }

        // Создание записи для обычной переменной и добавление в массив
        char *variable_entry = (char *)malloc(strlen(variable_name) + strlen(variable_value) + 2);
        sprintf(variable_entry, "%s=%s", variable_name, variable_value);
        (*env)[i] = variable_entry;
    }

    (*env)[i] = NULL; // Завершить массив NULL

    // Закрыть файл
    fclose(file);
}