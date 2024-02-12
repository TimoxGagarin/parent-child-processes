#pragma once

void choose_options(char* vars_filepath, char* envp[]);
void createChildProcess(const char* child_path, const char* vars_filepath, char ch);