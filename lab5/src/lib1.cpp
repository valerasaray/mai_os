#include <iostream>
#include <stdlib.h>

using namespace std;

// Функция перестановки элементов
void Swap(int * a, int * b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Пузырькова сортировка
extern "C" int * Sort(int * array, int size)
{
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (array[j] > array[j+1]) {
                Swap(&array[j], &array[j+1]);
            }
        }
    }
    return array;
}

// ряд Валлиса
extern "C" float Pi(int K)
{
    float pi = 0.0;
    int sign = 1;
    for (int i = 1; i <= K; i += 2) {
        pi += (float)sign * 4.0 / (float)i;
        sign = -sign;
    }
    return pi;
}