#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cinttypes>
#include <ctime>
#include <vector>
#include <array>
#include <iostream>

#ifndef NDEBUG
    constexpr bool DEBUG = false;
#else
    constexpr bool DEBUG = true;
#endif

constexpr uint8_t ADDRESS_LENGTH = 4;
constexpr size_t BUFFER_SIZE = 65536;
constexpr size_t MAX_PEERS = 65536;

// message types
constexpr uint8_t HELLO = 1;
constexpr uint8_t HELLO_REPLY = 2;
constexpr uint8_t CONNECT = 3;
constexpr uint8_t ACK_CONNECT = 4;
constexpr uint8_t SYNC_START = 11;
constexpr uint8_t DELAY_REQUEST = 12;
constexpr uint8_t DELAY_RESPONSE = 13;
constexpr uint8_t LEADER = 21;
constexpr uint8_t GET_TIME = 31;
constexpr uint8_t TIME = 32;

inline uint8_t getMessageType(std::array<uint8_t, BUFFER_SIZE>& buffer) {
    return buffer[0];
}

struct Message {
    uint8_t type;
    Message() = default;
    Message(uint8_t type) : type(type) {}
    virtual size_t serialize(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        buffer[0] = type;
        return 1;
    }
    virtual void deserialize(const std::array<uint8_t, BUFFER_SIZE>& buffer) {
        type = buffer[0];
    }
    
    // Debug printing function that will be overridden by derived classes
    virtual void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const {
        if constexpr (DEBUG) {
            std::string msg_type = get_message_type_name();
            std::cout << direction << " basic message: " << msg_type 
                      << " (type=" << static_cast<int>(type) << ")"
                      << " with " << ip << ":" << port << std::endl;
        }
    }
    
    // Helper to get message type name
    std::string get_message_type_name() const {
        switch (type) {
            case HELLO: return "HELLO";
            case HELLO_REPLY: return "HELLO_REPLY";
            case CONNECT: return "CONNECT";
            case ACK_CONNECT: return "ACK_CONNECT";
            case SYNC_START: return "SYNC_START";
            case DELAY_REQUEST: return "DELAY_REQUEST";
            case DELAY_RESPONSE: return "DELAY_RESPONSE";
            case LEADER: return "LEADER";
            case GET_TIME: return "GET_TIME";
            case TIME: return "TIME";
            default: return "UNKNOWN";
        }
    }
};


// --------------------------messages with only type---------------------------


// ale w sumie to mozna by zrobic funkcje ktora tworzy odpowiedni typ wiadmosci na podstawie parametru)
struct Hello : public Message {
    Hello() : Message(HELLO) {}
    Hello(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        deserialize(buffer);
    }
    
    void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::cout << direction << " HELLO message (type=" << static_cast<int>(type) 
                      << ") with " << ip << ":" << port << std::endl;
        }
    }
};

struct Connect : public Message {
    Connect() : Message(CONNECT) {}
    Connect(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        deserialize(buffer);
    }
    
    void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::cout << direction << " CONNECT message (type=" << static_cast<int>(type) 
                      << ") with " << ip << ":" << port << std::endl;
        }
    }
};

struct AckConnect : public Message {
    AckConnect() : Message(ACK_CONNECT) {}
    AckConnect(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        deserialize(buffer);
    }
    
    void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::cout << direction << " ACK_CONNECT message (type=" << static_cast<int>(type) 
                      << ") with " << ip << ":" << port << std::endl;
        }
    }
};

struct DelayRequest : public Message {
    DelayRequest() : Message(DELAY_REQUEST) {}
    DelayRequest(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        deserialize(buffer);
    }
    
    void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::cout << direction << " DELAY_REQUEST message (type=" << static_cast<int>(type) 
                      << ") with " << ip << ":" << port << std::endl;
        }
    }
};

struct GetTime : public Message {
    GetTime() : Message(GET_TIME) {}
    GetTime(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        deserialize(buffer);
    }
    
    void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::cout << direction << " GET_TIME message (type=" << static_cast<int>(type) 
                      << ") with " << ip << ":" << port << std::endl;
        }
    }
};


// --------------------------messages with timestamp---------------------------

struct TimeStampMessage : public Message {
    uint8_t synchronized;
    int64_t timestamp;
    TimeStampMessage() = default;
    TimeStampMessage(uint8_t type, uint8_t synchronized, int64_t timestamp) : 
            Message(type), synchronized(synchronized), timestamp(timestamp) {}
    virtual size_t serialize(std::array<uint8_t, BUFFER_SIZE>& buffer) override {
        buffer[0] = type;
        buffer[1] = synchronized;
        for (size_t i = 0; i < sizeof(timestamp); ++i) {
            buffer[i + 2] = static_cast<uint8_t>((timestamp >> (i * 8)) & 0xFF);
        }
        return sizeof(timestamp) + 2;
    }
    virtual void deserialize(const std::array<uint8_t, BUFFER_SIZE>& buffer) override {
        type = buffer[0];
        synchronized = buffer[1];
        timestamp = 0;
        for (size_t i = 0; i < sizeof(timestamp); ++i) {
            timestamp |= static_cast<int64_t>(buffer[i + 2]) << (i * 8);
        }
    }
    
    // Override debug_print to include timestamp and sync level
    virtual void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::string msg_type = get_message_type_name();
            std::cout << direction << " message type: " << msg_type 
                      << " (" << static_cast<int>(type) << ")"
                      << " with " << ip << ":" << port
                      << " | sync_level: " << static_cast<int>(synchronized)
                      << " | timestamp: " << timestamp << std::endl;
        }
    }
};

struct SyncStart : public TimeStampMessage {
    SyncStart(uint8_t synchronized, int64_t timestamp) : 
            TimeStampMessage(SYNC_START, synchronized, timestamp) {}
    SyncStart(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        deserialize(buffer);
    }
    
    void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::cout << direction << " SYNC_START message (type=" << static_cast<int>(type) 
                      << ") with " << ip << ":" << port
                      << " | sync_level=" << static_cast<int>(synchronized)
                      << " | timestamp=" << timestamp << std::endl;
        }
    }
};

struct DelayResponse : public TimeStampMessage {
    DelayResponse(uint8_t synchronized, int64_t timestamp) : 
            TimeStampMessage(DELAY_RESPONSE, synchronized, timestamp) {}
    DelayResponse(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        deserialize(buffer);
    }
    
    void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::cout << direction << " DELAY_RESPONSE message (type=" << static_cast<int>(type) 
                      << ") with " << ip << ":" << port
                      << " | sync_level=" << static_cast<int>(synchronized)
                      << " | timestamp=" << timestamp << std::endl;
        }
    }
};

struct Time : public TimeStampMessage {
    Time(uint8_t synchronized, int64_t timestamp) : 
            TimeStampMessage(TIME, synchronized, timestamp) {}
    Time(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        deserialize(buffer);
    }
    
    void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::cout << direction << " TIME message (type=" << static_cast<int>(type) 
                      << ") with " << ip << ":" << port
                      << " | sync_level=" << static_cast<int>(synchronized)
                      << " | timestamp=" << timestamp << std::endl;
        }
    }
};


//---------------------------other messages------------------------------------

// struct for peer information
struct Peer {
    uint32_t address;
    uint16_t port;
    Peer() : address(0), port(0) {}
    Peer(uint32_t address, uint16_t port) : 
            address(address), port(port) {}
    Peer(std::string address, uint16_t port) : 
            address(inet_addr(address.c_str())), port(port) {}
};

struct HelloReply : public Message {
    std::vector<Peer> peers;
    HelloReply(uint16_t peer_count, std::array<Peer, MAX_PEERS>& peers) : Message(HELLO_REPLY) {
        this->peers.reserve(peer_count);
        for (uint16_t i = 0; i < peer_count; ++i) {
            this->peers.emplace_back(peers[i]);
        }
    }
    HelloReply(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        deserialize(buffer);
    }
    size_t serialize(std::array<uint8_t, BUFFER_SIZE>& buffer) override {
        uint16_t count = peers.size();
        buffer[0] = type;
        
        buffer[1] = static_cast<uint8_t>((count >> 8) & 0xFF);
        buffer[2] = static_cast<uint8_t>(count & 0xFF);
        size_t offset = 3;
        
        for (uint16_t i = 0; i < count && i < peers.size(); ++i) {
            const auto& peer = peers[i];
            buffer[offset++] = ADDRESS_LENGTH;
            for (size_t j = 0; j < ADDRESS_LENGTH; ++j) {
                buffer[offset++] = static_cast<uint8_t>((peer.address >> (j * 8)) & 0xFF);
            }
            buffer[offset++] = static_cast<uint8_t>((peer.port >> 8) & 0xFF);
            buffer[offset++] = static_cast<uint8_t>(peer.port & 0xFF);
        }
        return offset;
    }
    void deserialize(const std::array<uint8_t, BUFFER_SIZE>& buffer) override {
        type = buffer[0];
        uint16_t count = (static_cast<uint16_t>(buffer[1]) << 8) | static_cast<uint16_t>(buffer[2]);
        size_t offset = 3;
        for (size_t i = 0; i < count; ++i) {
            uint8_t received_address_length = buffer[offset++];
            if (received_address_length != ADDRESS_LENGTH) {
                std::cerr << "ERROR MSG ";
                for (size_t k = 0; k < 10 && k < BUFFER_SIZE; ++k) {
                    std::cerr << std::hex << static_cast<int>(buffer[k]) << " ";
                }
            }
            
            // Convert 4 bytes to a uint32_t address
            uint32_t address = 0;
            for (size_t j = 0; j < ADDRESS_LENGTH; ++j) {
                address |= static_cast<uint32_t>(buffer[offset++]) << (j * 8);
            }
            
            uint16_t port = (static_cast<uint16_t>(buffer[offset]) << 8) | static_cast<uint16_t>(buffer[offset + 1]);
            offset += 2;
            
            peers.emplace_back(address, port);
        }
    }
    
    void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::cout << direction << " HELLO_REPLY message (type=" << static_cast<int>(type) 
                      << ") with " << ip << ":" << port
                      << " | peer_count=" << peers.size() << std::endl;
            
            // Print details of each peer in the reply
            for (size_t i = 0; i < peers.size(); i++) {
                char peer_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &peers[i].address, peer_ip, sizeof(peer_ip));
                std::cout << "   Peer " << i << ": " << peer_ip << ":" << peers[i].port << std::endl;
            }
        }
    }
};

struct Leader : public Message {
    uint8_t synchronized;
    Leader(uint8_t synchronized) : Message(LEADER), 
            synchronized(synchronized) {}
    Leader(std::array<uint8_t, BUFFER_SIZE>& buffer) {
        deserialize(buffer);
    }
    size_t serialize(std::array<uint8_t, BUFFER_SIZE>& buffer) override {
        buffer[0] = type;
        buffer[1] = synchronized;
        return 2;
    }
    void deserialize(const std::array<uint8_t, BUFFER_SIZE>& buffer) override {
        type = buffer[0];
        synchronized = buffer[1];
    }
    
    void debug_print(const std::string& direction, const std::string& ip, uint16_t port) const override {
        if constexpr (DEBUG) {
            std::cout << direction << " LEADER message (type=" << static_cast<int>(type) 
                      << ") with " << ip << ":" << port
                      << " | sync_level=" << static_cast<int>(synchronized) << std::endl;
        }
    }
}; 

#endif // MESSAGE_HPP