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
#include <chrono>


// Замер времени выполнения операций
class Timer {
public:
    // время начала измерения времени выполнения
    void start() {
        m_startTime = std::chrono::high_resolution_clock::now();
        m_running = true;
    }
    // время окончания измерения времени выполнения
    void stop() {
        m_endTime = std::chrono::high_resolution_clock::now();
        m_running = false;
    }
    // время выполнения
    double time() {
        std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

        if (m_running) {
            endTime = std::chrono::high_resolution_clock::now();
        } else {
            endTime = m_endTime;
        }

        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_endTime;
    bool m_running = false;
};



using namespace advancedZMQ;

std::vector<zmq::socket_t> childSockets;
std::vector<int> childIds;

int main(int argc, char const *argv[])
{
    int curId = std::stoi(std::string(argv[1])), flag = 1;
    // Инициализация контекста ZeroMQ с одним потоком выполнения
    zmq::context_t parentContext(1);
    // Создание ZeroMQ-сокета типа pair с использованием контекста parentContext
    zmq::socket_t parentSocket(parentContext, zmq::socket_type::pair);
    // Подключение созданного сокета parentSocket к адресу "tcp://127.0.0.1:(PORT + curId + 1)
    parentSocket.connect(("tcp://127.0.0.1:" + std::to_string(PORT + curId + 1)));
    // Создание объекта Timer для измерения времени
    Timer timer;
    void *arg = NULL;
    while (flag) {
        // Получение JSON-сообщения от parentSocket
        nlohmann::json reply = Recv(parentSocket);
        // Создание нового JSON-сообщения request 
        nlohmann::json request;
        request["type"] = reply["type"];
        request["id"] = curId;
        request["pid"] = 0;
        request["ans"] = "error";


        if (reply["type"] == "ping") {
            // проверка на совпадение id
            if (reply["id"] == curId) {
                // формирование ответного сообщения
                request["ans"] = "ok";
                request["id"] = curId;
                request["pid"] = getpid();
            } else {
                // обход вектора дочерних сокетов и отправление на каждый новое сообщение типа "ping"
                for (int i = 0; i < childSockets.size(); i++) {
                    nlohmann::json pingRequest;
                    pingRequest["type"] = "ping";
                    pingRequest["id"] = reply["id"];
                    nlohmann::json pingReply = sendAndRecv(pingRequest, childSockets[i], 0);
                    if (pingReply["ans"] == "ok") {
                        // формирование ответного сообщения
                        request["ans"] = "ok";
                        request["id"] = curId;
                        request["pid"] = getpid();
                        break;
                    }
                }
            }

        
        } else if (reply["type"] == "create") {
            if (reply["parentId"] == curId) {
                int i = reply["id"];
                // Создается новый сокет для взаимодействия с созданным процессом
                zmq::socket_t childSocket(parentContext, zmq::socket_type::pair);
                // задаются опции сокета для установки времени ожидания приема и отправки сообщений
                childSocket.setsockopt(ZMQ_RCVTIMEO, WAIT_TIME);
                childSocket.setsockopt(ZMQ_SNDTIMEO, WAIT_TIME);
                // Устанавливается соединение между сокетами родительского и созданного процессов.
                childSocket.bind(("tcp://*:" + std::to_string(PORT + i + 1)).c_str());
                // создается новый процесс
                int pid = fork();
                if (pid == 0) {
                    int i = reply["id"];
                    // запуск исполняемого файла "server"
                    execl("./server", "./server", std::to_string(i).c_str(), NULL);
                    // завершение работы
                    return 0;

                
                } else {
                    // Создание JSON-объекта pingRequest
                    nlohmann::json pingRequest;
                    pingRequest["type"] = "ping";
                    pingRequest["id"] = reply["id"];
                    // Отправка созданного запроса на сокет childSocket
                    // Ожидание ответа в течение 0 миллисекунд с помощью функции sendAndRecv
                    nlohmann::json pingReply = sendAndRecv(pingRequest, childSocket, 0);
                    if (pingReply["ans"] == "ok") {
                        // Вывод на экран значения поля "pid
                        std::cout << "OK: " << pingReply["pid"] << std::endl;
                        int i = reply["id"];
                        // Добавление сокета childSocket в вектор childSockets
                        childSockets.push_back(std::move(childSocket));
                        // Добавление идентификатора reply["id"] в вектор childIds
                        childIds.push_back(reply["id"]);
                        // Установка значения поля "ans" в "ok" в JSON-объекте request
                        request["ans"] = "ok";
                        // Установка значения поля "parentId" в идентификатор родительского процесса 
                        request["parentId"] = reply["parentId"];
                    }
                }


            } else {
                // Создание нового JSON-объекта newRequest, который получает те же поля, что и reply
                nlohmann::json newRequest = reply;
                // Запуск цикла по всем имеющимся дочерним сокетам в childSockets
                for (int i = 0; i < childSockets.size(); i++) {
                    nlohmann::json newReply = sendAndRecv(newRequest, childSockets[i], 0);
                    // Если ответ содержит поле "ans", равное "ok"
                    if (newReply["ans"] == "ok") {
                        // то устанавливается значение поля "ans" в исходном запросе request равным "ok"
                        request["ans"] = "ok";
                        break;
                    }
                }
            }


        } else if (reply["type"] == "remove") {
            // количество удаленных дочерних процессов
            int c = 0;
            // цикл по всем дочерним сокетам
            for (int i = 0; i < childSockets.size(); i++) {
                //  отправляем каждому из них сообщение типа "destroy" с идентификатором удаляемого процесса
                int childId = childIds[i];
                nlohmann::json destroyRequest;
                destroyRequest["type"] = "destroy";
                // Если идентификатор найден и удаление прошло успешно
                if (childId == reply["id"]) {
                    // удаляем соответствующий дочерний сокет и идентификатор из векторов childSockets и childIds
                    Send(destroyRequest, childSockets[i]);
                    int k = reply["id"];
                    childSockets.erase(std::next(childSockets.begin() + i));
                    childIds.erase(childIds.begin() + i);
                    c++;
                    break; 
                }
            }
            // Если был удален хотя бы один дочерний процесс
            if (c > 0) {
                // устанавливаем ответ "ok" в сообщении.
                request["ans"] = "ok";
                c = 0;
            }
            // Отправка оставшимся дочерним процессам полученное сообщение для проверки корректности удаления.
            for (int i = 0; i < childSockets.size(); i++) {
                nlohmann::json newReply = sendAndRecv(reply, childSockets[i], 0);
                if (newReply["ans"] == "ok") {
                    request["ans"] = "ok";
                }
            }


        } else if (reply["type"] == "destroy") {
            nlohmann::json newRequest = reply;
            for (int i = 0; i < childSockets.size(); i++) {
                int childId = childIds[i];
                Send(newRequest, childSockets[i]);
            }
            flag = 0;


        } else if (reply["type"] == "exec") {
            // если идентификатор, указанный в запросе, совпадает с идентификатором текущего процесса
            if (reply["id"] == curId) {
                if (reply["command"] == "start") {
                    // запуск таймера
                    timer.start();
                    std::cout << "Ok:" << reply["id"] << std::endl;
                    request["ans"] = "ok";
                } else if (reply["command"] == "time") {
                    // получаем текущее время таймера
                    std::cout << "Ok:" << reply["id"] << ":" << timer.time() << std::endl;
                    request["ans"] = "ok";
                } else if (reply["command"] == "stop") {
                    // останавка таймера
                    timer.stop();
                    std::cout << "Ok:" << reply["id"] << std::endl;
                    request["ans"] = "ok";
                } else {
                    // ошибка
                    request["ans"] = "error";
                }


            } else {
                nlohmann::json newRequest = reply;
                // цикл по всем дочерним сокетам
                for (int i = 0; i < childSockets.size(); i++) {
                    // Отправка запроса newRequest и ожидание ответа sendAndRecv(). Результат сохраняется в переменную newReply
                    nlohmann::json newReply = sendAndRecv(newRequest, childSockets[i], 0);
                    if (newReply["ans"] == "ok") {
                        request["ans"] = "ok";
                        break;
                    }
                }
            }
        }
        // отправляет сообщение request на сокет
        Send(request, parentSocket);
    }
    // закрывает все сокеты, используемые для связи с дочерними процессами.
    for (int i = 0; i < childSockets.size(); i++) {
        childSockets[i].close();
    }
    parentSocket.close();
    return 0;
}
