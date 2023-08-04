#include <iostream>
#include <stdexcept>
#include <string>

#include "http_test_server.hpp"

static void ShowHelp() {
    std::cout << "Usage: ./htpp_server [-h] [-i <ip>] [-p <port>] \n"
              << "\t-h,--help\tShow this help message\n"
              << "\t-i,--ip-addr\tThe IP address to which the server will "
                 "bind. Default is 0.0.0.0\n"
              << "\t-p,--port\tHTTP server port number. Default is 8080\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string ip_addr{"0.0.0.0"};

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            ShowHelp();
            return 0;
        } else if ((arg == "-i") || (arg == "--ip-addr")) {
            if (i + 1 >= argc) {
                std::cout << "Invalid ip addr option" << std::endl;
                ShowHelp();
                return -1;
            }
            ip_addr = std::string(argv[i + 1]);
        } else if ((arg == "-p") || (arg == "--port")) {
            if (i + 1 >= argc) {
                std::cout << "Invalid port option" << std::endl;
                ShowHelp();
                return -1;
            }
            port = atoi(argv[i + 1]);
        }
    }

    try {
        HttpTestServer htpp_server(ip_addr, port);
    } catch (std::runtime_error const& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}