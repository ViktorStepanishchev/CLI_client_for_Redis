#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <string>
#include <string.h>
#include <ctype.h>
#include <typeinfo>
#include <openssl/ssl.h>
#include <openssl/err.h>
using namespace std;

#define BUFFER_SIZE 1024
#define REDIS_SERVER "127.0.0.1"
#define REDIS_PORT 6379

std::vector<std::string> break_text_to_words(const std::string& msg) {
    std::vector<std::string> words;
    std::istringstream potok(msg);
    std::string word;
    while (potok >> word) {
        words.push_back(word);
    }
    return words;
}

bool isNumber(const std::string& s)
{
    std::stringstream ss(s);
    float num;
    ss >> num;

    if(ss.fail() || !ss.eof()) {
        return false;
    }
    return true;
}

std::string create_RESP_command_for_hget(std::vector<std::string>&words_msg){
    std::stringstream resp;
    resp << "*" << words_msg.size() << "\r\n";
    resp << "$" << words_msg[0].size() << "\r\n";
    resp << words_msg[0] << "\r\n";
    resp << "$" << words_msg[1].size() << "\r\n";
    resp << words_msg[1] << "\r\n";
    resp.str();
    for (int i = 2; i < words_msg.size(); i++){
        if (i % 2 == 0){
            resp << "$" << words_msg[i].size() << "\r\n";
            resp << words_msg[i] << "\r\n";
            resp.str();
        }
        else{
            int s;
            if (isNumber(words_msg[i])){
                s = std::stoi(words_msg[i]);
                resp << "$" << words_msg[i].size() << "\r\n";
                resp << s << "\r\n";
                resp.str();
            }
            else{
                resp << "$" << words_msg[i].size() << "\r\n";
                resp << words_msg[i] << "\r\n";
                resp.str();
            }
        }
    }
    return resp.str();
}

std::string create_RESP_command(const std::string& command, const std::string& parametr, const std::string& value, const std::string& sec_value, int att) {
    if (att == 4){
        if (command == "LREM"){
            int v;
            v = std::stoi(value);
            std::stringstream resp;

            resp << "*" << att << "\r\n";
            resp << "$" << command.size() << "\r\n";
            resp << command << "\r\n";
            resp << "$" << parametr.size() << "\r\n";
            resp << parametr << "\r\n";
            resp << "$" << value.size() << "\r\n";
            resp << v << "\r\n";
            resp << "$" << sec_value.size() << "\r\n";
            resp << sec_value << "\r\n";
            return resp.str();
        }
        int v, s_v;
        v = std::stoi(value);
        s_v = std::stoi(sec_value);
        std::stringstream resp;
        resp << "*" << att << "\r\n";
        resp << "$" << command.size() << "\r\n";
        resp << command << "\r\n";
        resp << "$" << parametr.size() << "\r\n";
        resp << parametr << "\r\n";
        resp << "$" << value.size() << "\r\n";
        resp << v << "\r\n";
        resp << "$" << sec_value.size() << "\r\n";
        resp << s_v << "\r\n";
        return resp.str();
    }
    if (att == 3){
        if (command == "LINDEX"){
            int v;
            v = std::stoi(value);
            std::stringstream resp;

            resp << "*" << att << "\r\n";
            resp << "$" << command.size() << "\r\n";
            resp << command << "\r\n";
            resp << "$" << parametr.size() << "\r\n";
            resp << parametr << "\r\n";
            resp << "$" << value.size() << "\r\n";
            resp << v << "\r\n";
            return resp.str();
        }
        std::stringstream resp;
        resp << "*" << att << "\r\n";
        resp << "$" << command.size() << "\r\n";
        resp << command << "\r\n";
        resp << "$" << parametr.size() << "\r\n";
        resp << parametr << "\r\n";
        resp << "$" << value.size() << "\r\n";
        resp << value << "\r\n";
        return resp.str();
    }
    if (att == 2){
        std::stringstream resp;
        resp << "*" << att << "\r\n";
        resp << "$" << command.size() << "\r\n";
        resp << command << "\r\n";
        resp << "$" << parametr.size() << "\r\n";
        resp << parametr << "\r\n";
        return resp.str();
    }
    if (att == 1){
        std::stringstream resp;
        resp << "*" << att << "\r\n";
        resp << "$" << command.size() << "\r\n";
        resp << command << "\r\n";
        return resp.str();
    }
}

std::string commands_with_any_attribute(std::string msg, int sock, char *buffer){
    std::vector<std::string>words_msg = break_text_to_words(msg);
    if (words_msg[0] == "SET"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "GET"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "DEL"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "TYPE"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "ECHO"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "KEYS"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "UNLINK"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "EXPIRE"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "RENAME"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "LPUSH"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "RPUSH"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "LRANGE"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 4);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "RRANGE"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 4);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "LPOP"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "RPOP"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "LLEN"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "LREM"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 4);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "LLEN"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "LINDEX"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "LSET"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 4);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "HEXISTS"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "HSET"){
        std::string command = create_RESP_command_for_hget(words_msg);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "HGET"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "HDEL"){
        std::string command = create_RESP_command_for_hget(words_msg);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "HGETALL"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "HKEYS"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "HLEN"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "HMSET"){
        std::string command = create_RESP_command_for_hget(words_msg);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "HVALS"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    return "Error command!\n";
}

std::string command_no_space(std::string msg, int sock, char* buffer){
    if (msg == "ping"){
        std::string command = create_RESP_command(msg, "", "", "", 1);;
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (msg == "info"){
        std::string command = create_RESP_command(msg, "", "", "", 1);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (msg == "flushall"){
        std::string command = create_RESP_command(msg, "", "", "", 1);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    return "Error command!\n";
}

bool quit(std::string msg){
    if (msg == "quit"){
        return true;
    }
}

int main() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
    SSL *ssl = SSL_new(ctx);

    int server = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in sa_serv;
    memset(&sa_serv, '\0', sizeof(sa_serv));

    sa_serv.sin_family = AF_INET;
    sa_serv.sin_port = htons(REDIS_PORT);
    sa_serv.sin_addr.s_addr = inet_addr(REDIS_SERVER);

    connect(server, (struct sockaddr*) &sa_serv, sizeof(sa_serv));

    SSL_set_fd(ssl, server);

    if (SSL_connect(ssl) == -1) {
        ERR_print_errors_fp(stderr);
    } else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        // Здесь вы можете написать код для отправки и получения данных от сервера Redis.
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(server);
    SSL_CTX_free(ctx);
    EVP_cleanup();

    return 0;
}


//while (true) {
//        cout << SERVER_IP << ":" << DEFAULT_PORT << ">";
//        getline(cin, message);
//        if (quit(message)) {
//            command = "*1\\r\\n$4\\r\\nQUIT\\r\\n";
//            send(sock, command.c_str(), command.size(), 0);
//            break;
//        }
//        if (message.find(' ') > 0 && message.find(' ') != 18446744073709551615) {
//            cout << commands_with_any_attribute(message, sock, buffer);
//        }
//        if (message.find(' ') == 18446744073709551615) {
//            cout << command_no_space(message, sock, buffer);
//        }
//    }