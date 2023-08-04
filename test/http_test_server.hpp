#ifndef HTTP_TEST_SERVER_
#define HTTP_TEST_SERVER_

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <string>

class HttpTestServer {
 public:
    struct cmp_str {
        bool operator()(char const* a, char const* b) const {
            return strcmp(a, b) < 0;
        }
    };

    HttpTestServer(const std::string& ip_address, int port);
    ~HttpTestServer();

 private:
    struct sockaddr_in sock_addr_;
    int server_fd_;
    static constexpr uint32_t kMaxConnectionNumber = 5;

    int InitServer();
    void StopServer();
    int BuildHttpReply(int err_code, std::string* reply);
    void ListenForConnections();
    void WaitForConnections();
    int GetTestErrCode(const std::string& request_content);
};

#endif  // HTTP_TEST_SERVER_