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

int main(int argc, char* argv[]) {
    char file_name[MAX_LENGTH];
    char mmapped_file_name[MAX_LENGTH];
    strcpy(file_name, argv[1]);
    strcpy(mmapped_file_name, argv[2]);

    char semaphore_name[MAX_LENGTH];
    strcpy(semaphore_name, argv[3]);
    char semaphoreForParent_name[MAX_LENGTH];
    strcpy(semaphoreForParent_name, argv[4]);

    int file_descriptor = open(file_name, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);

    int mmapped_file_descriptor = shm_open(mmapped_file_name, O_RDWR | O_CREAT, 0777);
    ftruncate(mmapped_file_descriptor, MAX_LENGTH);
    char* mmapped_file_pointer = mmap(NULL, MAX_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, mmapped_file_descriptor, 0);

    sem_t* semaphore = sem_open(semaphore_name, 0);
    sem_t* semaphoreForParent = sem_open(semaphoreForParent_name, 0);

    char vowels[] = {'a', 'e', 'i', 'o', 'u', 'y'};
    char string[MAX_LENGTH];
    while (1) {
        sem_wait(semaphore);

        for (int i = 0; i < strlen(mmapped_file_pointer); ++i) {
            string[i] = mmapped_file_pointer[i];
        }
        string[ strlen(mmapped_file_pointer) ] = 0;
    
        sem_post(semaphoreForParent);

        if (strlen(string) == 0) {
            break;
        }

        for (int index = 0; index < strlen(string); ++index) {
            if (memchr(vowels, string[index], 6) == NULL) {
                write(file_descriptor, &string[index], 1);
            }
        }
    }

    munmap(mmapped_file_pointer, 0);
    sem_close(semaphore);
    sem_close(semaphoreForParent);
    close(file_descriptor);

    return 0;
}