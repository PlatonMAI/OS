#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/wait.h"
#include "string.h"

#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "sys/mman.h"
#include "semaphore.h"

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
    const char* mmapped_file_names[2];
    mmapped_file_names[1] = "mmaped_file_1";
    mmapped_file_names[0] = "mmaped_file_2";
    const char* semaphores_names[2];
    semaphores_names[0] = "/semaphoreOne";
    semaphores_names[1] = "/semaphoreTwo";
    const char* semaphoresForParent_names[2];
    semaphoresForParent_names[0] = "/semaphoresForParentOne";
    semaphoresForParent_names[1] = "/semaphoresForParentTwo";

    for (int i = 0; i < number_processes; ++i) {
        shm_unlink(mmapped_file_names[i]);
        sem_unlink(semaphores_names[i]);
        sem_unlink(semaphoresForParent_names[i]);
    }

    // Считываем имена для файлов
    unsigned char file_names[number_processes][MAX_LENGTH];
    for (int i = 0; i < number_processes; ++i) {
        fgets(file_names[i], MAX_LENGTH, stdin);

        int str_len = strlen(file_names[i]);
        if (file_names[i][str_len - 1] == 10) { // перенос строки
            file_names[i][str_len - 1] = 0;
        }
    }

    // Получаем дескрипторы memory-mapped-файлов размером MAX_LENGTH и выделяем их в память
    int mmapped_file_descriptors[number_processes];
    char* mmapped_file_pointers[number_processes];
    for (int i = 0; i < number_processes; ++i) {
        mmapped_file_descriptors[i] = shm_open(mmapped_file_names[i], O_RDWR | O_CREAT, 0777);
        ftruncate(mmapped_file_descriptors[i], MAX_LENGTH);
        mmapped_file_pointers[i] = mmap(NULL, MAX_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, mmapped_file_descriptors[i], 0);
    }

    // Создаем семафоры
    sem_t* semaphores[number_processes][2];
    for (int i = 0; i < number_processes; ++i) {
        semaphores[i][0] = sem_open(semaphores_names[i], O_CREAT, 0777, 0);
        semaphores[i][1] = sem_open(semaphoresForParent_names[i], O_CREAT, 0777, 1);
    }

    for (int index = 0; index < number_processes; ++index) {
        pid_t process_id = create_process();
        if (process_id == 0) {
            // Дочерний процесс
            execl("child", "", file_names[index], mmapped_file_names[index], semaphores_names[index], semaphoresForParent_names[index], NULL);
            perror("exec");
            exit(-1);
        }
    }

    // Слушаем консоль
    char string[MAX_LENGTH];
    int counter = 0;
    while (fgets(string, MAX_LENGTH, stdin)) {
        if (++counter % 2 == 1) {
            sem_wait(semaphores[0][1]);

            for (int i = 0; i < strlen(string); ++i) {
                mmapped_file_pointers[0][i] = string[i];
            }
            mmapped_file_pointers[0][ strlen(string) ] = 0;

            sem_post(semaphores[0][0]);
        } else {
            sem_wait(semaphores[1][1]);

            for (int i = 0; i < strlen(string); ++i) {
                mmapped_file_pointers[1][i] = string[i];
            }
            mmapped_file_pointers[1][ strlen(string) ] = 0;

            sem_post(semaphores[1][0]);
        }
    }
    
    sem_wait(semaphores[0][1]);
    sem_wait(semaphores[1][1]);
    mmapped_file_pointers[0][0] = 0;
    mmapped_file_pointers[1][0] = 0;
    sem_post(semaphores[0][0]);
    sem_post(semaphores[1][0]);

    wait(NULL);

    for (int i = 0; i < number_processes; ++i) {
        munmap(mmapped_file_pointers[i], 0);
        shm_unlink(mmapped_file_names[i]);

        sem_close(semaphores[i][0]);
        sem_close(semaphores[i][1]);
    }

    return 0;
}