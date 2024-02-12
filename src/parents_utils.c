#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "headers/parent_utils.h"
#include "headers/environ_utils.h"

/**
 * @brief Обрабатывает опции, вводимые пользователем, и выполняет соответствующие действия.
 *
 * @param vars_filepath Путь к файлу с переменными окружения.
 * @param envp Массив переменных окружения.
 */
void choose_options(char* vars_filepath, char* envp[]) {
    extern char** environ;

    printf("Вход в цикл обработки нажатий клавиши...\n");
    char ch;
    while ((ch = getchar()) != EOF) {
        switch (ch)
        {
        case '+':
            // Запуск дочернего процесса с указанным расположением, полученным из переменной окружения "CHILD_PATH"
            createChildProcess(getenv("CHILD_PATH"), vars_filepath, ch);
            break;
        case '*':
            // Запуск дочернего процесса с указанным расположением, полученным из переменной окружения "CHILD_PATH"
            createChildProcess(get_env_value(envp, "CHILD_PATH"), vars_filepath, ch);
            break;
        case '&':
            // Запуск дочернего процесса с указанным расположением, полученным из глобальной переменной окружения "environ"
            createChildProcess(get_env_value(environ, "CHILD_PATH"), vars_filepath, ch);
            break;
        case 'q':
            // Завершение программы при вводе 'q'
            exit(EXIT_SUCCESS);
        case '\n':
            // Пропуск символа новой строки, продолжение цикла
            continue;
        default:
            // Вывод символа, если введена неизвестная команда
            printf("Неизвестная команда: %c\n", ch);
            break;
        }
    }
}

/**
 * @brief Создает дочерний процесс и выполняет указанную программу с определенными аргументами и окружением.
 *
 * @param child_path Путь к программе, которую необходимо выполнить в дочернем процессе.
 * @param vars_filepath Путь к файлу с переменными окружения.
 * @param ch Опция выбора.
 */
void createChildProcess(const char* child_path, const char* vars_filepath, char ch) {
    static int child_number = 0;
    // Установка имени программы (argv[0]) как "child_XX"
    char child_name[50];
    snprintf(child_name, sizeof(child_name), "child_%02d", child_number);

    // Инкрементация номера для следующего запуска
    child_number++;

    if (child_number > 99) {
        printf("Count of child greater than 100. Counter is set to zero\n");
        child_number = 0;
    }

    // Создание массива аргументов (argv)
    char *argv[] = {child_name, vars_filepath, &ch, NULL};

    // Запуск дочернего процесса
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Код дочернего процесса
        char full_child_path[256];
        snprintf(full_child_path, strlen(child_path) + 1, "%s/child", child_path);

        char **custom_environment = (char **)malloc(sizeof(char *));
        if (custom_environment == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        createCustomEnviroment(vars_filepath, &custom_environment);

        // Замена текущего процесса кодом нового процесса
        execve(full_child_path, argv, custom_environment);

        // В случае ошибки при выполнении execve
        perror("execve");
        exit(EXIT_FAILURE);
    } else {
        // Код родительского процесса
        // Ждем завершения дочернего процесса
        waitpid(pid, NULL, 0);
    }
}