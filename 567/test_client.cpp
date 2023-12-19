#include <iostream>
#include <zmq_addon.hpp>

int main() {
    zmq::context_t ctx;

    zmq::socket_t sock1(ctx, zmq::socket_type::pull);
    sock1.bind("tcp://127.0.0.1:4041");

    for (int i = 0; i < 2; ++i) {
        std::cout << "Пытаюсь получить" << std::endl;
        zmq::message_t m(1024);
        auto res = sock1.recv(m, zmq::recv_flags::none);
        if (!res)
            std::cout << "Ха-ха" << std::endl;

        std::cout << m.to_string() << std::endl;
    }

    std::cout << "dead..." << std::endl;

    return 0;
}