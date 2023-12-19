#include <iostream>
#include <zmq.hpp>
#include <thread>

#include "lib.hpp"

using namespace zmq;

const int MAX_LEN_MSG = 256;

void waitResponse(socket_t &pubPrev, socket_t &subResponse) {
	std::cout << "Жду ответа" << "\n";
    message_t response(MAX_LEN_MSG);
    auto res_r = subResponse.recv(response, recv_flags::none);
    if (!res_r.has_value()) {
        // std::cout << "Пиздец при получении" << std::endl;
        return;
    }

	std::cout << "Ответ получен, отправляю" << "\n";

    auto res_s = pubPrev.send(response, send_flags::none);
    if (!res_s.has_value()) {
        // std::cout << "Пиздец при получении" << std::endl;
        return;
    }
}

void bindPubPrev(socket_t &pub, int id) {
	pub.bind(getAddrPrev(id));
}
void bindPubNext(socket_t &pub, int id) {
	pub.bind(getAddrNext(id));
}

void bindPubs(socket_t &pubPrev, socket_t &pubNext, int idPrev, int idNext) {
	bindPubPrev(pubPrev, idPrev);
	if (idNext != -2)
		bindPubNext(pubNext, idNext);
}

void rebindPubPrev(socket_t &pub, int idOld, int id) {
	std::cout << "Сейчас буду перебиндить предыдущего" << "\n";
	pub.unbind(getAddrPrev(idOld));
	bindPubPrev(std::ref(pub), id);

	std::cout << "Перебиндил предыдущего на " << getAddrPrev(id) << "\n" << "\n";
}
void rebindPubNext(socket_t &pub, int idOld, int id) {
	std::cout << "Сейчас буду перебиндить следующего с " << idOld << " на " << id << "\n";
	if (idOld != -2)
		pub.unbind(getAddrNext(idOld));
	bindPubNext(std::ref(pub), id);
	
	std::cout << "Перебиндил следующего на " << getAddrNext(id) << "\n" << "\n";
}

void heartbit(socket_t &pub, std::chrono::milliseconds time, int id) {
	srand(id);
	while (true) {
		if (rand() % 11 == 0) {
			std::cout << "Я узел " << id << " больше не стучу сука" << std::endl;
			break;
		}

		std::cout << "Я узел " << id << " стучу" << std::endl;

		auto res_s = pub.send(buffer( itos(id) ), send_flags::none);
		if (!res_s.has_value()) {
			// std::cout << "Пиздец при отправке" << std::endl;
			break;
		}

		std::this_thread::sleep_for(std::chrono::duration(time));
	}
}

int main(int argc, char* argv[]) {
	assert(argc == 4);

	int idPrev = std::stoi(argv[1]);
	int id = std::stoi(argv[2]);
	int idNext = std::stoi(argv[3]);

	// std::cout << "Я запустился с " << idPrev << " " << id << " " << idNext << std::endl;

	context_t ctx;
    socket_t pubPrev(ctx, socket_type::push);
    socket_t pubNext(ctx, socket_type::push);
    socket_t subRequest(ctx, socket_type::pull);
    socket_t subResponse(ctx, socket_type::pull);

	bindPubs(std::ref(pubPrev), std::ref(pubNext), idPrev, idNext);
	subRequest.connect(getAddr( getPort(id) ));
	subResponse.connect(getAddr( getPort(id) + 1 ));

	std::vector<std::thread> threads;

	socket_t pubHeartbit(ctx, socket_type::push);
	pubHeartbit.connect(getAddrNext(-1));
	std::optional<std::thread> threadHeartbit;

	while (true) {
		message_t request(MAX_LEN_MSG);
		auto res_r = subRequest.recv(request, recv_flags::none);
		if (!res_r.has_value()) {
			// std::cout << "Пиздец при получении" << std::endl;
			break;
		}

		std::cout << "\n" << "Я узел " << id << ", получил сообщение: ";
		std::cout << request.to_string() << "\n";

		std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(1000)));

		std::stringstream ss(request.to_string());

		std::string mode;
		ss >> mode;

		if (mode == "exec") {
			int idTarget, n;
			ss >> idTarget >> n;
			
			if (id == idTarget) {
				std::cout << "Я целевой узел, значит отправляю результат обратно" << "\n";

				std::string res = "";
				int sum_ = 0;
				int x;
				for (int i = 0; i < n; ++i) {
					ss >> x;
					sum_ += x;
				}

				res = itos(sum_);

				mutable_buffer mbuf = buffer(res);
				auto res_s = pubPrev.send(mbuf, send_flags::none);
				if (!res_s.has_value()) {
					// std::cout << "Пиздец при отправке" << std::endl;
					break;
				}
			} else {
				std::cout << "Я не целевой узел, значит отправляю дальше" << "\n";

				auto res_s = pubNext.send(request, send_flags::none);
				if (!res_s.has_value()) {
					// std::cout << "Пиздец при отправке" << std::endl;
					break;
				}

				threads.emplace_back(waitResponse, std::ref(pubPrev), std::ref(subResponse));
			}
		} else if (mode == "rebind") {
			int idTarget;
			ss >> idTarget;

			if (id == idTarget) {
				std::cout << "Я целевой узел, значит буду перерибиндить" << "\n";

				std::string submode;
				ss >> submode;
				
				int idNewPrev, idNewNext;
				if (submode == "prev" || submode == "all") {
					ss >> idNewPrev;
					rebindPubPrev(std::ref(pubPrev), idPrev, idNewPrev);
					idPrev = idNewPrev;
				}
				if (submode == "next" || submode == "all") {
					ss >> idNewNext;
					rebindPubNext(std::ref(pubNext), idNext, idNewNext);
					idNext = idNewNext;
				}
			} else {
				std::cout << "Я не целевой узел, значит отправляю дальше: " << idTarget << "\n";

				auto res_s = pubNext.send(request, send_flags::none);
				if (!res_s.has_value()) {
					// std::cout << "Пиздец при отправке" << std::endl;
					break;
				}
			}
		} else if (mode == "heartbit") {
			if (threadHeartbit) {
				threadHeartbit->detach();
			}

			uint time;
			ss >> time;
			std::chrono::milliseconds time_ms(time);

			if (idNext != -2) {
				auto res_s = pubNext.send(request, send_flags::none);
				if (!res_s.has_value()) {
					// std::cout << "Пиздец при получении" << std::endl;
					break;
				}
			}

			threadHeartbit = std::thread(heartbit, std::ref(pubHeartbit), time_ms, id);
		} else {
			std::cout << "Пиздец не знаю операции" << std::endl;
			break;
		}
	}
	
	return 0;
}