#include "server.hpp"

map<string, Client> clients;
map<string, vector<string>> groups;

void send_message(string login_sender, string login_receiver, char* message, bool inGroup = false, string group_name = "") {
    cout << "Вызвана функция отправки сообщения" << endl;

    cout << "Формирую сообщение..." << endl;
    Message msg;
    if (inGroup)
        msg = Message(login_sender, message, group_name);
    else
        msg = Message(login_sender, message);
    cout << "Вот такое:\n" << msg.to_string() << endl;

    cout << "Записываю сообщение в общую память нужного клиента" << endl;
    strcpy(clients[login_receiver].mmapped_file_pointer, msg.to_string().c_str());

    cout << "Толкаю клиентский семафор" << endl;
    sem_post(clients[login_receiver].semaphore_client, 1000ms);

    cout << "Завершаюсь" << endl;
}

void send_message_group(string login_sender, string group_name, char* message) {
    cout << "Вызвана функция отправки сообщения группе" << endl;

    for (string login_receiver : groups[group_name]) {
        if (login_sender == login_receiver) {
            cout << "Сам себе не отправляю" << endl;
            continue;
        }

        cout << "Отправляю пользователю " << login_receiver << endl;
        send_message(login_sender, login_receiver, message, true, group_name);
    }
}

void user_thread(string data) {
    stringstream ss(data);

    char mmapped_file_nameSend[MAX_LENGTH_];
    char mmapped_file_nameRecv[MAX_LENGTH_];

    char semaphore_server_nameSend[MAX_LENGTH_];
    char semaphore_server_nameRecv[MAX_LENGTH_];
    char semaphore_client_nameSend[MAX_LENGTH_];
    char semaphore_client_nameRecv[MAX_LENGTH_];

    string login;

    ss >> mmapped_file_nameSend >> mmapped_file_nameRecv >> semaphore_server_nameSend >> semaphore_server_nameRecv >> semaphore_client_nameSend >> semaphore_client_nameRecv >> login;

    int mmapped_file_descriptorSend = open_mmapped_file(mmapped_file_nameSend);
    char* mmapped_file_pointerSend = open_mmap(mmapped_file_descriptorSend);
    int mmapped_file_descriptorRecv = open_mmapped_file(mmapped_file_nameRecv);
    char* mmapped_file_pointerRecv = open_mmap(mmapped_file_descriptorRecv);

    sem_t* semaphore_serverSend = create_semaphore(semaphore_server_nameSend, 0);
    sem_t* semaphore_serverRecv = create_semaphore(semaphore_server_nameRecv, 0);
    sem_t* semaphore_clientSend = create_semaphore(semaphore_client_nameSend, 0);
    sem_t* semaphore_clientRecv = create_semaphore(semaphore_client_nameRecv, 0);

    cout << login << " " << "Создали клиента в базе" << endl;
    Client client{mmapped_file_pointerRecv, semaphore_serverRecv, semaphore_clientRecv};
    clients[login] = client;

    while (true) {
        cout << login << " " << "Жду команды от клиента" << endl;
        sem_wait(semaphore_serverSend);

        cout << login << " " << "Получил команду:" << endl;
        cout << mmapped_file_pointerSend << endl;
        
        cout << login << " " << "Парсинг..." << endl;
        stringstream ss(mmapped_file_pointerSend);
        mmapped_file_pointerSend[0] = 0;
        string command;
        ss >> command;
        cout << login << " " << "Получена команда " << command << endl;

        if (command == "create") {
            cout << login << " " << "Хотим создать группу" << endl;

            string group_name;
            ss >> group_name;
            cout << login << " " << "С таким именем: " << group_name << endl;

            if (groups.find(group_name) != groups.end()) {
                cout << login << " " << "Такая группа уже существует!" << endl;
                strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::CREATE_GROUP_EXISTS).c_str());
            } else {
                cout << login << " " << "Создаю группу" << endl;
                groups[group_name].push_back(login);
                strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::CREATE_GROUP_DONE).c_str());
            }
        } else if (command == "join") {
            cout << login << " " << "Хотим зайти в группу" << endl;

            string group_name;
            ss >> group_name;
            cout << login << " " << "С таким именем: " << group_name << endl;

            if (groups.find(group_name) != groups.end()) {
                cout << login << " " << "Группа существует" << endl;

                if ( find(groups[group_name].begin(), groups[group_name].end(), login) != groups[group_name].end() ) {
                    cout << login << " " << "Пользователь уже есть в этой группе" << endl;
                    strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::JOIN_GROUP_JOINED).c_str());
                } else {
                    cout << login << " " << "Пользователя нет, добавляю" << endl;
                    groups[group_name].push_back(login);
                    strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::JOIN_GROUP_DONE).c_str());
                }
            } else {
                cout << login << " " << "Такой группы не существует" << endl;
                strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::JOIN_GROUP_NOT_EXISTS).c_str());
            }
        } else if (command == "send") {
            cout << login << " " << "Хотим отправить" << endl;

            string subcommand;
            ss >> subcommand;
            cout << login << " " << "Получена подкоманда " << subcommand << endl;
            if (subcommand == "user") {
                cout << login << " " << "Отправляем пользователю" << endl;

                string login_user;
                ss >> login_user;
                cout << login << " " << "Требуемый логин: " << login_user << endl;

                if (clients.find(login_user) != clients.end()) {
                    cout << login << " " << "Такой пользователь существует, отправляю..." << endl;
                    int shift = command.size() + 1 + subcommand.size() + 1 + login_user.size() + 1;
                    send_message(login, login_user, mmapped_file_pointerSend + shift);
                    cout << login << " " << "Вот что: " << mmapped_file_pointerSend + shift << endl;
                    strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::SEND_USER_DONE).c_str());
                } else {
                    cout << login << " " << "Такого пользователя не существует!" << endl;
                    strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::SEND_USER_NOT_EXISTS).c_str());
                }
            } else if (subcommand == "group") {
                cout << login << " " << "Отправляем в группу" << endl;

                string group_name;
                ss >> group_name;
                cout << login << " " << "Требуемая группа: " << group_name << endl;

                if (groups.find(group_name) != groups.end()) {
                    cout << login << " " << "Группа существует, отправляю..." << endl;
                    int shift = command.size() + 1 + subcommand.size() + 1 + group_name.size() + 1;
                    send_message_group(login, group_name, mmapped_file_pointerSend + shift);
                    cout << login << " " << "Вот что: " << mmapped_file_pointerSend + shift << endl;
                    strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::SEND_GROUP_DONE).c_str());
                } else {
                    cout << login << " " << "Такой группы не существует" << endl;
                    strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::SEND_GROUP_NOT_EXISTS).c_str());
                }
            } else {
                cout << login << " " << "Неизвестная подкоманда!" << endl;
                strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::SEND_UNKNOWN_SUBCOMMAND).c_str());
            }
        } else {
            cout << login << " " << "Неизвестная команда!" << endl;
            strcpy(mmapped_file_pointerSend, itos(SERVER_RESPONSE::SEND_UNKNOWN_COMMAND).c_str());
        }

        cout << login << " " << "Разблокирую клиента" << endl;
        sem_post(semaphore_clientSend, 1000ms);
    }
}

int main() {
    // cout << PARSE_RESPONSE[SERVER_RESPONSE(0)] << endl;

    const char* mmapped_file_name = "mmapped_file";
    const char* semaphore_name_server = "/semaphoreServer";
    const char* semaphore_name_client = "/semaphoreClient";
    const char* semaphore_name_temp = "/semaphoreTemp";

    shm_unlink(mmapped_file_name);
    sem_unlink(semaphore_name_server);
    sem_unlink(semaphore_name_client);
    sem_unlink(semaphore_name_temp);

    int mmapped_file_descriptor = open_mmapped_file(mmapped_file_name);
    char* mmapped_file_pointer = open_mmap(mmapped_file_descriptor);

    sem_t* semaphore_server = create_semaphore(semaphore_name_server, 0);
    sem_t* semaphore_client = create_semaphore(semaphore_name_client, 0);
    sem_t* semaphore_temp = create_semaphore(semaphore_name_temp, 0);

    vector<thread> user_threads;

    char string[MAX_LENGTH];
    while (1) {
        cout << "Скопировал имя временного семафора в общую память" << endl;
        strcpy(mmapped_file_pointer, semaphore_name_temp);

        // std::this_thread::sleep_for(5000ms);

        cout << "Толкаю рандомного клиента" << endl;
        sem_post(semaphore_client, 1000ms);
        cout << "Жду пока откликнуться" << endl;
        sem_wait(semaphore_server);

        bool ok = false;
        char login[MAX_LENGTH];
        while (true) {
            cout << "Откликнулись, читаю логин" << endl;
            strcpy(login, mmapped_file_pointer);
            cout << "Вот логин: " << login << endl;

            cout << "Проверяю, не занят ли он" << endl;
            if (clients.find(login) == clients.end()) {
                cout << "Не занят, сообщаю об успехе" << endl;
                strcpy(mmapped_file_pointer, itos(SERVER_RESPONSE::LOGIN_DONE).c_str());
                ok = true;
            } else {
                cout << "Занят, сообщаю об этом" << endl;
                strcpy(mmapped_file_pointer, itos(SERVER_RESPONSE::LOGIN_EXISTS).c_str());
            }

            cout << "Толкаю временный семафор" << endl;
            sem_post(semaphore_temp, 1000ms);
            cout << "Жду реакции" << endl;
            sem_wait(semaphore_server);

            if (ok) break;
        }

        cout << "Отправляю уникальные каналы для связи" << endl;
        stringstream ss;
        ss << "mmapped_fileSend" << login << " " << "mmapped_fileRecv" << login << " " << "/semaphoreServerSend" << login << " " << "/semaphoreServerRecv" << login << " " << "/semaphoreClientSend" << login << " " << "/semaphoreClientRecv" << login;
        strcpy(mmapped_file_pointer, ss.str().c_str());
        cout << "Отправил такое сообщение:\n" << mmapped_file_pointer << endl;
        
        cout << "Создаю поток для этого пользователя" << endl;
        ss << " " << login;
        user_threads.emplace_back(user_thread, ss.str());

        cout << "Толкаю временный семафор" << endl;
        sem_post(semaphore_temp, 1000ms);
        cout << "Жду, пока разберут данные, чтобы не перетереть их потом" << endl;
        sem_wait(semaphore_server);
    }

    munmap(mmapped_file_pointer, 0);
    shm_unlink(mmapped_file_name);

    sem_close(semaphore_server);
    sem_close(semaphore_client);
    sem_close(semaphore_temp);
    sem_unlink(semaphore_name_server);
    sem_unlink(semaphore_name_client);
    sem_unlink(semaphore_name_temp);
}