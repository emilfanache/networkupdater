#ifndef HTTP_TEST_SERVER_
#define HTTP_TEST_SERVER_

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <map>
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
    int InitServer();
    void StopServer();
    int BuildHttpReply(int err_code, std::string* reply);
    void ListenForConnections();
    void WaitForConnections();
    int GetTestErrCode(const std::string& request_content);

    struct sockaddr_in sock_addr_;
    int server_fd_;
    static constexpr uint32_t kMaxConnectionNumber = 5;
    // HARDCODE error codes to test my content
    std::map<const char*, int, cmp_str> code_map_ = {{"b1", 401},
                                                     {"b2", 404},
                                                     {"b3", 409},
                                                     {"b4", 500}};
};

#endif  // HTTP_TEST_SERVER_