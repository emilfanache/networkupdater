#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "../include/json.hpp"
#include "./http_test_server.hpp"

HttpTestServer::HttpTestServer(const std::string& ip_address, int port) {
    sock_addr_.sin_family = AF_INET;
    sock_addr_.sin_port = htons(port);
    sock_addr_.sin_addr.s_addr = inet_addr(ip_address.c_str());

    if (InitServer() < 0) {
        throw(std::runtime_error("Unable to initialize socket server!"));
    }

    WaitForConnections();
}

HttpTestServer::~HttpTestServer() {
    StopServer();
}

void HttpTestServer::ListenForConnections() {
    int status = listen(server_fd_, kMaxConnectionNumber);
    if (listen(server_fd_, kMaxConnectionNumber) < 0) {
        std::cout << "Error listening to socket:" << errno << std::endl;
        StopServer();
        return;
    }
}

int HttpTestServer::InitServer() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cout << "Error opening socket:" << errno << std::endl;
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
        0) {
        std::cout << "Error setting socket option:" << errno << std::endl;
        StopServer();
        return -1;
    }

    if (bind(server_fd_, reinterpret_cast<sockaddr*>(&sock_addr_),
             sizeof(sock_addr_)) < 0) {
        std::cout << "Error binding to socket:" << errno << std::endl;
        StopServer();
        return -1;
    }

    return 0;
}

void HttpTestServer::StopServer() {
    close(server_fd_);
}

void HttpTestServer::WaitForConnections() {
    // std::cout << "----- WaitForConnections -----" << std::endl;

    ListenForConnections();

    int bytes;
    int addrlen = sizeof(sock_addr_);
    constexpr uint32_t kBufferSize = 1024 * 10;  // 10 kbytes
    std::shared_ptr<char[]> buffer(new char[kBufferSize]);

    while (true) {
        int new_socket =
            accept(server_fd_, reinterpret_cast<sockaddr*>(&sock_addr_),
                   reinterpret_cast<socklen_t*>(&addrlen));

        bytes = read(new_socket, buffer.get(), kBufferSize);
        if (bytes < 0) {
            close(new_socket);
            std::cout << "Error reading from socket:" << errno << std::endl;
            StopServer();
            return;
        }

        // std::cout << " = = = = = = = = Received: = = = = = = = =" << std::endl;
        // std::cout << buffer.get() << std::endl;

        int code = GetTestErrCode(std::string(buffer.get()));

        std::string reply;
        if (BuildHttpReply(code, &reply) == 0) {
            bytes = write(new_socket, reply.c_str(), reply.size());
            if (bytes < 0) {
                close(new_socket);
                std::cout << "Error writing to socket:" << errno << std::endl;
                StopServer();
                return;
            }
        }

        close(new_socket);
    }
}

int HttpTestServer::BuildHttpReply(int err_code, std::string* reply) {
    // TODO(emil): Avoid hardcoded path
    std::string resource_path{"../test/headers/"};
    resource_path += std::to_string(err_code) + ".txt";

    std::ifstream input_file(resource_path.c_str());
    if (input_file.is_open()) {
        std::string header;
        std::stringstream file_stream;
        file_stream << input_file.rdbuf();

        // success case
        if (err_code == 200) {
            header = std::string(
                "HTTP/1.1 200 OK\nContent-Type: "
                "application/json\nContent-Length:");
        } else {
            auto json = nlohmann::json::parse(file_stream.str());
            header = std::string("HTTP/1.1 ") + std::to_string(err_code) +
                     std::string(" ") + std::string(json["error"]) +
                     std::string(
                         "\nContent-Type: application/json\nContent-Length:");
        }

        std::string reply_header = header +
                                   std::to_string(file_stream.str().size()) +
                                   std::string("\n\n");

        std::stringstream str_stream;
        str_stream << reply_header;
        str_stream << file_stream.str();
        *reply = str_stream.str();
        // std::cout << " = = = = = = = = Replying: = = = = = = = =" << std::endl;
        // std::cout << reply->c_str() << std::endl;
        return 0;
    }

    return -1;
}

int HttpTestServer::GetTestErrCode(const std::string& request_content) {
    std::istringstream str_stream(request_content);
    std::string first_line;
    getline(str_stream, first_line);
    std::size_t pos = first_line.find("clientId:");
    std::string mac_first_octet =
        first_line.substr(pos + strlen("clientId:"), 2);

    // HARDCODE error codes to test my content
    std::map<const char*, int, cmp_str> code_map = {
        {"b1", 401}, {"b2", 404}, {"b3", 409}, {"b4", 500}};

    if (code_map.count(mac_first_octet.c_str())) {
        return code_map[mac_first_octet.c_str()];
    }

    return 200;
}