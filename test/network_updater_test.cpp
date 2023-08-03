#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

#include <fstream>
#include <memory>
#include <sstream>

#include "../include/network_updater.hpp"
#include "../test/http_test_server.hpp"

class NetworkUpdaterTest : public ::testing::Test {
 protected:
    void SetUp() override {
        CreateHostFile();
        CreateJsonConfig();
    }

    void TearDown() override {
        remove(host_file_.c_str());
        remove(json_config_.c_str());
    }

    void CreateHostFile() {
        std::ofstream hostf(host_file_.c_str());
        if (hostf.is_open()) {
            hostf << "\"mac_addresses, id1, id2, id3\"\n"
                  << "\"b1:11:cc:dd:ee:ff, 1, 2, 3\"\n"
                  << "\"b2:22:cc:dd:ee:ff, 1, 2, 3\"\n"
                  << "\"b3:33:cc:dd:ee:ff, 1, 2, 3\"\n"
                  << "\"b4:44:cc:dd:ee:ff, 1, 2, 3\"\n";
        }
    }

    void CreateJsonConfig() {
        std::ofstream jsonc(json_config_.c_str());
        if (jsonc.is_open()) {
            char const* string = R"(
            {
                "profile": {
                    "applications": [
                    {
                        "id": "my_app",
                        "version": "v1.2.3"
                    },
                    {
                        "id": "your_app",
                        "version": "v4.5.6"
                    }
                    ]
                }
            })";
        }
    }

 protected:
    std::string host_file_{"test_host_file.txt"};
    std::string json_config_{"test_config.json"};
    std::string uri_{"http://localhost"};
    int port_{8080};
};

TEST_F(NetworkUpdaterTest, ThrowWrongHostFile) {
    std::unique_ptr<NetworkUpdater> nwup;
    ASSERT_THROW(
        nwup = std::make_unique<NetworkUpdater>(
            "wrong_file.txt", json_config_.c_str(), uri_.c_str(), port_),
        std::invalid_argument);
}

TEST_F(NetworkUpdaterTest, ThrowNoMacAddresses) {
    std::ofstream empty_file("empty_file.csv");
    std::unique_ptr<NetworkUpdater> nwup;
    ASSERT_THROW(
        nwup = std::make_unique<NetworkUpdater>(
            "empty_file.csv", json_config_.c_str(), uri_.c_str(), port_),
        std::runtime_error);
}

TEST_F(NetworkUpdaterTest, ThrowWrongJsonConfig) {
    std::unique_ptr<NetworkUpdater> nwup;
    ASSERT_THROW(
        nwup = std::make_unique<NetworkUpdater>(
            host_file_.c_str(), "wrong_file.json", uri_.c_str(), port_),
        std::invalid_argument);
}

TEST_F(NetworkUpdaterTest, ThrowWrongURI) {
    std::unique_ptr<NetworkUpdater> nwup;
    ASSERT_THROW(
        nwup = std::make_unique<NetworkUpdater>(
            host_file_.c_str(), json_config_.c_str(), "something", port_),
        std::invalid_argument);

    ASSERT_THROW(
        nwup = std::make_unique<NetworkUpdater>(
            host_file_.c_str(), json_config_.c_str(), "www.abc", port_),
        std::invalid_argument);

    ASSERT_THROW(
        nwup = std::make_unique<NetworkUpdater>(
            host_file_.c_str(), json_config_.c_str(), "www.abc.com", port_),
        std::invalid_argument);

    ASSERT_THROW(
        nwup = std::make_unique<NetworkUpdater>(
            host_file_.c_str(), json_config_.c_str(), "abc.com", port_),
        std::invalid_argument);
}

TEST_F(NetworkUpdaterTest, CheckMacList) {
    std::unique_ptr<NetworkUpdater> nwup;
    EXPECT_NO_THROW(
        nwup = std::make_unique<NetworkUpdater>(
            host_file_.c_str(), json_config_.c_str(), uri_.c_str(), port_));

    std::vector<std::string> mac_list = nwup->GetMacList();

    // 4 mac addresses in the host file
    EXPECT_EQ(mac_list.size(), 4);

    ASSERT_THAT(mac_list,
                testing::ElementsAre("b1:11:cc:dd:ee:ff", "b2:22:cc:dd:ee:ff",
                                     "b3:33:cc:dd:ee:ff", "b4:44:cc:dd:ee:ff"));
}
