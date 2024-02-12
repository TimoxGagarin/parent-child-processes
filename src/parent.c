#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <sys/wait.h>
#include "headers/parent_utils.h"
#include "headers/environ_utils.h"


int main(int argc, char *argv[], char *envp[]) {
    // Устанавливаем LC_COLLATE=C
    setlocale(LC_COLLATE, "C");
    
    if(argc != 2){
        printf("Неверное количество аргументов. Правильное использование программы:\n");
        printf("CHILD_PATH=<путь/к/папке> ./parent <путь/к/файлу>\n");
        exit(EXIT_FAILURE);
    }

    // Вывод отсортированных переменных окружения
    printEnviron();

    // Обработка опций и выполнение соответствующих действий
    choose_options(argv[1], envp);

    exit(EXIT_SUCCESS);
}