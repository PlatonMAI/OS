#include "lib.hpp"

string itos(int x) {
    stringstream ss;
    return (stringstream() << x).str();
}

int open_mmapped_file(const char* mmapped_file_name) {
    int mmapped_file_descriptor = shm_open(mmapped_file_name, O_RDWR | O_CREAT, 0777);
    ftruncate(mmapped_file_descriptor, MAX_LENGTH);
    return mmapped_file_descriptor;
}
char* open_mmap(int mmapped_file_descriptor) {
    return (char*)mmap(NULL, MAX_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, mmapped_file_descriptor, 0);
}

sem_t* create_semaphore(const char* semaphore_name, int value = 0) {
    return sem_open(semaphore_name, O_CREAT, 0777, value);
}
sem_t* open_semaphore(const char* semaphore_name) {
    return sem_open(semaphore_name, 0);
}

void sem_post(sem_t* semaphore, chrono::milliseconds time) {
    this_thread::sleep_for(time);

    sem_post(semaphore);
}

string get_date() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    stringstream ss;
    ss << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << " " << now->tm_mday << "." << now->tm_mon + 1 << "." << now->tm_year + 1900;
    return ss.str();
}

Message::Message() {}
Message::Message(string _login_sender, char* _msg) : login_sender(_login_sender) {
    strcpy(msg, _msg);
    date = get_date();
}
Message::Message(string _login_sender, char* _msg, string _group_name) : Message(_login_sender, _msg) {
    group_name = _group_name;
    inGroup = true;
}

string Message::to_string() {
    stringstream ss;
    ss << "from: " << login_sender << ", " << date << "\n";
    if (inGroup)
        ss << "in: " << group_name << "\n";
    ss << msg << "\n";
    return ss.str();
}