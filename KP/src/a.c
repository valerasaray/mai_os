#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 255 // размер буфера для обмена данными между процессами
#define SHARED_MEMORY "/shm_file" // имя разделяемой памяти
#define S_1 "/sem1" // имя первого семафора
#define S_2 "/sem2"  // имя второго семафора
#define S_3 "/sem3" // имя четвертого семафора

int main()
{
    int fd_shm; // дескриптор разделяемой памяти
    char* shmem; // указатель на начало разделяемой памяти
    char* tmp = (char*)malloc(sizeof(char) * BUF_SIZE); // временный буфер для чтения данных
    char* buf_size = (char*)malloc(sizeof(char) * 10); // буфер для хранения размера буфера

    // семафоры для синхронизации процессов
    // sem_open создает новый именованный семафор (параметр O_CREAT).
    // 0660 - задает права доступа
    sem_t* sem1 = sem_open(S_1, O_CREAT, 0660, 0);
    sem_t* sem2 = sem_open(S_2, O_CREAT, 0660, 0);
    sem_t* sem3 = sem_open(S_3, O_CREAT, 0660, 0);

    if (sem1 == SEM_FAILED || sem2 == SEM_FAILED || sem3 == SEM_FAILED) {
        perror("ошибка sem_open в программе A\n");
        exit(1);
    }
    // shm_open() создает новый именованный объект разделяемой памяти, будет открыта как чтение и запись (параметр O_RDW)
    if ((fd_shm = shm_open(SHARED_MEMORY, O_RDWR | O_CREAT, 0660)) == -1) {
        perror("ошибка shm_open в программе A\n");
        exit(1);
    }
    // ftruncate() устанавливает размер (BUF_SIZE) объекта разделяемой памяти, связанного с файловым дескриптором.
    if (ftruncate(fd_shm, BUF_SIZE) == -1) {
        perror("ошибка ftruncate в программе A\n");
        exit(-1);
    }

    /* Создание разделяемой памяти (mmap)
    NULL, указывает на то, что система сама выберет адрес для разделяемой памяти. 
    BUF_SIZE определяет размер памяти
    PROT_WRITE и PROT_READ определяют права доступа к памяти для записи и чтения соответственно.
    MAP_SHARED разделяет память между несколькими процессами
    fd_shm - это файловый дескриптор
    0, указывает на то, что память начинается с начала объекта.*/
    shmem = (char*)mmap(NULL, BUF_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd_shm, 0);
    // Формирование списка аргументов для запуска другого процесса
    sprintf(buf_size, "%d", BUF_SIZE); // форматирует значение переменной BUF_SIZE в строку и записывает ее в буфер buf_size
    char* argv[] = {buf_size, SHARED_MEMORY, S_2, S_3, NULL}; // создает массив указателей на строки, который будет передан в функцию execvp() для запуска нового процесса

    // цикл считывание входных данных со стандартного потока ввода
    while (scanf("%s", tmp) != EOF) {
        // создание дочернего процесса
        pid_t pid = fork();
        // если процесс дочерний
        if (pid == 0) {
            // создание еще одного дочернего процесса pid_1
            pid_t pid_1 = fork();

            if (pid_1 == 0) {
                sem_wait(sem1);
                printf("программа A отправила:\n");
                // запускает новый процесс ./b.out
                if (execve("./b.out", argv, NULL) == -1) {
                    perror("Не удалось выполнить в программе A\n");
                }
            } else if (pid_1 > 0) {
                sem_wait(sem3); // блокирует семафор

                if (execve("./c.out", argv, NULL) == -1) {
                    perror("Не удалось выполнить в программе A\n");
                }
            }
        } else if (pid > 0) {
            sprintf(shmem, "%s", tmp); // запись строки в разделяемую память
            sem_post(sem1); // разблокирует семафор
            sem_wait(sem2); // блокирует семафор
            printf("	\n\n");
        }
    }

    shm_unlink(SHARED_MEMORY); // удаляет именованный сегмент разделяемой памяти
    // удаляет именованные семафоры
    sem_unlink(S_1);
    sem_unlink(S_2);
    sem_unlink(S_3);
    // закрывает все открытые семафоры
    sem_close(sem1);
    sem_close(sem2);
    sem_close(sem3);
    // закрывает дескриптор
    close(fd_shm);
}
