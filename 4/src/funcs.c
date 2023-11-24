#include "../include/funcs.h"

void print_array(int* array, int size) {
    printf("array = [");
    for (int i = 0; i < size; ++i) {
        if (i == size - 1)
            printf("%d]\n", array[i]);
        else
            printf("%d, ", array[i]);
    }
}