#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/wait.h"
#include "string.h"

const int MAX_LENGTH = 255;

int main(int argc, char* argv[]) {
    char filename[MAX_LENGTH];
    strcpy(filename, argv[1]);

    // Перенаправили вывод в файл
    FILE* file = fopen(filename, "w");
    int file_descriptor = fileno(file);
    dup2(file_descriptor, 1);
    printf("```Начало работы```\n");

    char vowels[] = {'a', 'e', 'i', 'o', 'u', 'y'};
    char string[MAX_LENGTH];
    char eof = 1;
    while (fgets(string, MAX_LENGTH, stdin)) {
        if (string[0] == eof) {
            close(file_descriptor);
            fclose(file);
            return 0;
        }

        for (int index = 0; index < strlen(string); ++index) {
            if (memchr(vowels, string[index], 6) == NULL) {
                printf("%c", string[index]);
            }
        }
    }
}