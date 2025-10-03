#include "common.h"

namespace common {

void error_exit(const std::string& msg) {
    if (errno != 0) {
        std::cerr << "ERROR: " << msg << " (errno: " << errno << " - " << strerror(errno) << ")" << std::endl;
    } else {
        std::cerr << "ERROR: " << msg << std::endl;
    }
    exit(1);
}

bool parse_double(const std::string& str, double* result) {
    try {
        size_t pos;
        *result = std::stod(str, &pos);
        return pos == str.length();
    } catch (const std::exception&) {
        return false;
    }
}

bool parse_int(const std::string& str, int* result) {
    try {
        size_t pos;
        *result = std::stoi(str, &pos);
        return pos == str.length();
    } catch (const std::exception&) {
        return false;
    }
}

void send_message(int socket, const std::string& message) {
    std::string full_message = message + "\r\n";
    ssize_t bytes_sent = 0;
    ssize_t total_bytes = full_message.length();
    
    while (bytes_sent < total_bytes) {
        ssize_t result = send(socket, full_message.c_str() + bytes_sent, 
                             total_bytes - bytes_sent, MSG_NOSIGNAL);
        if (result <= 0) {
            std::string error_msg = "Failed to send message";
            if (errno != 0) {
                error_msg += " (errno: " + std::to_string(errno) + " - " + strerror(errno) + ")";
            }
            throw std::runtime_error(error_msg);
        }
        bytes_sent += result;
    }
}

bool receive_message(int socket, std::string& buffer) {
    buffer.clear();
    char c;
    bool got_cr = false;
    
    while (true) {
        errno = 0;  // Clear errno before system call
        ssize_t result = recv(socket, &c, 1, 0);
        if (result <= 0) {
            if (result == 0) {
                // Connection closed by peer
                return false;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Non-blocking socket would block
                return false;
            }
            // Other error occurred
            return false;
        }
        
        if (c == '\r') {
            got_cr = true;
        } else if (c == '\n' && got_cr) {
            return true;
        } else if (got_cr) {
            buffer += '\r';
            buffer += c;
            got_cr = false;
        } else {
            buffer += c;
        }
        
        if (buffer.length() > MAX_MESSAGE_LEN) {
            return false;
        }
    }
}

std::string format_double(double value) {
    std::ostringstream oss;
    oss << std::fixed;
    
    // Remove trailing zeros and decimal point if not needed
    if (value == static_cast<int>(value)) {
        oss << std::setprecision(0) << value;
    } else {
        oss << std::setprecision(7) << value;
        std::string str = oss.str();
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        if (str.back() == '.') {
            str.pop_back();
        }
        return str;
    }
    return oss.str();
}

void print_diagnostic(const std::string& message) {
    std::cout << message << std::endl;
}

} // namespace common
