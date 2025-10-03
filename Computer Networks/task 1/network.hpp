#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <array>

#include "message.hpp"

class UdpSocket {
public:
    UdpSocket(uint16_t port = 0, const std::string& bind_address = "0.0.0.0", 
              uint64_t timeout_seconds = 2, uint64_t timeout_microseconds = 0);
    ~UdpSocket();
    UdpSocket(const UdpSocket& other) = delete;  // Zablokuj kopiowanie
    UdpSocket& operator=(const UdpSocket& other) = delete;
    UdpSocket(UdpSocket&& other) noexcept;
    UdpSocket& operator=(UdpSocket&& other) noexcept;
    void send_to(Message& message, const std::string& ip, uint16_t port);
    void send_to(Message& message, const uint32_t& ip, uint16_t port);
    ssize_t recv_from(std::string& sender_ip, uint16_t& sender_port);
    
    std::array<uint8_t, BUFFER_SIZE> buffer;  // Buffer is now a member variable

    int get_fd() const;

    void set_timeout(uint64_t seconds = 0, uint64_t microseconds = 0);

private:
    int sockfd;
    sockaddr_in addr;

    sockaddr_in make_sockaddr(const std::string& ip, uint16_t port);
};


#endif // NETWORK_HPP