#include <iostream>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <fstream>
#include <set>

using namespace std;

int main(int argc, char const *argv[])
{
    char *semFile = (char *) argv[2];
    sem_t *sem = sem_open(semFile,  O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH, 0);
    std::string vovels = "aoueiy";
    std::set<char> volSet(vovels.begin(), vovels.end());
    string filename = argv[0];
    fstream cur_file;
    cur_file.open(filename, fstream::in | fstream::out | fstream::app);
    char *backfile = (char *) argv[1];
    int state;
    while (1)
    {
        sem_getvalue(sem, &state); // получение значения семафора
        // проверка значения семафора
        if (state == 0) {
            int fd = shm_open(backfile, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
            struct stat statBuf;
            fstat(fd, &statBuf); // размер файла
            int size_of_str = statBuf.st_size;
            ftruncate(fd, size_of_str);
            char *mapped = (char *) mmap(NULL, size_of_str, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // отображение в память
            std::string allocated = mapped; // содержимое отображенного файла
            string result_str;
            for (int i = 0; i < size_of_str; i++) {
                if (volSet.find(std::tolower(allocated[i])) == volSet.cend()) {
                    result_str.push_back(allocated[i]);
                }
            }
            cur_file << result_str << endl;
            close(fd);
            munmap(mapped, sizeof(int));
            sem_post(sem);
        }
    }
    
}