#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <errno.h>
#include <string.h>
#include <set>
#include <algorithm>

using namespace std;

int main(int argc, char const *argv[])
{
    // Определение строковой переменной для гласных букв:
    std::string vovels = "aoueiy";

    // Создание множества символов гласных букв:
    std::set<char> volSet(vovels.begin(), vovels.end());

    // Определение переменной для имени файла:
    string filename = argv[2];
    int fd[2];
    fd[0] = stoi(argv[0]);
    fd[1] = stoi(argv[1]);

    // Создание и открытие файла:
    fstream cur_file;
    cur_file.open(filename, fstream::in | fstream::out | fstream::app);

    while (true)
    {
        // Чтение размера строки из канала:
        int size_of_str;
        read(fd[0], &size_of_str, sizeof(int));

        // Чтение строки из канала:
        char str_array[size_of_str];
        read(fd[0], &str_array, sizeof(char) * size_of_str);

        string result_str;

        // Удаление гласных букв из строки:
        for (int i = 0; i < size_of_str; i++) {
            if (volSet.find(std::tolower(str_array[i])) == volSet.cend()) {
                result_str.push_back(str_array[i]);
            }
        }
        // Запись строки в файл:
        cur_file << result_str << endl;
    }
    return 0;
}
