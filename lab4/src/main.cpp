#include <iostream>
#include <string>
#include <mutex>
#include <algorithm>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <fstream>
#include <string.h>

using namespace std;

int main()
{
    string current_str; // текущая строка, введенная пользователем
    int child_tag; // тег дочернего процесса
    fstream res_file; // файловый поток для записи результата
    string child1, child2; // имена для двух дочерних файлов

    cout << "Введите имя для первого дочернего файла: ";
    cin >> child1;
    cout << "Введите имя для второго дочернего файла: ";
    cin >> child2;

    // Удаление предыдущей разделяемой памяти и семафоров с именами "1.back", "2.back", "_sem1" и "_sem2":
    shm_unlink("1.back");
    shm_unlink("2.back");
    sem_unlink("_sem1");
    sem_unlink("_sem2");

    // Создание семафоров для синхронизации между двумя процессами:
    // Изначально счетчик каждого семафора равен 0
    sem_t *sem1 = sem_open("_sem1",  O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH, 0);
    sem_t *sem2 = sem_open("_sem2",  O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH, 0);
    int state = 0;
    std::mutex mx;
    mx.lock();
    while (++state < 2) {
        sem_post(sem1); // освобождение семафора
    }
    while (--state > 1) {
        sem_wait(sem1); // блокировка процесса
    }
    mx.unlock();
    state = 0;
    mx.lock();
    while (++state < 2) {
        sem_post(sem2);  // освобождение семафора
    }
    while (--state > 1) {
        sem_wait(sem2);  // блокировка процесса
    }
    mx.unlock();

    // Создание двух процессов-потомков:
    pid_t f_id1 = fork();
    if (f_id1 == -1) // проверка на ошибки
    {
        cout << "Ошибка fork с кодом -1, возвращенным в родительском процессе, child1 не создан" << endl;
        exit(EXIT_FAILURE);
    }
    else if (f_id1 == 0) // если ошибок нет (дочерний процесс)
    {
        sem_close(sem1);
        string child = child1;
        execlp("./child", child.c_str(), "1.back", "_sem1", NULL);
        perror("Ошибка execlp");
    }


    pid_t f_id2 = fork();  // проверка на ошибки  (дочерний процесс)
    if (f_id2 == -1)
    {
        cout << "Ошибка fork с кодом -1, возвращенным в родительском процессе, child2 не создан" << endl;
        exit(EXIT_FAILURE);
    }
    else if (f_id2 == 0)  // если ошибок нет
    {
        sem_close(sem2);
        string child = child2;
        execlp("./child", child.c_str(), "2.back", "_sem2", NULL);
        perror("Ошибка execlp");
    }

    else
    {
        while (getline(std::cin, current_str))
        {
            int s_size = current_str.size() + 1;
            char *buffer = (char *) current_str.c_str();
            if (current_str.size() <= 10)
            {
                int fd = shm_open("1.back", O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
                ftruncate(fd, s_size); // задание размера файла
                char *mapped = (char *) mmap(NULL, s_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                memset(mapped, '\0', s_size); // заполнение блока памяти
                sprintf(mapped, "%s", buffer); // форматирование строки перед записью в разделяемую памать
                munmap(mapped, s_size); // Родительский процесс размещает строки в разделяемой памяти
                close(fd); // закрытие файлового дескриптора
                sem_wait(sem1);
            }
            else
            {
                int fd = shm_open("2.back", O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
                ftruncate(fd, s_size); // задание размера файла
                char *mapped = (char *) mmap(NULL, s_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                memset(mapped, '\0', s_size); // заполнение блока памяти
                sprintf(mapped, "%s", buffer); // форматирование строки перед записью в разделяемую памать
                munmap(mapped, s_size); // Родительский процесс размещает строки в разделяемой памяти
                close(fd); // закрытие файлового дескриптора
                sem_wait(sem2);
            }
        }
    }
    return 0;
}
