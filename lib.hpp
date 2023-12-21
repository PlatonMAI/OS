#pragma once

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <algorithm>

#include "sys/mman.h"
#include "semaphore.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/types.h"
#include "string.h"

using namespace std;

static inline const int MAX_LENGTH = 1024;
static inline const int MAX_LENGTH_ = 128;

enum SERVER_RESPONSE {
    LOGIN_DONE,
    LOGIN_EXISTS,
    SEND_UNKNOWN_COMMAND,
    SEND_UNKNOWN_SUBCOMMAND,
    SEND_USER_DONE,
    SEND_USER_NOT_EXISTS,
    CREATE_GROUP_DONE,
    CREATE_GROUP_EXISTS,
    JOIN_GROUP_DONE,
    JOIN_GROUP_JOINED,
    JOIN_GROUP_NOT_EXISTS,
    SEND_GROUP_DONE,
    SEND_GROUP_NOT_EXISTS,
};

static inline map<SERVER_RESPONSE, string> PARSE_RESPONSE = {
    {LOGIN_DONE, "Успешное задание логина!"},
    {LOGIN_EXISTS, "Ошибка: Логин уже занят!"},
    {SEND_UNKNOWN_COMMAND, "Ошибка: Неизвестная команда!"},
    {SEND_UNKNOWN_SUBCOMMAND, "Ошибка: Неизвестная подкоманда!"},
    {SEND_USER_DONE, "Успешная отправка сообщения!"},
    {SEND_USER_NOT_EXISTS, "Ошибка: Такого пользователя не существует!"},
    {CREATE_GROUP_DONE, "Успешное создание группы!"},
    {CREATE_GROUP_EXISTS, "Ошибка: Такая группа уже существует!"},
    {JOIN_GROUP_DONE, "Успешное присоединение к группе!"},
    {JOIN_GROUP_JOINED, "Ошибка: Вы уже присоединились к этой группе!"},
    {JOIN_GROUP_NOT_EXISTS, "Ошибка: Такой группы не существует!"},
    {SEND_GROUP_DONE, "Успешная отправка сообщения в группу!"},
    {SEND_GROUP_NOT_EXISTS, "Ошибка: Такой группы не существует!"},
};

string itos(int x);

struct Client {
    char* mmapped_file_pointer;
    sem_t* semaphore_server;
    sem_t* semaphore_client;
};

int open_mmapped_file(const char* mmapped_file_name);
char* open_mmap(int mmapped_file_descriptor);

sem_t* create_semaphore(const char* semaphore_name, int value);
sem_t* open_semaphore(const char* semaphore_name);

void sem_post(sem_t* semaphore, chrono::milliseconds time);

string get_date();

class Message {
private:
    string login_sender;
    string date;
    char msg[MAX_LENGTH];

    bool inGroup = false;
    string group_name;

public:
    Message();
    Message(string _login_sender, char* _msg);
    Message(string _login_sender, char* _msg, string group_name);
    string to_string();
};