#pragma once

const char* get_env_value(char **envp, char *variable);
void printEnviron();
void createCustomEnviroment(char* filename, char*** env);
void print_custom_environ(char* filename, char* envp[]);