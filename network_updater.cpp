#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>

#include <cpr/cpr.h>
#include "include/json.hpp"
#include "include/network_updater.hpp"

NetworkUpdater::NetworkUpdater(const char* hosts_fname, const char* json_fname,
                               const char* uri, int port) {
    if (ReadMacAddrList(hosts_fname) == NetworkUpdater::UpdaterErr::Fail) {
        throw(std::invalid_argument(
            "Invalid hosts file name. Can not retrieve client mac addresses!"));
    }

    if (ReadJsonConfig(json_fname) == NetworkUpdater::UpdaterErr::Fail) {
        throw(std::invalid_argument("Invalid json config file name!"));
    }

    uri_ = std::string(uri);
    if (!IsUrlValid(uri_)) {
        throw(std::invalid_argument("Invalid destination address!"));
    }

    port_ = port;
}

NetworkUpdater::UpdaterErr NetworkUpdater::ReadMacAddrList(
    const char* hosts_fname) {
    std::string line;
    std::ifstream input_file(hosts_fname);
    std::string result;

    if (!input_file.is_open()) {
        return NetworkUpdater::UpdaterErr::Fail;
    }

    while (std::getline(input_file, line)) {
        if (line.find("mac") != std::string::npos) {
            continue;
        }

        std::stringstream line_stream(line);
        if (getline(line_stream, result, ',')) {
            result.erase(std::remove(result.begin(), result.end(), '"'),
                         result.end());
            mac_list_.push_back(result);
        }
    }

    return NetworkUpdater::UpdaterErr::Ok;
}

NetworkUpdater::UpdaterErr NetworkUpdater::ReadJsonConfig(
    const char* json_fname) {
    std::ifstream input_file(json_fname);
    std::string result;

    if (!input_file.is_open()) {
        return NetworkUpdater::UpdaterErr::Fail;
    }

    std::stringstream str_stream;
    str_stream << input_file.rdbuf();
    json_config_ = str_stream.str();

    return NetworkUpdater::UpdaterErr::Ok;
}

NetworkUpdater::UpdaterErr NetworkUpdater::SendRequest(
    const std::string& mac_addr) {
    std::string uri;
    std::string port_string;
    if (port_) {
        port_string = std::string(":") + std::to_string(port_);
    }

    uri = uri_ + port_string + std::string("/profiles/clientId:") + mac_addr;
    // std::cout << uri.c_str() << std::endl;

    // before sending the request we need a token
    RequestToken();

    std::string client_id = std::to_string(GenerateHttpId());
    cpr::Response r =
        cpr::Put(cpr::Url{uri.c_str()}, cpr::Body{json_config_.c_str()},
                 cpr::Header{{"Content-Type", "application/json"},
                             {"x-client-id", client_id.c_str()},
                             {"x-authentication-token", token_.c_str()}});

    // std::cout << r.text << std::endl;
    // auto json = nlohmann::json::parse(r.text);
    // std::cout << json.dump(4) << std::endl;

    switch (r.status_code) {
        case NetworkUpdater::HttpError::Success:
            return NetworkUpdater::UpdaterErr::Ok;

        // either the token is not right or it expired
        case NetworkUpdater::HttpError::AuthError:
            RequestToken();
            return NetworkUpdater::UpdaterErr::Retry;

        case NetworkUpdater::HttpError::InvalidProfileOrClient:
        case NetworkUpdater::HttpError::Conflict:
        case NetworkUpdater::HttpError::InternalError: {
            auto json = nlohmann::json::parse(r.text);
            if (json["message"]) {
                std::cout << json["message"] << std::endl;
            }
            return NetworkUpdater::UpdaterErr::Fail;
        }

        default:
            std::cout << "Request failed with an unknown error. Code: "
                      << r.status_code << std::endl;
            break;
    }

    return NetworkUpdater::UpdaterErr::Fail;
}

uint32_t NetworkUpdater::GenerateHttpId() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1, kMaxClientId);
    return dist(mt);
}

void NetworkUpdater::RequestToken() {
#if VALID_TOKEN_SCENARIO
#if CPR_LIBCURL_VERSION_NUM >= 0x073D00
    // Perform the request like usually:
    cpr::Response r = cpr::Get(cpr::Url{"http://www.httpbin.org/bearer"},
                               cpr::Bearer{"ACCESS_TOKEN"});
#else
    // Manually add the Authorization header:
    cpr::Response r = cpr::Get(cpr::Url{"http://www.httpbin.org/bearer"},
                               cpr::HeaderAuthorization);
#endif
    auto json = nlohmann::json::parse(r.text);
    if (!json["token"]) {
        throw(std::runtime_error("Unable to get authentication token"))
    }
    token_ = std::string(json["token"]);
#endif
    token_ = std::string("823f3161ae4f4495bf0a90c00a7dfbff");
}

std::vector<std::string> const& NetworkUpdater::GetMacList() const {
    return mac_list_;
}

bool NetworkUpdater::IsUrlValid(const std::string& url) {
    std::regex url_regex(R"(^https?://[0-9a-z\.-]+(:[1-9][0-9]*)?(/[^\s]*)*$)");
    return std::regex_match(url, url_regex);
}