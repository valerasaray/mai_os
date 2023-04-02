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

int main(int argc, char const* argv[])
{
    // Проверка количества переданных аргументов (минимум 2: размер буфера и название разделяемой памяти)
    if (argc < 2) {
        perror("args < 2 в программе B\n");
        exit(1);
    }
    
    int buf_size = atoi(argv[0]); // целое число, размер буфера
    char const* shared_memory_name = argv[1]; // название разделяемой памяти
    char const* sem3_name = argv[3]; // название семафора для синхронизации процессов

    int fd_shm;
    // открытие разделяемой памяти в режиме чтения и записи
    if ((fd_shm = shm_open(shared_memory_name, O_RDWR, 0660)) == -1) {
        perror("ошибка shm_open в программе B\n");
        exit(1);
    }

    sem_t* sem3 = sem_open(sem3_name, 0, 0, 0); // Открытие семафора в режиме блокировки
    if (sem3 == SEM_FAILED) {
        perror("ошибка sem3 в программе B\n");
        exit(1);
    }
    char* shmem = (char*)mmap(NULL, buf_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd_shm, 0); // Получение доступа к разделяемой памяти
    int size = strlen(shmem); // получение длины строки

    printf("%d symbols\n", size);
    sem_post(sem3); // разблокирует семафор
}
