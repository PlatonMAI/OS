#include <stdio.h>

#include "include/funcs.h"

int main() {
    while (1) {
        int mode;
        scanf("%d", &mode);

        if (mode == 1) {
            int A, B;
            scanf("%d %d", &A, &B);

            printf("Вы вызвали %dую функцию с аргументами A = %d, B = %d\n", mode, A, B);
            
            printf("Результат: %d\n", GCF(A, B));
        } else if (mode == 2) {
            int size;
            scanf("%d", &size);

            int array[size];
            for (int i = 0; i < size; ++i)
                scanf("%d", array + i);

            printf("Вы вызвали %dую функцию с аргументами size = %d, ", mode, size);
            print_array(array, size);

            printf("Результат: ");
            print_array(Sort(array, size), size);
        } else {
            printf("Неизвестный режим %d\n", mode);
        }
    }
}