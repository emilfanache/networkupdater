#ifndef HTTP_TEST_SERVER_
#define HTTP_TEST_SERVER_

#include <netinet/in.h>
#include <sys/socket.h>
#include <string>

class HttpTestServer {
 public:
    HttpTestServer(const std::string& ip_address, int port);
    ~HttpTestServer();
    void WaitForConnections();

 private:
    struct sockaddr_in sock_addr_;
    int server_fd_;
    static constexpr uint32_t kMaxConnectionNumber = 5;

    int InitServer();
    void StopServer();
    int BuildHttpReply(int err_code, std::string* reply);
};

#endif  // HTTP_TEST_SERVER_