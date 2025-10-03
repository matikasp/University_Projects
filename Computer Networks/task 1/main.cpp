#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/socket.h>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <chrono>

#include "network.hpp"
#include "message.hpp"

struct Node {
    std::array<Peer, MAX_PEERS> peers = {Peer()};
    uint16_t peer_count = 0;
    uint16_t port = 0;
    uint16_t peer_port = 0;
    int64_t curr_time_offset = 0;
    int64_t next_time_offset = 0;
    uint8_t sync_level = 255; // Default sync level
    bool connect_to_peer = false;
    bool isSynchronizing = false;
    uint32_t synchronized_peer_ip = 0;
    uint16_t synchronized_peer_port = 0;
    uint32_t synchronizing_peer_ip = 0;
    uint16_t synchronizing_peer_port = 0;
    UdpSocket socket;
    std::vector<Peer> unack_peers;
    std::string bind_address = "0.0.0.0";
    std::string peer_address = "";
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> last_sync_time; // Last time we received SYNC_START from our synchronized peer
    
};

static Node node;

void connect_to_peer () {
    Hello hello;
    node.socket.send_to(hello, node.peer_address, node.peer_port);
    std::string sender_ip = "";
    uint16_t sender_port = -1;
    size_t received;
    
    // Wait for a HELLO_REPLY message from the peer we're trying to connect to
    do {
        received = node.socket.recv_from(sender_ip, sender_port);
        
        if (received < 1) continue; // Skip empty messages
        
        if (sender_ip == node.peer_address && 
            sender_port == node.peer_port && 
            getMessageType(node.socket.buffer) == HELLO_REPLY) {
            break;  // Found the message we're looking for
        }
    } while (true);
    // Now deserialize the message as a HelloReply
    HelloReply hello_reply(node.socket.buffer);

    node.peers[node.peer_count++] = Peer(sender_ip, sender_port);
    
    node.unack_peers = hello_reply.peers;
    for (Peer& peer : node.unack_peers) {
        // Convert IP address from network byte order to string and back
        // to fix the byte order issue
        char ip_buf[INET_ADDRSTRLEN];
        struct in_addr addr;
        addr.s_addr = peer.address;
        inet_ntop(AF_INET, &addr, ip_buf, sizeof(ip_buf));
        std::string peer_ip(ip_buf);
        
        Connect connect;
        node.socket.send_to(connect, peer_ip, peer.port);
    }
}

bool parse_arguments(int argc, char* argv[]) {
    bool has_b = false, has_p = false, has_a = false, has_r = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-b" && i + 1 < argc) {
            if (!has_b) {
                node.bind_address = argv[++i];
                has_b = true;
            } else {
                ++i; // skip duplicate
            }
        }
        else if (arg == "-p" && i + 1 < argc) {
            if (!has_p) {
                node.port = std::atoi(argv[++i]);
                has_p = true;
            } else {
                ++i;
            }
        }
        else if (arg == "-a" && i + 1 < argc) {
            if (!has_a) {
                node.peer_address = argv[++i];
                has_a = true;
            } else {
                ++i;
            }
        }
        else if (arg == "-r" && i + 1 < argc) {
            if (!has_r) {
                node.peer_port = std::atoi(argv[++i]);
                if (node.peer_port < 1) {
                    std::cerr << "Error: peer_port must be in range 1–65535.\n";
                    return false;
                }
                has_r = true;
            } else {
                ++i;
            }
        }
        else {
            std::cerr << "Error: unknown argument or missing value: " 
                      << arg << "\n";
            return false;
        }
    }

    if ((has_a && !has_r) || (!has_a && has_r)) {
        std::cerr << "Error: both -a and -r must be provided together.\n";
        return false;
    }

    node.connect_to_peer = has_a && has_r;
    return true;
}

void print_usage() {
    std::cout << "Usage:\n"
              << "  ./node [-b bind_address] [-p port] [-a peer_address -r peer_port]\n"
              << "Options:\n"
              << "  -b   IP address to listen on (default: 0.0.0.0)\n"
              << "  -p   Port to listen on (0–65535, default: 0)\n"
              << "  -a   Address of another node to connect to\n"
              << "  -r   Port of the peer node (1–65535)\n";
}

void log_error_message(const std::array<uint8_t, BUFFER_SIZE>& buffer, size_t received) {
    std::cerr << "ERROR MSG ";
    size_t length_to_print = std::min<size_t>(10, received);
    for (size_t i = 0; i < length_to_print; ++i) {
        std::cerr << std::hex << static_cast<int>(buffer[i]);
    }
    std::cerr << std::endl;
}

int main(int argc, char* argv[]) {
    if (!parse_arguments(argc, argv)) {
        print_usage();
        return 1;
    }
    node.start_time = std::chrono::steady_clock::now();
    node.curr_time_offset = 0;
    
    node.socket = UdpSocket(node.port, node.bind_address, 2, 0);

    if (node.connect_to_peer) {
        connect_to_peer();
    }

    auto last_message_time = std::chrono::steady_clock::now();

    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_message_time).count();
        
        // Check if we haven't received a SYNC_START from our synchronized peer for too long (20-30 seconds)
        if (node.sync_level < 255 && node.synchronized_peer_ip != 0) {
            auto sync_elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - node.last_sync_time).count();
            if (sync_elapsed > 20) { 
                // We haven't received SYNC_START from our synchronized peer for too long
                node.sync_level = 255;
                node.synchronized_peer_ip = 0;
                node.synchronized_peer_port = 0;
                node.curr_time_offset = 0;
            }
        }
        
        if (elapsed > 5 && node.sync_level < 254) {
            // send sync_start message every 5-10 seconds
            // Calculate current time relative to start time plus any offset
            int64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - node.start_time).count() - node.curr_time_offset;
            
            // Create SYNC_START message with node's sync level and current timestamp
            SyncStart sync_start(node.sync_level, current_time);
            
            // Send SYNC_START to all known peers
            for (uint16_t i = 0; i < node.peer_count; ++i) {
                std::string peer_ip = inet_ntoa({node.peers[i].address});
                node.socket.send_to(sync_start, peer_ip, node.peers[i].port);
            }
            
            last_message_time = now;
        }
        std::string sender_ip;
        uint16_t sender_port;
        ssize_t received = node.socket.recv_from(sender_ip, sender_port);
        now = std::chrono::steady_clock::now();
        uint32_t sender_ip_num = inet_addr(sender_ip.c_str());
        if constexpr (DEBUG) {
            std::cout << "Node state:\n"
                                << "  Sync Level: " << static_cast<int>(node.sync_level) << "\n"
                                << "  Synchronized Peer IP: " << inet_ntoa({node.synchronized_peer_ip}) << "\n"
                                << "  Synchronized Peer Port: " << node.synchronized_peer_port << "\n"
                                << "  Synchronizing Peer IP: " << inet_ntoa({node.synchronizing_peer_ip}) << "\n"
                                << "  Synchronizing Peer Port: " << node.synchronizing_peer_port << "\n"
                                << "  Current Time Offset: " << node.curr_time_offset << "\n"
                                << "  Next Time Offset: " << node.next_time_offset << "\n"
                                << "  Peer Count: " << node.peer_count << "\n"
                                << "  Last time synced: " << node.last_sync_time.time_since_epoch().count() << "\n"
                                << "  Synced time: " << std::chrono::duration_cast<std::chrono::milliseconds>(now - node.start_time).count() - node.curr_time_offset << "\n";
        }
        if (received == 0) {
            // This is a timeout, silently continue without logging an error
            continue;
        } else if (received < 0) {
            // This shouldn't happen as exceptions should be thrown by recv_from for actual errors
            log_error_message(node.socket.buffer, received);
            continue;
        }
        
        uint8_t message_type = getMessageType(node.socket.buffer);
        
        switch (message_type) {
            case HELLO: {
                HelloReply hello_reply(node.peer_count, node.peers);
                node.socket.send_to(hello_reply, sender_ip, sender_port);
                node.peers[node.peer_count++] = Peer(sender_ip, sender_port);
                break;
            }
            case CONNECT: {
                Connect connect(node.socket.buffer);
                if (node.peer_count < MAX_PEERS) {
                    node.peers[node.peer_count++] = Peer(sender_ip, sender_port);
                    AckConnect ack_connect;
                    node.socket.send_to(ack_connect, sender_ip, sender_port);
                } else {
                    log_error_message(node.socket.buffer, received);
                }
                break;
            } 
            case ACK_CONNECT: {
                AckConnect ack_connect(node.socket.buffer);
                auto it = std::find_if(node.unack_peers.begin(), node.unack_peers.end(), 
                                       [&](const Peer& peer) {
                                           return peer.address == sender_ip_num && peer.port == sender_port;
                                       });

                if (it != node.unack_peers.end() && node.peer_count < MAX_PEERS) {
                    node.peers[node.peer_count++] = *it;
                    node.unack_peers.erase(it);
                } else {
                    log_error_message(node.socket.buffer, received);
                }
                break;
            }
            case SYNC_START: {
                SyncStart sync_start(node.socket.buffer);
                auto it = std::find_if(node.peers.begin(), node.peers.begin() + node.peer_count, 
                                       [&](const Peer& peer) {
                                           return peer.address == sender_ip_num && peer.port == sender_port;
                                       });

                if (it != node.peers.begin() + node.peer_count && sync_start.synchronized < 254) {
                    // Check if this is our synchronized peer
                    if (node.synchronized_peer_ip == sender_ip_num && 
                        node.synchronized_peer_port == sender_port) {
                        
                        // Update the last sync time since we received a SYNC_START from our synchronized peer
                        node.last_sync_time = std::chrono::steady_clock::now();
                        
                        // If synchronized peer's level is >= our level, set our sync_level to 255
                        if (sync_start.synchronized >= node.sync_level) {
                            node.sync_level = 255;
                            node.synchronized_peer_ip = 0;
                            node.synchronized_peer_port = 0;
                            node.isSynchronizing = false;
                        } else {
                            node.synchronizing_peer_ip = sender_ip_num;
                            node.synchronizing_peer_port = sender_port;
                            node.isSynchronizing = true;
                        }
                    } else if (sync_start.synchronized < node.sync_level - 1) {
                        // Synchronize with this peer as it has a lower sync level
                        node.synchronizing_peer_ip = sender_ip_num;
                        node.synchronizing_peer_port = sender_port;
                        node.isSynchronizing = true;
                        node.last_sync_time = std::chrono::steady_clock::now();
                    }
                    if (node.isSynchronizing) {
                        DelayRequest delay_request;
                        int64_t time_from_program_start = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now - node.start_time).count();
                        node.next_time_offset = time_from_program_start - sync_start.timestamp + std::chrono::duration_cast<std::chrono::milliseconds>(now - node.start_time).count();
                        node.socket.send_to(delay_request, sender_ip, sender_port);
                    }
                    
                } else {
                    log_error_message(node.socket.buffer, received);
                }
                
                break;
            }
            case DELAY_REQUEST: {
                if (node.sync_level == 255) {
                    log_error_message(node.socket.buffer, received);
                    break;
                }
                DelayRequest delay_request(node.socket.buffer);
                auto it = std::find_if(node.peers.begin(), node.peers.begin() + node.peer_count, 
                                       [&](const Peer& peer) {
                                           return peer.address == sender_ip_num && peer.port == sender_port;
                                       });
                if (it != node.peers.begin() + node.peer_count) {
                    int64_t time_from_program_start = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - node.start_time).count();
                    // Send DELAY_RESPONSE with our sync level and current timestamp
                    DelayResponse delay_response(node.sync_level, time_from_program_start);
                    node.socket.send_to(delay_response, sender_ip, sender_port);
                } else {
                    log_error_message(node.socket.buffer, received);
                }
                break;
            }
            case DELAY_RESPONSE: {
                DelayResponse delay_response(node.socket.buffer);
                if (node.isSynchronizing &&
                    node.synchronizing_peer_ip == sender_ip_num &&
                    node.synchronizing_peer_port == sender_port) {
                        node.next_time_offset -= delay_response.timestamp;
                        node.curr_time_offset = node.next_time_offset/2;
                        node.sync_level = delay_response.synchronized + 1;
                        node.synchronized_peer_ip = sender_ip_num;
                        node.synchronized_peer_port = sender_port;
                        node.isSynchronizing = false;
                } else {
                    log_error_message(node.socket.buffer, received);
                }
                break;
            }
            case LEADER: {
                Leader leader(node.socket.buffer);
                
                if (leader.synchronized == 0) {
                    node.sync_level = 0;
                } else if (leader.synchronized == 255 && node.sync_level == 0) {
                    node.sync_level = 255;
                } else {
                    log_error_message(node.socket.buffer, received);
                }

                break;
            }
            case GET_TIME: {
                // Handle GET_TIME message
                GetTime get_time(node.socket.buffer);
                now = std::chrono::steady_clock::now();
                int64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(now - node.start_time).count();
                if (node.sync_level < 255) {
                    time -= node.curr_time_offset;
                }
                Time time_message(node.sync_level, time);
                node.socket.send_to(time_message, sender_ip, sender_port);
                break;
            }
            default: {
                log_error_message(node.socket.buffer, received);
                break;
            }
        }
    }
    return 0;
}