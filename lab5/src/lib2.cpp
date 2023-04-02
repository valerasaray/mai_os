#include <iostream>

using namespace std;

// сортировка Хоара (быстрая)
void quickSort(int *array, int low, int high) {
    // верхняя и нижняя границы
    int i = low; // индекс первого элемента
    int j = high; // индекс последнего элемента

    // средний элемент подмассива, заданный нижней и верхней границами
    int pivot = array[(i + j) / 2]; // опорный элемент

    int temp;

    while (i <= j) {
        while (array[i] < pivot) i++;
        while (array[j] > pivot) j--;
        if (i <= j) {
            temp = array[i];
            array[i] = array[j];
            array[j] = temp;
            ++i;
            --j;
        }
    }
    if (j > low) quickSort(array, low, j);
    if (i < high) quickSort(array, i, high);
}


extern "C" int * Sort(int * array, int size)
{
    quickSort(array, 0, size - 1);
    return array;
}

// Ряд Лейбница
extern "C" float Pi(int k)
{
    float res = 1;
    // итеративное вычисление суммы ряда
    for (int i = 1; i <= k; i++) {
        float num = (float)(2*i);
        res *= (num / (num - 1)) * (num / (num + 1));
    }
    return res * 2;
}
