#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "include/funcs.h"

void* open_lib(const char* name) {
    void* library = dlopen(name, RTLD_LAZY);
    if (library == NULL) {
        printf("%s", dlerror());
        exit(1);
    }

    return library;
}

void change_realization(void** libraries, int (**GCF)(int, int), int* (**Sort)(int*, int), int* realization) {
    *realization = *realization ^ 1;

    *GCF = dlsym(libraries[*realization], "GCF");
    *Sort = dlsym(libraries[*realization], "Sort");

    // printf("Смена реализации: %p, %p\n", *GCF, *Sort);
}

int main() {
    void* library1 = open_lib("lib/librealization1.so");
    void* library2 = open_lib("lib/librealization2.so");
    void* libraries[2] = {library1, library2};

    int (*GCF)(int, int);
    int* (*Sort)(int*, int);

    int realization = 1;
    change_realization(libraries, &GCF, &Sort, &realization);

    while (1) {
        int mode;
        scanf("%d", &mode);

        if (mode == 0) {
            change_realization(libraries, &GCF, &Sort, &realization);
            printf("Вы переключились на %dую реализацию\n", realization + 1);
        } else if (mode == 1) {
            int A, B;
            scanf("%d %d", &A, &B);

            printf("Вы вызвали %dую функцию с аргументами A = %d, B = %d\n", mode, A, B);
            // printf("Адреса функций: %p, %p\n", GCF, Sort);
            
            printf("Результат: %d\n", (*GCF)(A, B));
        } else if (mode == 2) {
            int size;
            scanf("%d", &size);

            int array[size];
            for (int i = 0; i < size; ++i)
                scanf("%d", array + i);

            printf("Вы вызвали %dую функцию с аргументами size = %d, ", mode, size);
            print_array(array, size);

            printf("Результат: ");
            print_array((*Sort)(array, size), size);
        } else {
            printf("Неизвестный режим %d\n", mode);
        }
    }
}