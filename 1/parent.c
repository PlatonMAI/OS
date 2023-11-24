#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/wait.h"
#include "string.h"

const int MAX_LENGTH = 255;

int create_process() {
    pid_t pid = fork();
    if (-1 == pid) {
        perror("fork");
        exit(-1);
    }
    return pid;
}

int main() {
    const int number_processes = 2;

    // Считываем имена для файлов
    char file_name1[MAX_LENGTH],
    file_name2[MAX_LENGTH];
    fgets(file_name1, MAX_LENGTH, stdin);
    fgets(file_name2, MAX_LENGTH, stdin);
    char* file_names[] = {file_name1, file_name2};

    // Создаем пайпы
    int pipe_file_descriptors[number_processes * 2];
    for (int index = 0; index < number_processes; ++index) {
        int error = pipe(pipe_file_descriptors + index * 2);
        if (error == -1) {
            perror("pipe");
            return -1;
        }
    }

    for (int index = 0; index < number_processes; ++index) {
        pid_t process_id = create_process();
        if (process_id == 0) {
            // Дочерний процесс
            dup2(pipe_file_descriptors[index * 2 + 0], 0);
            close(pipe_file_descriptors[index * 2 + 1]);
            execl("child", "", file_names[index], NULL);
            perror("exec");
            exit(-1);
        }
    }

    // Закрываем дескрипторы, которые не будем использовать
    for (int index = 0; index < number_processes; ++index)
        close(pipe_file_descriptors[index * 2 + 0]);

    // Слушаем консоль
    char string[MAX_LENGTH];
    int counter = 0;
    while (fgets(string, MAX_LENGTH, stdin)) {
        if (string[ strlen(string) - 1 ] != '\n')
            string[ strlen(string) ] = '\n';

        if (++counter % 2 == 1) {
            write(pipe_file_descriptors[0 * 2 + 1], string, sizeof(char) * strlen(string));
        } else {
            write(pipe_file_descriptors[1 * 2 + 1], string, sizeof(char) * strlen(string));
        }
    }

    char eof[2];
    eof[0] = 1;
    eof[1] = '\n';
    write(pipe_file_descriptors[0 * 2 + 1], eof, sizeof(char) * strlen(eof));
    write(pipe_file_descriptors[1 * 2 + 1], eof, sizeof(char) * strlen(eof));

    wait(NULL);
}