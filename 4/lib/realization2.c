#include <stdlib.h>

int GCF(int A, int B) {
    int min_ = A < B ? A : B;
    for (int i = min_; i >= 1; --i) {
        if (A % i == 0 && B % i == 0)
            return i;
    }
}

int* Sort(int* array, int size) {
    if (size <= 1)
        return array;

    int pivot = array[rand() % size];
    int ptr1 = 0,
        ptr2 = size - 1;

    while (ptr1 < ptr2 && (array[ptr1] != pivot || array[ptr2] != pivot)) {
        if (array[ptr1] < pivot) {
            ++ptr1;
        } else {
            while (array[ptr2] > pivot && ptr2 > ptr1)
                --ptr2;

            if (array[ptr2] <= pivot) {
                int tmp = array[ptr1];
                array[ptr1] = array[ptr2];
                array[ptr2] = tmp;
            }
        }
    }

    if (ptr1 == 0)
        ptr1 = 1;

    Sort(array, ptr1);
    Sort(array + ptr1, size - ptr1);

    return array;
}