#ifndef NETWORK_UPDATER_HPP_
#define NETWORK_UPDATER_HPP_

#include <string>
#include <vector>

#ifdef VALID_TOKEN_SCENARIO
#undef VALID_TOKEN_SCENARIO
#endif

// define it here in order to use the token functionality
// #define VALID_TOKEN_SCENARIO 1

class NetworkUpdater {
 public:
    enum UpdaterErr { Fail = -1, Ok = 0, Retry = 1 };

    enum HttpError {
        Success = 200,
        AuthError = 401,
        InvalidProfileOrClient = 404,
        Conflict = 409,
        InternalError = 500,

    };

    NetworkUpdater(const char* hosts_fname, const char* json_fname,
                   const char* uri, int port);
    ~NetworkUpdater() = default;
    NetworkUpdater::UpdaterErr SendRequest(const std::string& mac_addr);
    std::vector<std::string> const& GetMacList() const;

    static uint32_t kTokenRetryCount;

 private:
    NetworkUpdater::UpdaterErr ReadMacAddrList(const char* hosts_fname);
    NetworkUpdater::UpdaterErr ReadJsonConfig(const char* json_fname);
    uint32_t GenerateHttpId();
    void RequestToken();
    bool IsUrlValid(const std::string& url);

    static constexpr uint32_t kMaxClientId = 65535;
    std::string json_config_;
    std::vector<std::string> mac_list_;
    std::string token_;
    std::string uri_;
    int port_;
};

#endif  // NETWORK_UPDATER_HPP_
