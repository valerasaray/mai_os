#include<iostream>
#include<stdio.h>
#include<string>
#include"lib.h"
#include"config.h"
using namespace std;

int main()
{
	cout<<"Реализация номер 2"<<endl;
	cout<<"Версия компилятора: "<<COMP_VER<<endl;
	cout<<"ID компилятора: "<<COMP_ID<<endl;
	cout<<"Дата компиляции: "<<TIME_NOW<<endl;
	int command = 0;
	cout<<"Введите номер команды\n0 для смены режима\n1 для сортировки массива\n2 для числа пи:\n";
	while(cin>>command) {
		if (command == 1) {
			int n;
			cout<<"2 Введите число элементов массива: ";
            cin>>n;
			cout<<"2 Введите элементы массива: ";
            int *array = (int *) malloc(n * sizeof(int));
            for (int i = 0; i < n; i++) {
                cin>>array[i];
            }
            array = Sort(array, n);
            cout << "Отсортированный массив:\n";
            for (int i = 0; i < n; i++) {
                cout<<array[i]<<" ";
            }
            cout<<"\n";
			continue;
		}
		if (command == 2) {
			int k;
			float answ;
			cout<<"2 Введите число k: ";
			cin>>k;
			answ = Pi(k);
			cout<<"2 Результат трансляции: " << answ << endl;
			continue;
		}
		cout<<"2 Неверный номер команды"<<endl;
		
	}
}
