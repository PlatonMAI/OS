#include "lib.hpp"

void receiving(char* mmapped_file_pointer, sem_t* semaphore_server, sem_t* semaphore_client) {
    // cout << "Я поток получений сообщений" << endl;

    while (true) {
        // cout << "Жду уведомления о получении сообщения" << endl;
        sem_wait(semaphore_client);

        cout << "Сообщение получено:" << endl;
        cout << mmapped_file_pointer;
    }
}

void sending(char* mmapped_file_pointer, sem_t* semaphore_server, sem_t* semaphore_client) {
    // cout << "Я поток отправки сообщений" << endl;
    cout << "Список доступных команд:\n"
    << "$ send user login message - отправить пользователю с логином login сообщение message\n"
    << "$ create name - создать группу с именем name (после создания вы автоматически в нее вступите)\n"
    << "$ join name - отправить пользователю с логином login сообщение message\n"
    << "$ send group name message - отправить в группу с именем name сообщение message\n"
    << endl;
    cin.clear();
    cin.ignore(numeric_limits<std::streamsize>::max(), '\n');

    while (true) {
        cout << "Введите команду:" << endl;
        string request;
        getline(cin, request);

        // cout << "Помещаю команду в общую память" << endl;
        strcpy(mmapped_file_pointer, request.c_str());
        
        // cout << "Толкаю сервер" << endl;
        sem_post(semaphore_server);
        // cout << "Жду обработки сообщения" << endl;
        sem_wait(semaphore_client);

        // cout << "Дождался: " << mmapped_file_pointer << endl;
        int response = stoi(mmapped_file_pointer);

        cout << PARSE_RESPONSE[SERVER_RESPONSE(response)] << endl;
    }
}

int main() {
    // cout << PARSE_RESPONSE[SERVER_RESPONSE(0)] << endl;

    const char* mmapped_file_name = "mmapped_file";
    const char* semaphore_name_server = "/semaphoreServer";
    const char* semaphore_name_client = "/semaphoreClient";

    int mmapped_file_descriptor = open_mmapped_file(mmapped_file_name);
    char* mmapped_file_pointer = open_mmap(mmapped_file_descriptor);

    sem_t* semaphore_server = open_semaphore(semaphore_name_server);
    sem_t* semaphore_client = open_semaphore(semaphore_name_client);

    // cout << "Жду пока сервер толкнет" << endl;
    sem_wait(semaphore_client);

    // cout << "Толкнули, читаю временный семафор" << endl;
    char semaphore_name_temp[MAX_LENGTH];
    strcpy(semaphore_name_temp, mmapped_file_pointer);
    sem_t* semaphore_temp = open_semaphore(semaphore_name_temp);

    bool ok = false;
    while (true) {
        cout << "Введите логин: " << endl;
        string login;
        cin >> login;

        // cout << "Записываю логин в общую память" << endl;
        strcpy(mmapped_file_pointer, login.c_str());

        // cout << "Толкаю серверный семафор" << endl;
        sem_post(semaphore_server);
        // cout << "И жду временный" << endl;
        sem_wait(semaphore_temp);

        // cout << "Дождался, сообщение от сервера: " << mmapped_file_pointer << endl;
        int response = stoi(mmapped_file_pointer);
        cout << PARSE_RESPONSE[SERVER_RESPONSE(response)] << endl;
        ok = response == SERVER_RESPONSE::LOGIN_DONE;

        if (ok) break;
    }

    // cout << "Толкаю серверный семафор" << endl;
    sem_post(semaphore_server);
    // cout << "И жду временный" << endl;
    sem_wait(semaphore_temp);

    // cout << "Толкнули, читаю сообщение:" << endl;
    // cout << mmapped_file_pointer << endl;
    stringstream ss(mmapped_file_pointer);

    char mmapped_file_nameSend[MAX_LENGTH_];
    char mmapped_file_nameRecv[MAX_LENGTH_];

    char semaphore_server_nameSend[MAX_LENGTH_];
    char semaphore_server_nameRecv[MAX_LENGTH_];
    char semaphore_client_nameSend[MAX_LENGTH_];
    char semaphore_client_nameRecv[MAX_LENGTH_];

    ss >> mmapped_file_nameSend >> mmapped_file_nameRecv >> semaphore_server_nameSend >> semaphore_server_nameRecv >> semaphore_client_nameSend >> semaphore_client_nameRecv;

    // cout << "Сообщение прочитано, толкаю сервер" << endl;
    sem_post(semaphore_server);
    sem_close(semaphore_temp);

    // cout << "Создаю все mmap и семафоры" << endl;
    int mmapped_file_descriptorSend = open_mmapped_file(mmapped_file_nameSend);
    char* mmapped_file_pointerSend = open_mmap(mmapped_file_descriptorSend);
    int mmapped_file_descriptorRecv = open_mmapped_file(mmapped_file_nameRecv);
    char* mmapped_file_pointerRecv = open_mmap(mmapped_file_descriptorRecv);

    sem_t* semaphore_serverSend = open_semaphore(semaphore_server_nameSend);
    sem_t* semaphore_serverRecv = open_semaphore(semaphore_server_nameRecv);
    sem_t* semaphore_clientSend = open_semaphore(semaphore_client_nameSend);
    sem_t* semaphore_clientRecv = open_semaphore(semaphore_client_nameRecv);

    // cout << "Запускаю потоки отправки и получения сообщений" << endl;
    thread send_thread(sending, mmapped_file_pointerSend, semaphore_serverSend, semaphore_clientSend);
    thread recv_thread(receiving, mmapped_file_pointerRecv, semaphore_serverRecv, semaphore_clientRecv);

    send_thread.join();
    recv_thread.join();
}