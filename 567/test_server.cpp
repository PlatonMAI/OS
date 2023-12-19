// 3 Вариант
// 1 Топология
// 1 Тип команд
// 3 Тип проверки доступности узлов

#include <iostream>
#include <zmq_addon.hpp>

int main() {
    zmq::context_t ctx;

    zmq::socket_t sock1(ctx, zmq::socket_type::push);
    zmq::socket_t sock2(ctx, zmq::socket_type::push);
    sock1.connect("tcp://127.0.0.1:4041");
    sock2.connect("tcp://127.0.0.1:4041");

    std::cout << "Пытаюсь отправить" << std::endl;
    auto res = sock1.send(zmq::str_buffer("hello епта1"), zmq::send_flags::none);
    if (!res)
        std::cout << "Подожди, тут вообще песня..." << std::endl;
    
    res = sock2.send(zmq::str_buffer("hello епта2"), zmq::send_flags::none);
    if (!res)
        std::cout << "Подожди, тут вообще песня..." << std::endl;

    std::cout << "dead..." << std::endl;

    return 0;
}