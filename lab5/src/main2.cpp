#include<iostream>
#include<stdio.h>
#include<dlfcn.h>
#include"lib.h"
#include<string>
#include"config.h"

using namespace std;

int main()
{
	// информация о номере реализации
	cout<<"Реализация номер 1"<<endl;

	// информации о версии компилятора, его ID и дате компиляции,
	// которые определены в файле "config.h"
	cout<<"Версия компилятора: "<<COMP_VER<<endl;
	cout<<"ID компилятора: "<<COMP_ID<<endl;
	cout<<"Дата компиляции: "<<TIME_NOW<<endl;

	// пути к двум динамическим библиотекам "liblib1.so" и "liblib2.so"
	string lib1 = "./liblib1.so";
	string lib2 = "./liblib2.so";

	int command; // номер команды

	// Функция возвращает указатель на загруженную библиотеку,
	// который сохраняется в переменной "curlib".
	void* curlib = dlopen(lib1.c_str(), RTLD_LAZY); // загружаем первую библиотеку

	// переменные-указатели на функции: "Sort" и "Pi"
	int *(*Sort)(int *, int); // принимает указатель на массив, возвращает указатель на отсортированный
	float (*Pi)(int k); // принимает число k, возвращает значение числа π при заданной длине ряда k
	
	Sort = (int*(*)(int *, int)) dlsym(curlib, "Sort");
    Pi = (float(*)(int))dlsym(curlib, "Pi");
	
	int lib_id = 1; // номер текущей реализации

	cout<<"Введите номер команды\n0 для смены режима\n1 для сортировки массива\n2 для числа пи:\n";
	while(cin>>command) {
		// смена библиотеки
		if (command == 0) {
			dlclose(curlib);  // закрытие текущей библиотеки
			 // открытие другой библиотеки
			if (lib_id == 1) {
				curlib = dlopen(lib2.c_str(), RTLD_LAZY);
				lib_id = 2;
			} else {
				curlib = dlopen(lib1.c_str(), RTLD_LAZY);
				lib_id= 1;
			}
			Sort = (int*(*)(int *, int)) dlsym(curlib, "Sort");
    		Pi = (float(*)(int))dlsym(curlib, "Pi");
			continue;
		}
		if (command == 1) {
			int n; // число элементов массива
			cout<<"Введите число элементов массива: ";
            cin>>n;
			cout<<"Введите элементы массива: ";
            int *array = (int *) malloc(n * sizeof(int)); // создание массива размера n
            // циклический ввод элементов массива
			for (int i = 0; i < n; i++) {
                cin>>array[i];
            }
            array = Sort(array, n); // сортировка массива
            cout << "Отсортированный массив:\n";

			// циклический вывод элементов массива
            for (int i = 0; i < n; i++) {
                cout<<array[i]<<" ";
            }
            cout<<"\n";
			continue;
		}
		if (command == 2) {
			int k; // число k
			float answer; // ответ
			cout<<"Введите число k: ";
			cin>>k;
			answer = Pi(k);
			cout<<"Результат трансляции: " << answer << endl;
			continue;
		}
		cout<<"Неверный номер команды"<<endl;
	}

}
