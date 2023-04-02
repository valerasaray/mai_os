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

int main(int argc, char* const argv[])
{
    // Проверка аргументов командной строки (минимум 2: размер буфера и название разделяемой памяти)
    if (argc < 2) {
        printf("args < 2 в программе C\n");
        return 0;
    }

    int buf_size = atoi(argv[0]); // размер буфера
    char const* shared_memory_name = argv[1]; // имя разделяемой памяти
    // имена семафоров
    char const* sem2_name = argv[2];
    char const* sem3_name = argv[3];
    int fd_shm;
    // Открытие разделяемой памяти в режиме чтения и записи
    if ((fd_shm = shm_open(shared_memory_name, O_RDWR, 0660)) == -1) {
        perror("ошибка shm_open в программе C\n");
        exit(1);
    }

    sem_t* sem2 = sem_open(sem2_name, 0, 0, 0); // Открытие семафора 2 в режиме блокировки
    sem_t* sem3 = sem_open(sem3_name, 0, 0, 0); // Открытие семафора 3 в режиме блокировки

    if (sem2 == SEM_FAILED || sem3 == SEM_FAILED) {
        perror("ошибка sem2 или sem3 в программе C\\n");
        exit(1);
    }
    // Отображение разделяемой памяти в виртуальное адресное пространство (чтобы иметь доступ)
    char* shmem = (char*)mmap(NULL, buf_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd_shm, 0);
    // Создание нового процесса
    pid_t p = fork();

    // если дочерний процесс
    if (p == 0) {
        printf("program C got:\n");
        // запуск программы B
        if (execve("b.out", argv, NULL) == -1) {
            perror("ошибка execve в программе C\n");
            exit(1);
        }
    } else if (p > 0) {
        sem_wait(sem3); // блокирует семафор
        printf("%s\n", shmem);
    }
    
    sem_post(sem2); // разблокирует семафор
}
