#include <iostream>
#include <zmq.hpp>
#include <list>
#include <map>
#include <thread>
#include <mutex>

#include "lib.hpp"

using namespace zmq;

const int MAX_LEN_MSG = 256;

std::mutex print_mutex;
void print(std::vector<std::string> vec) {
    std::lock_guard<std::mutex> lock(print_mutex);
    for (auto elem : vec)
        std::cout << elem;
}

int create_process() {
    pid_t pid = fork();
    if (-1 == pid) {
        perror("fork");
        exit(-1);
    }
    return pid;
}
void kill_process(pid_t pid) {
    kill(pid, SIGTERM);
}

struct Node {
    int id;
    bool isChild;
    int idParent;
    pid_t pid;
};
std::ostream& operator<<(std::ostream& out, Node& node) {
    out << "id: " << node.id << " isChild: " << node.isChild << " idParent: " << node.idParent << " pid: " << node.pid << std::endl;
    return out;
}

void changeMainCalcNode(socket_t &pub, int &idMainCalcNode, int id) {
    // std::cout << "**********смена пажского караула**********" << std::endl;
    pub.bind(getAddrNext(id));
    idMainCalcNode = id;
}

void sendMsg(std::string msg, socket_t &pub) {
    mutable_buffer mbuf = buffer(msg);
    auto res_s = pub.send(mbuf, send_flags::none);
    if (!res_s.has_value()) {
        print({"Ошибка при отправке"});
        return;
    }
}

void execThread(std::string msg, socket_t &pub, socket_t &sub, int id) {
    mutable_buffer mbuf = buffer(msg);
    auto res_s = pub.send(mbuf, send_flags::none);
    if (!res_s.has_value()) {
        print({"Ошибка при отправке"});
        return;
    }

    message_t response(msg.size());
    auto res_r = sub.recv(response, recv_flags::none);
    if (!res_r.has_value()) {
        print({"Ошибка при получении"});
        return;
    }

    std::string res = response.to_string();
    std::stringstream ss;
    ss << "Ok:" << id << ": " << res << "\n";
    print({ss.str()});

    return;
}

std::optional< std::pair<int, int> > killNode(int id, std::list<Node> &nodes, int &idMainCalcNode, socket_t &pub, std::map<int, std::chrono::_V2::system_clock::time_point> &heartbits) {
    if (id == -1) {
        return {};
    }

    std::pair<int, int> res;

    auto i = nodes.begin();
    for (; i != nodes.end(); ++i) {
        if (i->id == id) {
            heartbits.erase(id);

            res.first = std::prev(i, 1)->id;
            if (std::next(i, 1) != nodes.end())
                res.second = std::next(i, 1)->id;
            else
                res.second = -2;

            
            print({"Ok: ", itos(i->pid), "\n"});
            nodes.erase(i);

            auto j = nodes.begin();
            for (; j != nodes.end(); ++j) {
                if (j->isChild && j->idParent == id) {
                    res.second = killNode(j->id, nodes, idMainCalcNode, std::ref(pub), heartbits)->second;
                    j = nodes.begin();
                }
            }

            break;
        }
    }

    if (i == nodes.end()) {
        return {};
    }

    return res;
}

std::optional< std::pair<int, int> > killNodeFull(int id, std::list<Node> &nodes, int &idMainCalcNode, socket_t &pub, socket_t &sub, std::map<int, std::chrono::_V2::system_clock::time_point> &heartbits, std::vector<std::thread> &threads) {
    auto res = killNode(id, nodes, idMainCalcNode, std::ref(pub), heartbits);
    if (!res)
        return {};

    // std::cout << "Границы при удалении: " << res->first << " " << res->second << "\n";

    std::stringstream ss;
    ss << "unbind " << res->first << " " << res->second << " search";
    if (res->first == -1)
        ss << " last";
    else
        ss << " first";
    threads.emplace_back(execThread, ss.str(), std::ref(pub), std::ref(sub), res->first);

    threads.back().join();

    if (id == idMainCalcNode) {
        if (nodes.size() > 1) {
            changeMainCalcNode(std::ref(pub), idMainCalcNode, std::next(nodes.begin(), 1)->id);
        } else {
            idMainCalcNode = -1;
        }
    }

    if (res->first != -1) {
        ss.str("");
        ss << "rebind " << res->first << " next " << res->second;
        sendMsg(ss.str(), pub);
        std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(100)));
    }

    if (res->second != -2) {
        std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(100)));
        // std::cout << "Буду менять предыдущий" << std::endl;
        ss.str("");
        ss.clear();
        ss << "rebind " << res->second << " prev " << res->first;
        sendMsg(ss.str(), pub);
        std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(100)));
    }

    return res;
}

void heartbitCheck(std::map<int, std::chrono::_V2::system_clock::time_point> &heartbits, std::chrono::milliseconds time, std::list<Node> &nodes, int &idMainCalcNode, socket_t &pub, socket_t &sub, std::vector<std::thread> &threads) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::duration(4 * time));

        std::vector<int> unavailableNodes;
        auto end = std::chrono::system_clock::now();
        // print({"Начинаю проверку узлов\n"});
        std::cout << "Начинаю проверку узлов\n";
        for (auto elem : heartbits) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( end - elem.second );
            // print({itos(elem.first), ": ", itos(duration.count()), "\n"});
            std::cout << itos(elem.first) << ": " << itos(duration.count()) << "\n";
            if (duration > 4 * time) {
                // print({"Heartbit: node ", itos(elem.first), " is unavailable now\n"});
                std::cout << "Heartbit: node " << itos(elem.first) << " is unavailable now\n";

                unavailableNodes.push_back(elem.first);
            }
        }

        for (int id : unavailableNodes)
            killNodeFull(id, nodes, idMainCalcNode, std::ref(pub), std::ref(sub), heartbits, std::ref(threads));
    }
}
void heartbitRecv(socket_t &sub, std::map<int, std::chrono::_V2::system_clock::time_point> &heartbits, int time) {
    while (true) {
        zmq::message_t m(MAX_LEN_MSG);
        auto res = sub.recv(m, zmq::recv_flags::none);
        if (!res.has_value()) {
            std::cout << "Ошибка при получении" << std::endl;
            return;
        }

        heartbits[stoi(m.to_string())] = std::chrono::system_clock::now();
        // std::cout << m.to_string() << "\n";
    }
}

int main() {
	std::list<Node> nodes{{-1, 0, 0, 0}};

    context_t ctx;
    socket_t pub(ctx, socket_type::push);
    socket_t sub(ctx, socket_type::pull);
    sub.connect(getAddrPrev(-1));
    std::string addr;
    int idMainCalcNode = -1;

    std::vector<std::thread> threads;
    
    socket_t subHeartbit(ctx, socket_type::pull);
    subHeartbit.bind(getAddrNext(-1));
    std::map<int, std::chrono::_V2::system_clock::time_point> heartbits;
    std::vector<std::thread> threadsHeartbit;

    while (true) {
        bool ok = true;

        print_mutex.lock();
        std::cout << "$ ";
        std::string s;
        std::cin >> s;
        print_mutex.unlock();

        if (s == "create") {
            int id;
            std::cin >> id;

            if (id == -2) {
                std::cout << "Error: Cannot create node with id = -2" << std::endl;
            }

            for (auto elem : nodes) {
                if (elem.id == id) {
                    std::cout << "Error: Already exists" << std::endl;
                    ok = false;
                    break;
                }
            }

            int idParent = -2;
            bool isChild = false;
            getline(std::cin, s);
            if (s.size() > 0) {
                idParent = stoi(s);
                isChild = true;
            }

            if (!ok) continue;

            int idPrev, idNext;
            auto iterParent = nodes.end();
            if (isChild) {
                idPrev = idParent;

                auto i = nodes.begin();
                for (; i != nodes.end(); ++i)
                    if (i->id == idParent) {
                        iterParent = i;
                        ++i;
                        break;
                    }

                if (iterParent == nodes.end()) {
                    std::cout << "Error: Parent not found" << std::endl;
                    continue;
                }
                
                if (i != nodes.end())
                    idNext = i->id;
                else
                    idNext = -2;
            } else {
                idNext = -2;
                idPrev = nodes.back().id;
            }

            pid_t process_id = create_process();
            if (process_id == 0) {
                std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(1000)));
                execl("client.out", "client.out", itos(idPrev).c_str(), itos(id).c_str(), itos(idNext).c_str(), NULL);
                perror("exec");
                exit(-1);
            }

            heartbits[id] = std::chrono::_V2::system_clock::time_point();
            
            Node node{id, isChild, idParent, process_id};
            if (isChild) {
                auto iterNode = nodes.insert(++iterParent, node);

                std::stringstream ss;
                if (idParent != -1) {
                    int idNodePrev = idParent;
                    ss << "rebind " << idNodePrev << " next " << id;
                    sendMsg(ss.str(), pub);
                    std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(100)));
                }

                auto iterNodeNext = std::next(iterNode);
                if (iterNodeNext != nodes.end()) {
                    int idNodeNext = iterNodeNext->id;
                    ss.str("");
                    ss << "rebind " << idNodeNext << " prev " << id;
                    sendMsg(ss.str(), pub);
                    std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(100)));
                }
            } else {
                nodes.push_back(node);
                
                auto iterNode = std::prev(nodes.end(), 2);
                int idNode = iterNode->id;
                if (idNode != -1) {
                    std::stringstream ss;
                    ss << "rebind " << idNode << " next " << id;
                    sendMsg(ss.str(), pub);
                    std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(100)));
                }
            }

            if (idMainCalcNode == -1 || idParent == -1)
                changeMainCalcNode(pub, idMainCalcNode, id);

            print({"Ok: ", itos(process_id), "\n"});

            for (auto elem : nodes) {
                print_mutex.lock();
                std::cout << elem;
                print_mutex.unlock();
            }
        } else if (s == "kill") {
            int id;
            std::cin >> id;

            auto res = killNodeFull(id, nodes, idMainCalcNode, std::ref(pub), std::ref(sub), std::ref(heartbits), std::ref(threads));
            if (!res) {
                std::cout << "Error: Node not found" << std::endl;
                continue;
            }

            for (auto elem : nodes) {
                print_mutex.lock();
                std::cout << elem;
                print_mutex.unlock();
            }
        } else if (s == "exec") {
            bool ok = true;

            int id, n;
            std::cin >> id >> n;

            auto i = nodes.begin();
            for (; i != nodes.end(); ++i)
                if (i->id == id)
                    break;
            if (i == nodes.end()) {
                std::cout << "Error: Node not found" << std::endl;
                ok = false;
            }

            std::stringstream ss;
            ss << "exec " << id << " " << n << " ";
            for (int i = 0; i < n; ++i) {
                int a;
                std::cin >> a;
                ss << a << " ";
            }

            if (!ok) continue;

            threads.emplace_back(execThread, ss.str(), std::ref(pub), std::ref(sub), id);
        } else if (s == "heartbit") {
            uint time;
            std::cin >> time;
            std::chrono::milliseconds time_ms(time);

            for (auto &elem : threadsHeartbit)
                elem.detach();
            threadsHeartbit.clear();

            std::stringstream ss;
            ss << "heartbit " << time;
            sendMsg(ss.str(), pub);
            std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(100)));

            threads.emplace_back(heartbitRecv, std::ref(subHeartbit), std::ref(heartbits), time);
            threads.emplace_back(heartbitCheck, std::ref(heartbits), time_ms, std::ref(nodes), std::ref(idMainCalcNode), std::ref(pub), std::ref(sub), std::ref(threads));
        } else if (s == "exit") {
            for (auto elem : nodes) {
                kill_process(elem.pid);
            }
            break;
        } else if (s == "look") {
            std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(10)));
        } else {
            std::cout << "Error: Unknown command" << std::endl;
        }
    }
	
	return 0;
}