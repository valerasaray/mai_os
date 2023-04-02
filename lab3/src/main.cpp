#include<iostream>
#include<fstream>
#include<thread>
#include<vector>
#include<malloc.h>
#include<unistd.h>
using namespace std;


void free_pol(int *result, int *pol1, int size){
	for (int i = 0; i<size; i++){
		pol1[i] = result[i]; // копирование элементов из массива значений произведения в массив значений первого множителя
		result[i] = 0; // освобождение массива значений произведения
	}
}

// функция вывода полинома
void print_pol(int *polynomial, int size){
	for (int i = size-1; i >0; i--){
	       cout<<polynomial[i]<<"x^"<<i<<"+";
       }
       cout<<polynomial[0]<<endl;

}	       

// функция умножения полиномов
void new_pol(int *result, int *minm, int *maxm, int n, int m, int K, int np){
	for(int i = 0; i<n; i++) {
		for(int j = (K-(i%K)+np)%K ; j < m; j = j + K){  
		       result [i+j] = result[i+j] + (minm[i] * maxm[j]);
	       }
       }
}    	       

int main(){
	int N; // количество полиномов
	int result_degree=0;  // степень результируюшего многочлена
	cout<<"Введите количество полиномов: ";
	cin>>N;
	// проверка на количество полиномов (N >= 2)
	if (N<2){
		cout<<"Количество полиномов меньше двух";
		return 1;
	}

	int K; // количество потоков
	cout<<"\nВведите ограничение на количество потоков.\nЕсли ограничения нет, то введите -1: ";
	cin>>K;
	// проверка на количество потоков (K >= 0, K == -1)
	if (K<0 && K != -1){
		cout << "Количество потоков отрицательно, и не равно -1"<<endl;
		return 1;
	}

	if (K == -1) {K = 2;} // количество потоков по умолчанию

	int a, max; max = -1;
	cout<<"\nВведите степень первого полинома: ";
	cin>>a; // степень первого полинома
	int all_degrees[N]; // массив всех степеней полиномов
	all_degrees[0]= a; // добавление степени первого полинома
	result_degree = a; // приравнивание результирющей степени к степени первого полинома

	for (int j = 1;j < N; j++){
		cout<<"Введите степень полинома номер " << j+1 << ": "; 
		int degree; // степень текущего полинома
		cin>>degree;

		// определение максимальной степени
		if (degree > max) {
			max = degree;
		}

		all_degrees[j] = degree;  // добавление степени текущего полинома
		result_degree += (degree-1);
	}

	int *pol1 = new int[result_degree]; // массив значений первого множителя
	int *pol2 = new int [max]; // массив значений второго множителя

	cout<<"\nВведите коэффициенты первого полинома, начиная с члена старшей степени: ";
	for (int i = a-1; i>=0; i--){
		cin>>pol1[i]; // ввод коэфицентов первого полинома
	}
        // fflush;
	int *result = new int[result_degree]; // массив результирующего полинома
	for(int i = 0; i<result_degree; i++){
		result[i] = 0;
	}

	int k = 1; // порядковый номер текущего полинома
	
	if (N!=1){
		thread th[K];
	
	// цикл, выполняемый пока не останется лишь один полином
	while (N > 1){

		int b;  // степень текущего результирующего полинома
		b = all_degrees[k];
	    k = k + 1;  // увелечение порядкового номера текущего полинома

		cout<<"Введите коэффициенты полинома номер " << k <<", начиная с члена старшей степени: ";
		for (int i = b-1; i>=0; i--){
			cin>>pol2[i];
		}
		cout<<"  "; print_pol(pol1, a);// fflush; // вывод первого множителя
		cout<<"*"<<endl;
	        cout<<"  ";print_pol(pol2, b); // fflush; // вывод второго множителя
		cout<<"___________________________________"<<endl;
	        
		for (int i = 0; i<K; i++){
			if (a>b){
			     th[i] = thread(new_pol, result, pol1, pol2, a, b, K, i);
			}	
			else{
			      th[i] = thread(new_pol, result, pol2, pol1, b, a, K, i);
			}
		}
		for(int i = 0; i<K; i++){
			th[i].join(); // синхронизация потоков, чтобы дождаться завершения каждого
		}
	
	        for (int i = b-1; i>=0; i--){
		    pol2[i] = 0;
		}	    
		a = a + b - 1;
		N = N - 1; // уменьшение количества полиномов
		print_pol(result, result_degree); // вывод произведения
		free_pol(result, pol1, result_degree); // очистка массива первого множителя
		      	
             }
	  }
	  // освобождение выделенной памяти
    	delete[] result;
        delete[] pol1;
        delete[] pol2;
}
