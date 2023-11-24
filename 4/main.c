// Вариант 26
// 4 - Подсчёт наибольшего общего делителя для двух натуральных чисел
// 9 - Отсортировать целочисленный массив

#include <stdio.h>
#include <stdlib.h>

#include "lib/realization1.c"
// #include "lib/realization2.c"

int main() {
    printf("%d\n", GCF(84324, 3238));
    printf("%d\n", GCF(832256, 5712));

    int array[] = {1, 7, 5, 3, 2, 0, 6, 10};
    Sort(array, 9);
    for (int i = 0; i < 9; ++i)
        printf("%d ", array[i]);
    printf("\n");
}