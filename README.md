# Networkupdater

Networkupdater is a tool that sends HTTP requests to several hosts in a network based on a predefined list of MAC addresses. The purpose is to send a configuration in json format.
The implementation is done in C++

## Requirements

The application was written in C++ and uses libraries like g++, Cmake, cpr (Curl for people) and googletest.<br/>
These are the recommended versions:<br/>
C++17<br/>
Cmake 3.20 (or later)<br/>
cpr 1.9.0 (https://docs.libcpr.org/)<br/>

CPR and Googletest versions are fetched via cmake. (Please see CMakelists.txt in the root folder)<br/>
Depdens on openssl, libssl-dev.<br/>

## Reasoning for technologies choice

This project comes as a good practice tool for my C++ skills. And since this is my current language of focus it was the natural choice.<br/>
Cmake for building the project, more intuitive, comes with a lot of documentation all over the tech sites.<br/>
cpr (Curl for people) offers a very intuitive API simplifying the many options one needs to set for a basic curl request. I tested several tools and this was the one that I found the easiest to understand and gave me the fewest problems.<br/>

## Quick start

- Prepare the json config to send in a separate json file. By default resources/versions.json is used.<br/>
- Prepare the host list in this format:<br/>
```mac_addresses
"11:aa:cc:dd:ee:ff"
"22:bb:cc:dd:ee:ff"
"33:bb:cc:dd:ee:ff"
"44:bb:cc:dd:ee:ff"
"55:bb:cc:dd:ee:ff"
"66:bb:cc:dd:ee:ff"
```
By default resources/input.csv is used.<br/>

```bash
mkdir build && cd build
cmake ..
make
./test_updater         # Run unit tests
./network_updater      # Sends the HTTP request to a server on listening on port 8080 (by default)
./http_test_server     # Runs a TCP server that receives the request and replys with harcoded 
                       # HTTP responses. Strictly used for testing purpose
```

## Supplemenatry options
networkupdater can be configured with several options described the in command's help.<br/>

```
#./network_updater --help
Usage: ./network_updater [-h] [-j <file>] [-m <file>] [-u <url>] [-p <port_no>] [-l <logfile>] [-f {0|1}]
    -h,--help   Show this help message
    -j,--json   Path of the json config file to be added in the HTTP request
    -m,--mac-file   Path of the host file containing the MAC addresses of the hosts
    -u,--uri    HTTP destination address of the request. Default is http://localhost. Must include http:// prefix.
    -p,--port   HTTP server port number. Default is 8080
    -l,--log-file   Location of the logs describing the host result
    -f,--fail-fast  The execution should exit at the first failed request
```

## Limitations
At the moment the tool is not supported on Windows hosts.<br/>
The HTTP server is not meant to be used by itself. It has several hardcoded components meant to test several specific scenarios of the tool.<br/>
At the moment there is no token support, the token returned by default will be 123456789abcdef123456789abcdefs.<br/>
The tool doesn't spawn a server to run in a loop. If this needs to be run at certain time intervals please use a cronjob.<br/>


## Testing considerations
The main ideea of the HTTP server was to reply to the updater with several specific HTTP statuses: 200, 401, 404, 409, 500. In order to make the server reply with these codes, the test searches for the first octet of the MAC address in the HTTP request header and it replies like this:<br/>

```
"b1" -> 401 (e.g. b1:11:22:33:44:55)
"b2" -> 404
"b3" -> 409
"b4" -> 500
```
The server is spwaned as a dettached thread in the google test SetUpTestSuite() static method that is executed before the suite run making it available for all the test fixtures.<br/>

## Special thanks

https://docs.libcpr.org/<br/>
https://github.com/nlohmann/json <br/>
https://stackoverflow.com/<br/>
https://github.com/OsasAzamegbe/http-server<br/>
