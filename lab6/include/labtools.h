#ifndef __LABTOOLS_H__
#define __LABTOOLS_H__

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <zmq.hpp>
#include <nlohmann/json.hpp>

// проверка на ошибку
#define CHECK_ERROR(expr, err, message) \
    do { \
        auto __result = (expr); \
        if (__result == err) { \
            fprintf(stderr, "Error: %s\n", message); \
            fprintf(stderr, "errno = %s, file %s, line %d\n", strerror(errno), \
                    __FILE__, __LINE__); \
            exit(-1); \
        } \
    } while (0)

const int WAIT_TIME = 1000; // время ожидания при сетевом взаимодействии
const int PORT = 7000; // номер порта для сетевого взаимодействия

class NTree
{
public:
    // вложенный класс NTree (отображение ключа на множество дочерних узлов)
    using Node = std::map<int, std::set<int>>;
    Node node;
    NTree();
    void print(); // текущее состояние дерева
    int dfs(int child, int curChild); // обход дерева в глубину
    int findCheck(int parent, int child); // проверка на существование связи между двумя узлами
    int find(int parent, int child);
    std::pair<int, int> findNode(int node); // возврат пары родительский и дочерний узео
    std::vector<int> findChilds(int node); // вывод дочерних узлов в виде вектора
    int insert(int parent, int child); // создание связи между узлами
    int erase(int parent, int child); // удаление связи
    void destroyUndertree(int node); // удаление из дерева поддерева
    ~NTree();
};
// конструктор класса, который инициализирует дерево пустым множеством корневых узлов
NTree::NTree()
{
    std::set<int> root;
    this->node.insert({-1, root});
}
// деструктор класса
NTree::~NTree()
{
}

// вывод дерева на консоль
void NTree::print()
{
    for (auto it = this->node.begin(); it != this->node.end(); it++) {
        std::cout << it->first << ": ";
        for (auto it1 = it->second.begin(); it1 != it->second.end(); it1++) {
            std::cout << *it1 << " ";
        }
        std::cout << "\n";
    }
}

// обход дерева в глубину
int NTree::dfs(int child, int curChild) {
    for (auto i: this->node[curChild]) {
        // std::cout << i << "\n";
        if (i == child) {
            return 1;
        }
        return NTree::dfs(child, i);
    }
    return -1;
}

// поиск
int NTree::find(int parent, int child)
{
    int ans = 0;
    for (auto curChild: this->node[parent]) {
        if (curChild == child) {
            return ans;
        } else if (dfs(child, curChild) != -1) {
            return ans;
        }
        ans++;
    }
    return -1;
}

// поиск узлов
std::pair<int, int> NTree::findNode(int node)
{
    for (auto i: this->node) {
        if (i.second.find(node) != i.second.cend()) {
            return std::make_pair(i.first, 0);
        }
    }
    return std::make_pair(-1, -1);
}

// поиск дочерних узлов
std::vector<int> NTree::findChilds(int node)
{
    std::vector<int> res;
    for (auto i: this->node[node]) {
        res.push_back(i);
    }
    return res;
}


int NTree::findCheck(int parent, int child)
{
    for (auto curChild: this->node[parent]) {
        if (curChild == child) {
            return 1;
        }
    }
    return -1;
}

int NTree::insert(int parent, int child)
{
    auto curParent = this->node.find(parent);
    if (curParent != this->node.cend()) {
        auto curChild = curParent->second.find(child);
        if (curChild != curParent->second.cend()) {
            return 0;
        }
        curParent->second.insert(child);
        std::set<int> childVec;
        this->node.insert({child, childVec});
        return 1;
    }
    return 0;
}

int NTree::erase(int parent, int child)
{
    auto curParent = this->node.find(parent);
    if (curParent != this->node.cend()) {
        auto curChild = curParent->second.find(child);
        if (curChild != curParent->second.cend()) {
            curParent->second.erase(child);
            auto p = this->node.find(child);
            this->node.erase(p);
            return 1;
        }
    }
    return 0;
}

void NTree::destroyUndertree(int node)
{
    auto curNode = this->node.find(node);
    if (curNode != this->node.cend()) {
        for (auto it: curNode->second) {
            this->destroyUndertree(it);
        }
    }
    curNode->second.clear();
    int parent = this->findNode(node).first;
    auto parentN = this->node.find(parent);
    parentN->second.erase(node);
    this->node.erase(node);
}

namespace advancedZMQ
{
    nlohmann::json sendAndRecv(nlohmann::json &request, zmq::socket_t &socket, int debug)
    {
        std::string strFromJson = request.dump();
        if (debug) {
            std::cout << strFromJson << std::endl;
        }
        zmq::message_t msg(strFromJson.size());
        memcpy(msg.data(), strFromJson.c_str(), strFromJson.size());
        socket.send(msg);
        nlohmann::json reply;
        zmq::message_t msg2;
        socket.recv(msg2);
        std::string strToJson(static_cast<char *> (msg2.data()), msg2.size());
        if (!strToJson.empty()) {
            reply = nlohmann::json::parse(strToJson);
        } else {
            if (debug) {
                std::cout << "bad socket" << std::endl;
            }
            reply["ans"] = "error";
        }
        if (debug) {
            std::cout << reply << "\n";
        }
        return reply;
    }

    void Send(nlohmann::json &request, zmq::socket_t &socket)
    {
        std::string strFromJson = request.dump();
        zmq::message_t msg(strFromJson.size());
        memcpy(msg.data(), strFromJson.c_str(), strFromJson.size());
        socket.send(msg);
    }

    nlohmann::json Recv(zmq::socket_t &socket)
    {
        nlohmann::json reply;
        zmq::message_t msg;
        socket.recv(msg);
        std::string strToJson(static_cast<char *> (msg.data()), msg.size());
        reply = nlohmann::json::parse(strToJson);
        return reply;
    }
}

#endif