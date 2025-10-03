#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cerrno>
#include <csignal>
#include <sys/select.h>
#include <fcntl.h>
#include <cmath>
#include <cctype>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace common {
    // Constants
    constexpr int MAX_MESSAGE_LEN = 8192;
    constexpr int MAX_PLAYER_ID_LEN = 256;
    constexpr int MAX_POINTS = 10001;
    constexpr int MAX_DEGREE = 8;
    constexpr int MAX_CLIENTS = 1000;
    constexpr int TIMEOUT_HELLO = 3;
    constexpr int PENALTY_BAD_PUT = 10;
    constexpr int PENALTY_EARLY_PUT = 20;

    // Message types
    constexpr const char* MSG_HELLO = "HELLO";
    constexpr const char* MSG_COEFF = "COEFF";
    constexpr const char* MSG_PUT = "PUT";
    constexpr const char* MSG_STATE = "STATE";
    constexpr const char* MSG_BAD_PUT = "BAD_PUT";
    constexpr const char* MSG_PENALTY = "PENALTY";
    constexpr const char* MSG_SCORING = "SCORING";

    // Function prototypes
    void error_exit(const std::string& msg);
    bool parse_double(const std::string& str, double* result);
    bool parse_int(const std::string& str, int* result);
    void send_message(int socket, const std::string& message);
    bool receive_message(int socket, std::string& buffer);
    std::string format_double(double value);
    void print_diagnostic(const std::string& message);
}

#endif // COMMON_H
