#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

using namespace std;

int main()
{
    string current_str; // текущая строка
    int child_tag; // тэг дочернего процесса (1 или 2)
    int fd[2]; // массив для хранения файловых дескрипторов для каналов связи между процессами
    fstream res_file; // файловый поток для записи результатов работы дочерних процессов
    string child1, child2; // имена файлов для запуска дочерних процессов


    // Ввод имен файлов для дочерних процессов:
    cout << "Введите имя для первого дочернего файла: ";
    cin >> child1;
    cout << "Введите имя для второго дочернего файла: ";
    cin >> child2;

    // Создание каналов связи между процессами
    // fd[0] - для чтения, fd[1] - для записи
    int fd1[2];
    int fd2[2];

    if (pipe(fd1) == -1) // проверка на ошибки
    {
        cout << "Произошла ошибка pipe" << endl;
        exit(EXIT_FAILURE);
    }
    if (pipe(fd2) == -1)
    {
        cout << "Произошла ошибка pipe" << endl;
        exit(EXIT_FAILURE);
    }

    // Создание первого дочернего процесса:
    pid_t f_id1 = fork(); // fork

    if (f_id1 == -1)  // проверка на ошибки
    {
        cout << "Ошибка fork с кодом -1, возвращенным в родительском процессе, child1 не создан" << endl;
        exit(EXIT_FAILURE);
    }
    else if (f_id1 == 0) // если ошибок нет
    {
        fd[1] = fd1[1];
        fd[0] = fd1[0];
        string child = child1;
        execlp("./child", to_string(fd[0]).c_str(), to_string(fd[1]).c_str(), child.c_str(), NULL); // запуск child.cpp
        perror("Ошибка execlp");
    }

    // Создание второго дочернего процесса:
    pid_t f_id2 = fork(); // fork

    if (f_id2 == -1) // проверка на ошибки
    {
        cout << "Ошибка fork с кодом -1, возвращенным в родительском процессе, child2 не создан" << endl;
        exit(EXIT_FAILURE);
    }
    else if (f_id2 == 0) // если ошибок нет
    {
        fd[1] = fd2[1];
        fd[0] = fd2[0];
        string child = child2;
        execlp("./child", to_string(fd[0]).c_str(), to_string(fd[1]).c_str(), child.c_str(), NULL);  // запуск child.cpp
        perror("Ошибка execlp");
    }

    else
    {
        while (getline(std::cin, current_str)) // цикл получения строк из потока ввода
        {
            int s_size = current_str.size() + 1;
            if (current_str.size() <= 10) // проверка на длину строки
            {
                write(fd1[1], &s_size, sizeof(int));
                write(fd1[1], current_str.c_str(), s_size);
            }
            else
            {
                write(fd2[1], &s_size, sizeof(int));
                write(fd2[1], current_str.c_str(), s_size);
            }
        }
    }

    // закрытие файловых дескрипторов
    close(fd2[1]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd1[0]);
    return 0;
}
