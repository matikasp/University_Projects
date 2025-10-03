#include "common.h"
#include <queue>
#include <map>
#include <algorithm>
#include <sys/epoll.h>
#include <vector>

using namespace common;

struct DelayedEvent {
    time_t execute_time;
    int client_fd;
    std::string response;
    bool is_bad_put_response;
    
    // For priority queue (earlier times have higher priority)
    bool operator>(const DelayedEvent& other) const {
        return execute_time > other.execute_time;
    }
};

struct Client {
    int socket;
    std::string player_id;
    std::string ip_address;
    int port;
    std::vector<double> coefficients;
    std::vector<double> approximation;
    double penalty;
    bool got_hello;
    bool got_coeff;
    bool waiting_for_response;
    time_t connect_time;
    int lowercase_count;
};

struct ServerConfig {
    int port;
    int K;
    int N;
    int M;
    std::string coefficients_file;
};

class ApproxServer {
private:
    ServerConfig config;
    int server_socket;
    int epoll_fd;
    int total_puts;
    std::map<int, Client> clients;
    std::priority_queue<DelayedEvent, std::vector<DelayedEvent>, std::greater<DelayedEvent>> event_queue;
    std::ifstream coeff_file;
    
public:
    ApproxServer(const ServerConfig& cfg) : config(cfg), epoll_fd(-1), total_puts(0) {
        coeff_file.open(config.coefficients_file);
        if (!coeff_file.is_open()) {
            error_exit("Cannot open coefficients file: " + config.coefficients_file);
        }
    }
    
    ~ApproxServer() {
        if (epoll_fd >= 0) {
            close(epoll_fd);
        }
        if (server_socket >= 0) {
            close(server_socket);
        }
        coeff_file.close();
    }
    
    void start() {
        setup_server_socket();
        print_diagnostic("Server listening on port " + std::to_string(config.port));
        
        while (true) {
            handle_connections();
            if (total_puts >= config.M) {
                send_scoring();
                reset_game();
            }
        }
    }
    
private:
    void setup_server_socket() {
        server_socket = socket(AF_INET6, SOCK_STREAM, 0);
        if (server_socket < 0) {
            error_exit("Cannot create server socket");
        }
        
        int opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        int v6only = 0;
        setsockopt(server_socket, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
        
        struct sockaddr_in6 server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin6_family = AF_INET6;
        server_addr.sin6_addr = in6addr_any;
        server_addr.sin6_port = htons(config.port);
        
        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            error_exit("Cannot bind server socket");
        }
        
        if (config.port == 0) {
            socklen_t len = sizeof(server_addr);
            getsockname(server_socket, (struct sockaddr*)&server_addr, &len);
            config.port = ntohs(server_addr.sin6_port);
        }
        
        if (listen(server_socket, 10) < 0) {
            error_exit("Cannot listen on server socket");
        }
        
        // Setup epoll
        epoll_fd = epoll_create1(0);
        if (epoll_fd < 0) {
            error_exit("Cannot create epoll instance");
        }
        
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = server_socket;
        
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) < 0) {
            error_exit("Cannot add server socket to epoll");
        }
    }
    
    void handle_connections() {
        const int MAX_EVENTS = 64;
        struct epoll_event events[MAX_EVENTS];
        
        // Calculate timeout in milliseconds based on next event in queue
        int timeout_ms = calculate_next_timeout_ms();
        
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout_ms);
        
        if (num_events > 0) {
            for (int i = 0; i < num_events; i++) {
                int fd = events[i].data.fd;
                
                if (fd == server_socket) {
                    accept_new_client();
                } else {
                    // Handle client message
                    auto client_it = clients.find(fd);
                    if (client_it != clients.end()) {
                        if (!handle_client_message(client_it->second)) {
                            remove_client(fd);
                        }
                    }
                }
            }
        }
        
        // Process all due events
        process_due_events();
        
        check_timeouts();
    }
    
    int calculate_next_timeout_ms() {
        time_t now = time(nullptr);
        time_t next_event_time = now + 30; // Default max timeout
        bool has_event = false;
        
        // Check event queue
        if (!event_queue.empty()) {
            next_event_time = std::min(next_event_time, event_queue.top().execute_time);
            has_event = true;
        }
        
        // Check client timeouts
        for (const auto& [fd, client] : clients) {
            if (!client.got_hello) {
                time_t timeout_time = client.connect_time + TIMEOUT_HELLO;
                next_event_time = std::min(next_event_time, timeout_time);
                has_event = true;
            }
        }
        
        if (has_event) {
            time_t wait_time = std::max(static_cast<time_t>(0), next_event_time - now);
            return static_cast<int>(wait_time * 1000); // Convert to milliseconds
        }
        
        return 30000; // 30 seconds default
    }
    
    void remove_client(int fd) {
        // Remove from epoll
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
        // Close socket
        close(fd);
        // Remove from clients map
        clients.erase(fd);
    }
    
    void process_due_events() {
        time_t now = time(nullptr);
        
        while (!event_queue.empty() && event_queue.top().execute_time <= now) {
            DelayedEvent event = event_queue.top();
            event_queue.pop();
            
            // Check if client still exists
            auto client_it = clients.find(event.client_fd);
            if (client_it == clients.end()) {
                continue; // Client disconnected
            }
            
            // Send the delayed response
            send_message(event.client_fd, event.response);
            
            if (event.is_bad_put_response) {
                // BAD_PUT response - no additional actions needed
            } else {
                // STATE response
                Client& client = client_it->second;
                print_diagnostic("Sending state " + format_state(client.approximation) + 
                                " to " + client.player_id + ".");
                client.waiting_for_response = false;
            }
        }
    }
    
    void accept_new_client() {
        struct sockaddr_in6 client_addr;
        socklen_t addr_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            return;
        }
        
        char ip_str[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &client_addr.sin6_addr, ip_str, INET6_ADDRSTRLEN);
        int port = ntohs(client_addr.sin6_port);
        
        print_diagnostic("New client [" + std::string(ip_str) + "]:" + std::to_string(port) + ".");
        
        // Add client socket to epoll
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = client_socket;
        
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) < 0) {
            close(client_socket);
            return;
        }
        
        Client client;
        client.socket = client_socket;
        client.ip_address = ip_str;
        client.port = port;
        client.penalty = 0;
        client.got_hello = false;
        client.got_coeff = false;
        client.waiting_for_response = false;
        client.connect_time = time(nullptr);
        client.approximation.resize(config.K + 1, 0.0);
        client.lowercase_count = 0;
        
        clients[client_socket] = client;
    }
    
    bool handle_client_message(Client& client) {
        std::string message;
        if (!receive_message(client.socket, message)) {
            return false;
        }
        
        std::istringstream iss(message);
        std::string command;
        iss >> command;
        
        if (command == MSG_HELLO && !client.got_hello) {
            return handle_hello(client, message);
        } else if (command == MSG_PUT && client.got_coeff) {
            return handle_put(client, iss, message);
        } else {
            print_bad_message(client, message);
            if (!client.got_hello) {
                return false;
            }
        }
        
        return true;
    }
    
    bool handle_hello(Client& client, const std::string& original_message) {
        // Validate exact format: "HELLO player_id" - exactly one space between HELLO and player_id
        std::string remaining = original_message.substr(5); // Skip "HELLO"
        
        // Should start with exactly one space
        if (remaining.empty() || remaining[0] != ' ') {
            print_bad_message(client, original_message);
            return false;
        }
        
        // Remove the first space
        remaining = remaining.substr(1);
        
        // Should not start with another space (no multiple spaces allowed)
        if (remaining.empty() || remaining[0] == ' ') {
            print_bad_message(client, original_message);
            return false;
        }
        
        // Player ID should contain only alphanumeric characters and no spaces
        std::string player_id = remaining;
        for (char c : player_id) {
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
                print_bad_message(client, original_message);
                return false;
            }
        }
        
        if (player_id.empty()) {
            print_bad_message(client, original_message);
            return false;
        }
        
        client.player_id = player_id;
        client.got_hello = true;
        
        // Count lowercase letters for delay
        client.lowercase_count = 0;
        for (char c : player_id) {
            if (c >= 'a' && c <= 'z') {
                client.lowercase_count++;
            }
        }
        
        print_diagnostic(client.ip_address + ":" + std::to_string(client.port) + 
                        " is now known as " + player_id + ".");
        
        // Send coefficients
        send_coefficients(client);
        return true;
    }
    
    void send_coefficients(Client& client) {
        std::string line;
        if (!std::getline(coeff_file, line)) {
            error_exit("No more coefficients in file");
        }
        
        // Parse COEFF message
        std::istringstream iss(line);
        std::string coeff_cmd;
        iss >> coeff_cmd;
        
        if (coeff_cmd != MSG_COEFF) {
            error_exit("Invalid coefficients file format");
        }
        
        client.coefficients.clear();
        double coeff;
        while (iss >> coeff) {
            client.coefficients.push_back(coeff);
        }
        
        client.got_coeff = true;
        
        // Remove trailing \r\n or \n safely
        std::string clean_line = line;
        while (!clean_line.empty() && (clean_line.back() == '\r' || clean_line.back() == '\n')) {
            clean_line.pop_back();
        }
        
        send_message(client.socket, clean_line);
        
        std::string coeff_str;
        for (size_t i = 0; i < client.coefficients.size(); ++i) {
            if (i > 0) coeff_str += " ";
            coeff_str += format_double(client.coefficients[i]);
        }
        print_diagnostic(client.player_id + " get coefficients " + coeff_str + ".");
    }
    
    bool handle_put(Client& client, std::istringstream& iss, const std::string& original_message) {
        if (client.waiting_for_response) {
            std::string point_str, value_str;
            iss >> point_str >> value_str;
            send_message(client.socket, "PENALTY " + point_str + " " + value_str);
            client.penalty += PENALTY_EARLY_PUT;
            return true;
        }
        
        std::string point_str, value_str;
        if (!(iss >> point_str >> value_str) || !iss.eof()) {
            print_bad_message(client, original_message);
            return true;
        }
        
        int point;
        double value;
        
        if (!parse_int(point_str, &point) || point < 0 || point > config.K ||
            !parse_double(value_str, &value) || value < -5.0 || value > 5.0) {
            
            // Schedule BAD_PUT response with 1 second delay
            DelayedEvent event;
            event.execute_time = time(nullptr) + 1;
            event.client_fd = client.socket;
            event.response = "BAD_PUT " + point_str + " " + value_str;
            event.is_bad_put_response = true;
            event_queue.push(event);
            
            client.penalty += PENALTY_BAD_PUT;
            return true;
        }
        
        // Valid PUT
        client.approximation[point] += value;
        total_puts++;
        
        print_diagnostic(client.player_id + " puts " + format_double(value) + 
                        " in " + std::to_string(point) + ", current state " + 
                        format_state(client.approximation) + ".");
        
        client.waiting_for_response = true;
        
        // Prepare response with delay
        std::string state_msg = "STATE " + format_state(client.approximation);
        
        if (client.lowercase_count > 0) {
            // Schedule delayed response
            DelayedEvent event;
            event.execute_time = time(nullptr) + client.lowercase_count;
            event.client_fd = client.socket;
            event.response = state_msg;
            event.is_bad_put_response = false;
            event_queue.push(event);
        } else {
            // Send immediately
            send_message(client.socket, state_msg);
            print_diagnostic("Sending state " + format_state(client.approximation) + 
                            " to " + client.player_id + ".");
            client.waiting_for_response = false;
        }
        
        return true;
    }
    
    std::string format_state(const std::vector<double>& state) {
        std::string result;
        for (size_t i = 0; i < state.size(); ++i) {
            if (i > 0) result += " ";
            result += format_double(state[i]);
        }
        return result;
    }
    
    void check_timeouts() {
        time_t now = time(nullptr);
        for (auto it = clients.begin(); it != clients.end();) {
            if (!it->second.got_hello && (now - it->second.connect_time) >= TIMEOUT_HELLO) {
                close(it->first);
                it = clients.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void send_scoring() {
        std::vector<std::pair<std::string, double>> results;
        
        for (const auto& [fd, client] : clients) {
            if (!client.got_coeff) continue;
            
            double score = client.penalty;
            for (int x = 0; x <= config.K; ++x) {
                double expected = evaluate_polynomial(client.coefficients, x);
                double diff = client.approximation[x] - expected;
                score += diff * diff;
            }
            
            results.emplace_back(client.player_id, score);
        }
        
        std::sort(results.begin(), results.end());
        
        std::string scoring_msg = "SCORING";
        for (const auto& [player_id, score] : results) {
            scoring_msg += " " + player_id + " " + format_double(score);
        }
        
        print_diagnostic("Game end, scoring: " + scoring_msg.substr(8) + ".");
        
        for (const auto& [fd, client] : clients) {
            send_message(fd, scoring_msg);
            close(fd);
        }
    }
    
    double evaluate_polynomial(const std::vector<double>& coeffs, int x) {
        double result = 0;
        double x_power = 1;
        for (double coeff : coeffs) {
            result += coeff * x_power;
            x_power *= x;
        }
        return result;
    }
    
    void reset_game() {
        clients.clear();
        while (!event_queue.empty()) {
            event_queue.pop();
        }
        total_puts = 0;
        sleep(1);
    }
    
    void print_bad_message(const Client& client, const std::string& message) {
        std::string player_id = client.player_id.empty() ? "UNKNOWN" : client.player_id;
        std::cerr << "ERROR: bad message from [" << client.ip_address << "]:" 
                 << client.port << ", " << player_id << ": " << message << std::endl;
    }
};

void print_usage() {
    std::cout << "Usage: approx-server -f file [-p port] [-k value] [-n value] [-m value]" << std::endl;
}

int main(int argc, char* argv[]) {
    ServerConfig config;
    config.port = 0;
    config.K = 100;
    config.N = 4;
    config.M = 131;
    config.coefficients_file = "";
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            if (!parse_int(argv[++i], &config.port) || config.port < 0 || config.port > 65535) {
                error_exit("Invalid port number");
            }
        } else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            if (!parse_int(argv[++i], &config.K) || config.K < 1 || config.K > 10000) {
                error_exit("Invalid K value");
            }
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            if (!parse_int(argv[++i], &config.N) || config.N < 1 || config.N > 8) {
                error_exit("Invalid N value");
            }
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            if (!parse_int(argv[++i], &config.M) || config.M < 1 || config.M > 12341234) {
                error_exit("Invalid M value");
            }
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            config.coefficients_file = argv[++i];
        } else {
            error_exit("Invalid command line arguments");
        }
    }
    
    if (config.coefficients_file.empty()) {
        error_exit("Coefficients file (-f) is required");
    }
    
    signal(SIGPIPE, SIG_IGN);
    
    ApproxServer server(config);
    server.start();
    
    return 0;
}
