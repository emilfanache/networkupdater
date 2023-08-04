#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "include/network_updater.hpp"

const char default_json_config[] = "../resources/versions.json";
const char default_host_file[] = "../resources/input.csv";
uint32_t NetworkUpdater::kTokenRetryCount = 3;
const char default_log_file[] = "../logs/result.log";

static void ShowHelp() {
    std::cout
        << "Usage: ./network_updater [-h] [-j <file>] [-m <file>] [-u "
           "<url>] [-p <port_no>] [-l <logfile>] [-f {0|1}]\n"
        << "\t-h,--help\tShow this help message\n"
        << "\t-j,--json\tPath of the json config file to be added in "
           "the HTTP request\n"
        << "\t-m,--mac-file\tPath of the host file containing the MAC "
           "addresses of the hosts\n"
        << "\t-u,--uri\tHTTP destination address of the request. "
           "Default is http://localhost. Must include http:// prefix.\n"
        << "\t-p,--port\tHTTP server port number. Default is 8080\n"
        << "\t-l,--log-file\tLocation of the logs describing the host result\n"
        << "\t-f,--fail-fast\tThe execution should exit at the first failed "
           "request\n"
        << std::endl;
}

int main(int argc, char* argv[]) {
    const char* host_file = default_host_file;
    const char* json_config = default_json_config;
    const char* uri = "http://localhost";
    int port = 8080;
    const char* log_file = default_log_file;
    bool fast_exit = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            ShowHelp();
            return 0;
        } else if ((arg == "-j") || (arg == "--json")) {
            if (i + 1 >= argc) {
                std::cout << "Invalid json option" << std::endl;
                ShowHelp();
                return -1;
            }
            json_config = argv[i + 1];
        } else if ((arg == "-m") || (arg == "--mac-file")) {
            if (i + 1 >= argc) {
                std::cout << "Invalid mac file option" << std::endl;
                ShowHelp();
                return -1;
            }
            host_file = argv[i + 1];
        } else if ((arg == "-u") || (arg == "--uri")) {
            if (i + 1 >= argc) {
                std::cout << "Invalid uri option" << std::endl;
                ShowHelp();
                return -1;
            }
            uri = argv[i + 1];
        } else if ((arg == "-p") || (arg == "--port")) {
            if (i + 1 >= argc) {
                std::cout << "Invalid port option" << std::endl;
                ShowHelp();
                return -1;
            }
            port = atoi(argv[i + 1]);
        } else if ((arg == "-l") || (arg == "--log-file")) {
            if (i + 1 >= argc) {
                std::cout << "Invalid log file option" << std::endl;
                ShowHelp();
                return -1;
            }
            log_file = argv[i + 1];
        } else if ((arg == "-f") || (arg == "--fail-fast")) {
            if (i + 1 >= argc) {
                std::cout << "Invalid port option" << std::endl;
                ShowHelp();
                return -1;
            }
            int fex = atoi(argv[i + 1]);
            fast_exit = (fex != 0);
        }
    }

    std::ofstream output_file(log_file);
    if (!output_file.is_open()) {
        std::cout << "WARNING: Unable to open log file. The execution will "
                     "continue without logging."
                  << std::endl;
    }

    std::unique_ptr<NetworkUpdater> nwup;
    try {
        nwup =
            std::make_unique<NetworkUpdater>(host_file, json_config, uri, port);
    } catch (std::invalid_argument const& e) {
        std::cout << e.what() << std::endl;
        return -1;
    }

    for (const auto& mac : nwup->GetMacList()) {
        uint32_t status_code = 0;
        NetworkUpdater::UpdaterErr status =
            nwup->SendRequest(mac, &status_code);
        if (status == NetworkUpdater::UpdaterErr::Fail) {
            if (fast_exit) {
                std::cout << "Unable to send request for the host with mac "
                          << mac.c_str() << std::endl;
                return -1;
            } else {
                output_file << "Unable to send request for the host with mac "
                            << mac.c_str() << std::endl;
            }
        }

        if (status == NetworkUpdater::UpdaterErr::Retry) {
            uint32_t retry_iteration = 1;  // first call already happened
            while (retry_iteration <= NetworkUpdater::kTokenRetryCount &&
                   status == NetworkUpdater::UpdaterErr::Retry) {
                output_file << "Retrying to send request after getting token "
                            << "for host mac: " << mac.c_str() << std::endl;
                retry_iteration++;
                status = nwup->SendRequest(mac, &status_code);
            }

            if (status != NetworkUpdater::UpdaterErr::Ok) {
                if (fast_exit) {
                    std::cout << "Unable to send request for the host with mac "
                              << mac.c_str() << std::endl;
                    return -1;
                } else {
                    output_file
                        << "Unable to send request for the host with mac "
                        << mac.c_str() << std::endl;
                }
            }
        }
    }

    std::cout << "Done!" << std::endl;

    return 0;
}
