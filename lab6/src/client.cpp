#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <unistd.h>
#include <csignal>
#include <thread>
#include <zmq.hpp>
#include <nlohmann/json.hpp>
#include "../include/labtools.h"

// использование пространства имён advancedZMQ
using namespace advancedZMQ;

// объявление объекта nodes класса NTree
NTree nodes;

int main(int argc, char const *argv[])
{
    // создание нового сокета ZeroMQ для связи между процессами
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::pair);
    socket.setsockopt(ZMQ_RCVTIMEO, WAIT_TIME);
    socket.setsockopt(ZMQ_SNDTIMEO, WAIT_TIME);
    // привязка сокета к адресу tcp://127.0.0.1:PORT
    socket.bind(("tcp://127.0.0.1:" + std::to_string(PORT)));

    int pid = fork(); // создание дочернего процесса
    // проверка: является ли процесс дочерним
    if (pid == 0) {
        // вызов исполняемого файла server
        // execl() заменяет текущий процесс новым процессом, который запускает "server"
        execl("./server", "./server", std::to_string(-1).c_str(), NULL);
        return 0;
    }

    // вывод списка команд
    std::cout << "create [id] [parentId] создать узел\n";
    std::cout << "remove [id] [parentId] удалить узел\n";
    std::cout << "exec [id] [start] начать измерение времени на узле\n";
    std::cout << "exec [id] [stop] закончить измерение времени на узле\n";
    std::cout << "exec [id] [time] посмотреть измеренное время\n";
    std::cout << "ping [id] проверить наличие узла\n";
    std::cout << "print показать дерево\n";
    std::cout << "help чтобы показать команды снова\n\n";
    std::string command;

    // бесконечный цикл ввода команд пользователем
    while (std::cin >> command) {


        if (command == "create") {
            int id; // дочерний узел, который нужно создать
            int parentId; // родительский узел, к которому нужно присоединить новый узел
            std::cin >> id >> parentId;

            // проверка на существование родительского узла с заданным parentId
            if (nodes.findNode(parentId).second == -1 && parentId != -1) {
                // вывод ошибки, если он не существует
                std::cout << "Ошибка: родительский узел номер " << parentId << " не существует" << std::endl;
                continue;
            }
            // проверка на существование дочернего узла с заданным id
            if (nodes.find(parentId, id) != -1) {
                std::cout << "EОшибка: дочерний узел номер " << id << " уже существует" << std::endl;
                continue;
            }
            // создание json запроса pingRequest для проверки доступности родительского узла
            nlohmann::json pingRequest;
            // содержание запроса:
            pingRequest["type"] = "ping";
            pingRequest["id"] = parentId;

            // использование функции sendAndRecv для отправки pingRequest на сервер
            // и получения ответа pingReply
            nlohmann::json pingReply = sendAndRecv(pingRequest, socket, 0);

            // проверка на содердание в ответе строки "ok" (родительский узел доступен)
            if (pingReply["ans"] != "ok") {
                socket.close(); // закрытие сокета
                context.close();
                std::cout << "Ошибка: родительский узел номер " << parentId << " недоступен" << std::endl;
                continue;
            }
            // создание json запроса request 
            nlohmann::json request;
            // содержание запроса
            request["type"] = "create";
            request["id"] = id;
            request["parentId"] = parentId;
            
            // отправление на сервер запроса с помощью функции sendAndRecv,
            // которая возвращает ответ сервера в виде запроса reply
            nlohmann::json reply = sendAndRecv(request, socket, 0);

            // проверка на содержания строки "ok" в поле "ans"
            // (новый узел успешно добавлен к родительскому узлу)
            if (reply["ans"] != "ok") {
                socket.close(); // закрытие сокета
                context.close();
                std::cout << "Ошибка: родительский узел номер " << parentId << " недоступен" << std::endl;
                continue;
            } else {
                // id нового узла добавляется в дерево.
                nodes.insert(parentId, id);
            }



        } else if (command == "remove") {
            int id;
            int parentId;
            std::cin >> id;

            // проверка - существует ли узел с таким идентификатором в дереве
            if (nodes.findNode(id).second == -1) {
                std::cout << "Ошибка узел номер " << id << " не найден" << std::endl;
                continue;
            }
            // определение идентификатора родительского узла
            parentId = nodes.findNode(id).first;
            
            // создание json запроса pingRequest для проверки доступности родительского узла
            nlohmann::json pingRequest;
            pingRequest["type"] = "ping";
            pingRequest["id"] = parentId;

            // отправление на сервер запроса с помощью функции sendAndRecv,
            // которая возвращает ответ сервера в виде запроса reply
            nlohmann::json pingReply = sendAndRecv(pingRequest, socket, 0);
            
            // проверка на содердание в ответе строки "ok" (родительский узел доступен)
            if (pingReply["ans"] != "ok") {
                std::cout << "Ошибка: родительский узел номер " << parentId << " недоступен" << std::endl;
                continue;
            }

            // создание json запроса request
            nlohmann::json request;
            // формируется запрос на удаление
            request["type"] = "remove";
            // идентификатор элемента
            request["id"] = id;
            request["parentId"] = parentId;
            // запрос на удаление
            nlohmann::json reply = sendAndRecv(request, socket, 0);
            if (reply["ans"] != "ok") {
                std::cout << "Ошибка узел номер  " << id << "  недоступен" << std::endl;
                continue;
            } else {
                // удаление
                nodes.destroyUndertree(id);
                std::cout << "ok" << std::endl;
            }



        } else if (command == "print") {
            // вывод дерева
            nodes.print();
        } else if (command == "exec") {
            int id, curInd;
            std::string string;
            std::cin >> id >> string;
            if (nodes.findNode(id).second != -1) {
                nlohmann::json request;
                request["type"] = "exec";
                request["id"] = id;
                request["command"] = string;
                nlohmann::json reply = sendAndRecv(request, socket, 0);
                if (reply["ans"] != "ok") {
                    std::cout << "Ошибка узел номер  " << id << "  недоступен" << std::endl;
                }
            } else {
                std::cout << "Ошибка узел номер  " << id << " не найден" << std::endl;
            }



        } else if (command == "ping") {
            int id = -1;
            std::cin >> id;
            nlohmann::json request;
            request["type"] = "ping";
            request["id"] = id;
            nlohmann::json reply = sendAndRecv(request, socket, 0);
            if (reply["ans"] == "ok") {
                std::cout << "ok:" << id << std::endl;
            } else {
                std::cout << "Ошибка узел номер  " << id << "  недоступен" << std::endl;
            }



        } else if (command == "help") {
            std::cout << "\ncreate [id] [parentId] создать узел\n";
            std::cout << "\nremove [id] [parentId] удалить узел\n";
            std::cout << "\nexec [id] [start] начать измерение времени на узле\n";
            std::cout << "\nexec [id] [stop] закончить измерение времени на узле\\n";
            std::cout << "\nexec [id] [time] посмотреть измеренное время\n";
            std::cout << "\nping [id] проверить наличие узла\n";
            std::cout << "\nprint показать дерево\n";
            std::cout << "\nhelp чтобы показать команды снова\n\n";
        }
    }


    // проверка, достигнут ли конец файла
    if (std::cin.eof()) {
        // создание объекта JSON для запроса на удаление узла
        nlohmann::json destroyRequest;
        destroyRequest["type"] = "remove";
        // поиск всех дочерних узлов корня дерева и сохранение их вектора целых чисел
        std::vector<int> nodesRoot = nodes.findChilds(-1);
        // цикл, в котором отправляются запросы на удаление всех дочерних узлов корня.
        for (int i = 0; i < nodesRoot.size(); i++) {
            destroyRequest["id"] = nodesRoot[i];
            sendAndRecv(destroyRequest, socket, 0);
        }   
    }
    socket.close();
    return 0;
}
