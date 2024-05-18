#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string.h>
#include <ctype.h>
#include <typeinfo>
#include <cstring>

using namespace std;

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 6379
#define BUFFER_SIZE 4096

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
    if (msg == "KEYS *"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        return buffer;
    }
    if (words_msg[0] == "SET"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        std::vector res = break_text_to_words(buffer);
        return res[0];
    }
    if (words_msg[0] == "GET"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        std::vector res = break_text_to_words(buffer);
        if (res.size() == 1){
            return res[0];
        }
        return res[1];
    }
    if (words_msg[0] == "DEL"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        std::vector res = break_text_to_words(buffer);
        return res[0];
    }
    if (words_msg[0] == "TYPE"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        std::vector res = break_text_to_words(buffer);
        return res[0];
    }
    if (words_msg[0] == "ECHO"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        std::vector res = break_text_to_words(buffer);
        return res[1];
    }
    if (words_msg[0] == "KEYS"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        std::vector res = break_text_to_words(buffer);
        return res[2];
    }
    if (words_msg[0] == "UNLINK"){
        std::string command = create_RESP_command_for_hget(words_msg);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        std::vector res = break_text_to_words(buffer);
        return res[0];
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
        std::string command = create_RESP_command_for_hget(words_msg);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        std::vector res = break_text_to_words(buffer);
        return res[0];
    }
    if (words_msg[0] == "RPUSH"){
        std::string command = create_RESP_command_for_hget(words_msg);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        std::vector res = break_text_to_words(buffer);
        return res[0];
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
        std::string command = create_RESP_command(msg, "", "", "", 1);
        send(sock, command.c_str(), command.size(), 0);
        read(sock, buffer, BUFFER_SIZE);
        std::vector res = break_text_to_words(buffer);
        return res[0];
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
        std::vector res = break_text_to_words(buffer);
        return res[0];
    }
    return "Error command!\n";
}

std::string commands_with_any_attribute_SSL(std::string msg, int sock, char *buffer, SSL* ssl){
    std::vector<std::string>words_msg = break_text_to_words(msg);
    if (msg == "KEYS *"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "SET"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::vector res = break_text_to_words(buffer);
            return res[0];
        }
    }
    if (words_msg[0] == "GET"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::vector res = break_text_to_words(buffer);
            if (res.size() == 1) {
                return res[0];
            }
            return res[1];
        }
    }
    if (words_msg[0] == "DEL"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::vector res = break_text_to_words(buffer);
            return res[0];
        }
    }
    if (words_msg[0] == "TYPE"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::vector res = break_text_to_words(buffer);
            return res[0];
        }
    }
    if (words_msg[0] == "ECHO") {
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::vector res = break_text_to_words(buffer);
            return res[1];
        }
    }
    if (words_msg[0] == "KEYS"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "UNLINK"){
        std::string command = create_RESP_command_for_hget(words_msg);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "EXPIRE"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "RENAME"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "LPUSH"){
        std::string command = create_RESP_command_for_hget(words_msg);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::vector res = break_text_to_words(buffer);
            return res[0];
        }
    }
    if (words_msg[0] == "RPUSH"){
        std::string command = create_RESP_command_for_hget(words_msg);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::vector res = break_text_to_words(buffer);
            return res[0];
        }
    }
    if (words_msg[0] == "LRANGE"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 4);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "RRANGE"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 4);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "LPOP"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "RPOP"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "LLEN"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "LREM"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 4);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "LLEN"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "LINDEX"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "LSET"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 4);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "HEXISTS"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "HSET"){
        std::string command = create_RESP_command_for_hget(words_msg);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "HGET"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 3);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "HDEL") {
        std::string command = create_RESP_command_for_hget(words_msg);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "HGETALL"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "HKEYS"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "HLEN"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "HMSET"){
        std::string command = create_RESP_command_for_hget(words_msg);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (words_msg[0] == "HVALS"){
        std::string command = create_RESP_command(words_msg[0], words_msg[1], words_msg[2], words_msg[3], 2);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    return "Error command!\n";
}

std::string command_no_space_SSL(std::string msg, int sock, char* buffer, SSL* ssl){
    if (msg == "ping"){
        std::string command = create_RESP_command(msg, "", "", "", 1);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
            std::vector res = break_text_to_words(buffer);
            return res[0];
        }
    }
    if (msg == "info"){
        std::string command = create_RESP_command(msg, "", "", "", 1);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
    }
    if (msg == "flushall"){
        std::string command = create_RESP_command(msg, "", "", "", 1);
        SSL_write(ssl, command.c_str(), sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
            std::vector res = break_text_to_words(buffer);
            return res[0];
        }
    }
    return "Error command!\n";
}

bool quit(std::string msg){
    if (msg == "quit"){
        return true;
    }
}


// Функция для инициализации SSL
void initSSL() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// Функция для очистки SSL
void cleanupSSL() {
    EVP_cleanup();
}

// Функция для создания SSL-соединения
SSL* createSSLConnection(const std::string& host, int port, const std::string& certFile) {
    int sock;
    struct sockaddr_in server;
    SSL* ssl;
    SSL_CTX* ctx;

    // Создаем TCP-соединение
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    server.sin_addr.s_addr = inet_addr(host.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Инициализируем SSL
    ctx = SSL_CTX_new(TLS_client_method());
    if (ctx == nullptr) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Устанавливаем сертификат CA
    if (SSL_CTX_load_verify_locations(ctx, certFile.c_str(), nullptr) != 1) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    if (SSL_connect(ssl) != 1) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ssl;
}

int main(int argc, char* argv[]) {
    std::string host = SERVER_IP;
    int port = DEFAULT_PORT;
    std::string certFile = "";
    bool useTLS = false;

    // Парсим аргументы командной строки
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--tls") {
            useTLS = true;
        } else if (arg == "--cacert") {
            if (i + 1 < argc) {
                certFile = argv[++i];
            } else {
                std::cerr << "--cacert requires a certificate file path" << std::endl;
                return 1;
            }
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            return 1;
        }
    }

    initSSL();
    SSL* ssl = nullptr;
    int sock;
    std::string message;
    char buffer[BUFFER_SIZE] = {0};
    std::string command;
    if (useTLS) {
        if (certFile.empty()) {
            std::cerr << "--cacert is required when using --tls" << std::endl;
            cleanupSSL();
            return 1;
        }
        ssl = createSSLConnection(host, port, certFile);
        sock = SSL_get_fd(ssl);
    }
    else {
        struct sockaddr_in serv_addr;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            cout << "Ошибка создания сокета" << std::endl;
            return -1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(DEFAULT_PORT);

        if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
            cout << "Неверный адрес или адрес не поддерживается" << std::endl;
            return -1;
        }

        if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            cout << "Ошибка подключения" << std::endl;
            return -1;
        }

        while (true) {
            cout << SERVER_IP << ":" << DEFAULT_PORT << ">";
            getline(cin, message);
            if (quit(message)) {
                break;
            }
            if (message.find(' ') > 0 && message.find(' ') != 18446744073709551615) {
                cout << commands_with_any_attribute(message, sock, buffer) << "\n";
            }
            if (message.find(' ') == 18446744073709551615) {
                cout << command_no_space(message, sock, buffer) << "\n";
            }
        }
        close(sock);
        return 0;
    }
    while(true) {
        cout << ">";
        getline(cin, message);
        if (quit(message)) {
            break;
        }
        if (message.find(' ') > 0 && message.find(' ') != 18446744073709551615) {
            cout << commands_with_any_attribute_SSL(message, sock, buffer, ssl) << "\n";
        }
        if (message.find(' ') == 18446744073709551615) {
            cout << command_no_space_SSL(message, sock, buffer, ssl) << "\n";
        }
    }
    close(sock);
    return 0;

//    std::string* cmd = "*1\r\n$4\r\nPING\r\n";
//    send(sock, cmd, cmd->size(), 0);
//
//    // Получаем ответ от сервера Redis
//    char buffer[BUFFER_SIZE];
//    int bytesRead = recv(sock, buffer, sizeof(buffer), 0);
//    if (bytesRead > 0) {
//        buffer[bytesRead] = '\0';
//        std::cout << "Server response: " << buffer << std::endl;
//    }

    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    close(sock);

    cleanupSSL();
    return 0;
}


