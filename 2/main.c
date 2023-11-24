#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "pthread.h"
#include "sys/time.h"

#define MAX_LENGTH 1000000
// const int MAX_LENGTH = 1000000;

// Почему из main работают константы, а здесь нет?
char text[MAX_LENGTH];
int text_length;
char pattern[MAX_LENGTH];
int pattern_length;
int result[MAX_LENGTH];
int index_result = 0;
pthread_mutex_t mutex;

int min(int a, int b) {
    return a < b ? a : b;
}

void* search_pattern(void* args) {
    // Как правильно рассчитывать start и end: при создании потоков или здесь, учитывая, что длина строки глобальна
    int start = ((int*)args)[0],
    end = ((int*)args)[1],
    id = ((int*)args)[2];

    for (int index_text = start; index_text < end; ++index_text) {
        if (index_text + pattern_length > text_length) {
            break;
        }

        bool is_ok = true;
        for (int index_pattern = 0; index_pattern < pattern_length; ++index_pattern) {
            if (text[index_text + index_pattern] != pattern[index_pattern]) {
                is_ok = false;
                break;
            }
        }

        if (is_ok) {
            pthread_mutex_lock(&mutex);
            result[index_result++] = index_text;
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit(0);
}

int main(int argc, char* argv[]) {
    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);

    int number_threads = 1;
    if (argc > 1)
        number_threads = atoi(argv[1]);

    fread(pattern, sizeof(char), MAX_LENGTH, stdin);
    pattern_length = strlen(pattern);

    FILE* file_descriptor = fopen("test/string.txt", "r");
    fread(text, sizeof(char), MAX_LENGTH, file_descriptor);
    text_length = strlen(text);

    if (number_threads > text_length)
        number_threads = text_length;

    int size_ = strlen(text) / number_threads;
    pthread_t threads_id[number_threads];
    int args[number_threads][3];
    pthread_mutex_init(&mutex, NULL);
    for (int index = 0; index < number_threads; ++index) {
        args[index][0] = size_ * index;
        if (index == number_threads - 1)
            args[index][1] = strlen(text);
        else
            args[index][1] = size_ * index + size_;
        args[index][2] = index;
        pthread_create(&threads_id[index], NULL, search_pattern, args + index);
    }

    for (int index = 0; index < number_threads; ++index) {
        pthread_join(threads_id[index], NULL);
    }

    printf("Индексы совпадений:\n");
    for (int index = 0; index < index_result; ++index)
        printf("%d ", result[index]);
    printf("\n");

    gettimeofday(&time_end, NULL);
    long seconds = (time_end.tv_sec - time_start.tv_sec);
    long micros = ((seconds * 1000000) + time_end.tv_usec) - (time_start.tv_usec);
    printf("Время выполнения: %ld\n", micros);

    return 0;
}