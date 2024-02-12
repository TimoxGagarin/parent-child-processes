#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <sys/wait.h>
#include "headers/environ_utils.h"

int main(int argc, char* argv[], char* envp[]) {
    extern char** environ;

    // Проверка на правильное количество аргументов
    if(argc != 3){
        printf("Неверное количество аргументов. Правильное использование программы:\n");
        printf("CHILD_PATH=<путь/к/папке> ./parent <путь/к/файлу> [+, *, &]\n");
        exit(EXIT_FAILURE);
    }

    // Вывод информации о процессе
    printf("-----------------------------------------\n");
    printf("Name: %s\n", argv[0]);
    printf("pid: %i\n", getpid());
    printf("ppid: %i\n", getppid());

    // Определение команды
    char ch = argv[2][0];

    // Выполнение действий в зависимости от команды
    switch (ch)
    {
    case '+':
        // Вывод переменных окружения из файла, игнорируя текущие переменные окружения
        print_custom_environ(argv[1], NULL);
        break;
    case '*':
        // Вывод переменных окружения из файла, включая текущие переменные окружения
        print_custom_environ(argv[1], envp);
        break;
    case '&':
        // Вывод текущих переменных окружения
        print_custom_environ(argv[1], environ);
        break;
    default:
        printf("Неизвестная команда: %c\n", ch);
        break;
    }

    // Завершение программы
    printf("-----------------------------------------\n");
    exit(EXIT_SUCCESS);
}
