#include "network.hpp"
#include <iostream>
#include <netdb.h>  // For getaddrinfo

// Debug constant reference

UdpSocket::UdpSocket(uint16_t port, const std::string& bind_address, 
                     uint64_t timeout_seconds, uint64_t timeout_microseconds) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        throw std::runtime_error("socket() failed");

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    // Try first with inet_pton (IP address)
    if (inet_pton(AF_INET, bind_address.c_str(), &addr.sin_addr) <= 0) {
        // If inet_pton fails, try to resolve as a hostname
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        
        int status = getaddrinfo(bind_address.c_str(), nullptr, &hints, &res);
        if (status != 0)
            throw std::runtime_error("Cannot resolve bind address: " + std::string(gai_strerror(status)));
        
        // Copy the resolved address
        addr.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
        freeaddrinfo(res);
    }

    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");

    set_timeout(timeout_seconds, timeout_microseconds);
}

UdpSocket::~UdpSocket() {
    if (sockfd >= 0)
        close(sockfd);
}

// Move constructor - correctly ordered initialization
UdpSocket::UdpSocket(UdpSocket&& other) noexcept 
    : buffer(std::move(other.buffer)), sockfd(other.sockfd), addr(other.addr) {
    // Take ownership of the descriptor and set the source object to -1,
    // so that the destructor doesn't close the socket
    other.sockfd = -1;
}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept {
    if (this != &other) {
        // Close the previous socket if it was open
        if (sockfd >= 0) {
            close(sockfd);
        }
        
        // Take ownership of resources in the same order as in the constructor
        buffer = std::move(other.buffer);
        sockfd = other.sockfd;
        addr = other.addr;
        
        // Reset the source object
        other.sockfd = -1;
    }
    return *this;
}

void send_to_common(Message& message, sockaddr_in& dest, int sockfd, std::array<uint8_t, BUFFER_SIZE>& buffer) {
    size_t message_size = message.serialize(buffer);
    
    // Debug print for sending messages
    if constexpr (DEBUG) {
        char ip_buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &dest.sin_addr, ip_buf, sizeof(ip_buf));
        std::string ip_str(ip_buf);
        uint16_t port_num = ntohs(dest.sin_port);
        message.debug_print("Sending", ip_str, port_num);
    }
    
    ssize_t sent = sendto(sockfd, buffer.data(), message_size, 0,
                          (sockaddr*)&dest, sizeof(dest));

    if (sent < 0)
        throw std::runtime_error("sendto() failed");
}

void UdpSocket::send_to(Message& message, const std::string& ip, uint16_t port) {
    sockaddr_in dest = make_sockaddr(ip, port);

    send_to_common(message, dest, sockfd, buffer);
}

void UdpSocket::send_to(Message& message, const uint32_t& ip, uint16_t port) {
    sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = htonl(ip);

    send_to_common(message, dest, sockfd, buffer);
}

ssize_t UdpSocket::recv_from(std::string& sender_ip, uint16_t& sender_port) {
    if (sockfd < 0) {
        throw std::runtime_error("Socket is not initialized or already closed");
    }
    sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);

    ssize_t received = recvfrom(sockfd, buffer.data(), BUFFER_SIZE, 0,
                                (sockaddr*)&sender, &sender_len);

    if (received < 0) {
        // Check if this is a timeout
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if constexpr (DEBUG) {
                std::cout << "Socket timeout occurred" << std::endl;
            }
            return 0; // Return 0 to indicate timeout
        } else {
            // Other error
            throw std::runtime_error("recvfrom() failed: " + std::string(strerror(errno)));
        }
    }

    char ip_buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sender.sin_addr, ip_buf, sizeof(ip_buf));
    sender_ip = ip_buf;
    sender_port = ntohs(sender.sin_port);
    
    // Debug printing for received messages
    if constexpr (DEBUG) {
        if (received < 1) {
            std::cerr << "Received empty message from " << sender_ip << ":" << sender_port << std::endl;
        }
        uint8_t message_type = buffer[0];
        
        // Create a temporary message object of the appropriate type to print debug info
        switch(message_type) {
            case HELLO: {
                Hello msg(buffer);
                msg.debug_print("Received", sender_ip, sender_port);
                break;
            }
            case HELLO_REPLY: {
                HelloReply msg(buffer);
                msg.debug_print("Received", sender_ip, sender_port);
                break;
            }
            case CONNECT: {
                Connect msg(buffer);
                msg.debug_print("Received", sender_ip, sender_port);
                break;
            }
            case ACK_CONNECT: {
                AckConnect msg(buffer);
                msg.debug_print("Received", sender_ip, sender_port);
                break;
            }
            case SYNC_START: {
                SyncStart msg(buffer);
                msg.debug_print("Received", sender_ip, sender_port);
                break;
            }
            case DELAY_REQUEST: {
                DelayRequest msg(buffer);
                msg.debug_print("Received", sender_ip, sender_port);
                break;
            }
            case DELAY_RESPONSE: {
                DelayResponse msg(buffer);
                msg.debug_print("Received", sender_ip, sender_port);
                break;
            }
            case LEADER: {
                Leader msg(buffer);
                msg.debug_print("Received", sender_ip, sender_port);
                break;
            }
            case GET_TIME: {
                GetTime msg(buffer);
                msg.debug_print("Received", sender_ip, sender_port);
                break;
            }
            case TIME: {
                Time msg(buffer);
                msg.debug_print("Received", sender_ip, sender_port);
                break;
            }
            default: {
                std::cout << "Received unknown message type: " << static_cast<int>(message_type) 
                          << " from " << sender_ip << ":" << sender_port << std::endl;
            }
        }
    }

    return received;
}

int UdpSocket::get_fd() const {
    return sockfd;
}

void UdpSocket::set_timeout(uint64_t seconds, uint64_t microseconds) {
    timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = microseconds;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        throw std::runtime_error("setsockopt(SO_RCVTIMEO) failed");
}

sockaddr_in UdpSocket::make_sockaddr(const std::string& ip, uint16_t port) {
    sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    
    // Try first with inet_pton (IP address)
    if (inet_pton(AF_INET, ip.c_str(), &sa.sin_addr) <= 0) {
        // If inet_pton fails, try to resolve as a hostname
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        
        int status = getaddrinfo(ip.c_str(), nullptr, &hints, &res);
        if (status != 0)
            throw std::runtime_error("Cannot resolve destination address: " + std::string(gai_strerror(status)));
        
        // Copy the resolved address
        sa.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
        freeaddrinfo(res);
    }

    return sa;
}
