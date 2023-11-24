int GCF(int A, int B) {
    int tmp;
    while (B != 0) {
        tmp = B;
        B = A % B;
        A = tmp;
    }

    return A;
}

int* Sort(int* array, int size) {
    for (int i = 0; i < size; ++i) {
        int swaps = 0;
        for (int j = 1; j < size; ++j) {
            if (array[j] < array[j - 1]) {
                int tmp = array[j - 1];
                array[j - 1] = array[j];
                array[j] = tmp;
                ++swaps;
            }
        }

        if (swaps == 0)
            break;
    }

    return array;
}