#include "common.h"

using namespace common;

struct ClientConfig {
    std::string player_id;
    std::string server_address;
    int port;
    bool force_ipv4;
    bool force_ipv6;
    bool automatic_mode;
};

class ApproxClient {
private:
    ClientConfig config;
    int client_socket;
    std::vector<double> coefficients;
    std::vector<double> current_state;
    bool got_coefficients;
    bool waiting_for_response;
    bool game_ended;
    std::queue<std::pair<int, double>> pending_puts;
    
public:
    ApproxClient(const ClientConfig& cfg) : config(cfg), got_coefficients(false), 
                                           waiting_for_response(false), game_ended(false) {}
    
    void start() {
        connect_to_server();
        send_hello();
        
        if (config.automatic_mode) {
            run_automatic_mode();
        } else {
            run_manual_mode();
        }
    }
    
private:
    void connect_to_server() {
        struct addrinfo hints, *result;
        memset(&hints, 0, sizeof(hints));
        
        if (config.force_ipv4) {
            hints.ai_family = AF_INET;
        } else if (config.force_ipv6) {
            hints.ai_family = AF_INET6;
        } else {
            hints.ai_family = AF_UNSPEC;
        }
        
        hints.ai_socktype = SOCK_STREAM;
        
        int status = getaddrinfo(config.server_address.c_str(), 
                                std::to_string(config.port).c_str(), &hints, &result);
        if (status != 0) {
            error_exit("Cannot resolve server address: " + std::string(gai_strerror(status)));
        }
        
        client_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (client_socket < 0) {
            freeaddrinfo(result);
            error_exit("Cannot create client socket");
        }
        
        if (connect(client_socket, result->ai_addr, result->ai_addrlen) < 0) {
            close(client_socket);
            freeaddrinfo(result);
            error_exit("Cannot connect to server");
        }
        
        char ip_str[INET6_ADDRSTRLEN];
        if (result->ai_family == AF_INET) {
            struct sockaddr_in* addr_in = (struct sockaddr_in*)result->ai_addr;
            inet_ntop(AF_INET, &addr_in->sin_addr, ip_str, INET_ADDRSTRLEN);
        } else {
            struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)result->ai_addr;
            inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip_str, INET6_ADDRSTRLEN);
        }
        
        print_diagnostic("Connected to [" + std::string(ip_str) + "]:" + std::to_string(config.port) + ".");
        
        freeaddrinfo(result);
    }
    
    void send_hello() {
        send_message(client_socket, "HELLO " + config.player_id);
        
        std::string response;
        if (!receive_message(client_socket, response)) {
            error_exit("unexpected server disconnect");
        }
        
        std::istringstream iss(response);
        std::string command;
        iss >> command;
        
        if (command != MSG_COEFF) {
            std::cerr << "ERROR: bad message from server: " << response << std::endl;
            close(client_socket);
            exit(1);
        }
        
        coefficients.clear();
        double coeff;
        while (iss >> coeff) {
            coefficients.push_back(coeff);
        }
        
        got_coefficients = true;
        
        std::string coeff_str;
        for (size_t i = 0; i < coefficients.size(); ++i) {
            if (i > 0) coeff_str += " ";
            coeff_str += format_double(coefficients[i]);
        }
        print_diagnostic("Received coefficients " + coeff_str + ".");
        
        // Process any pending puts from manual input
        while (!pending_puts.empty() && !waiting_for_response) {
            auto [point, value] = pending_puts.front();
            pending_puts.pop();
            send_put(point, value);
        }
    }
    
    void send_put(int point, double value) {
        if (!got_coefficients) {
            pending_puts.push({point, value});
            return;
        }
        
        if (game_ended) {
            // Game has ended, don't send PUT messages
            return;
        }
        
        // In manual mode, send PUT immediately regardless of waiting_for_response
        // In automatic mode, queue PUTs when waiting for response to avoid PENALTY
        if (!config.automatic_mode || !waiting_for_response) {
            std::string put_msg = "PUT " + std::to_string(point) + " " + format_double(value);
            send_message(client_socket, put_msg);
            waiting_for_response = true;
            
            print_diagnostic("Putting " + format_double(value) + " in " + std::to_string(point) + ".");
        } else {
            // Only queue in automatic mode when waiting for response
            pending_puts.push({point, value});
        }
    }
    
    void handle_server_response(const std::string& response) {
        std::istringstream iss(response);
        std::string command;
        iss >> command;
        
        if (command == "STATE") {
            current_state.clear();
            double value;
            while (iss >> value) {
                current_state.push_back(value);
            }
            
            std::string state_str;
            for (size_t i = 0; i < current_state.size(); ++i) {
                if (i > 0) state_str += " ";
                state_str += format_double(current_state[i]);
            }
            print_diagnostic("Received state " + state_str + ".");
            
        } else if (command == "BAD_PUT" || command == "PENALTY") {
            // Just print the response for now
            print_diagnostic("Received: " + response);
            
        } else if (command == "SCORING") {
            print_diagnostic("Game end, scoring: " + response.substr(8) + ".");
            game_ended = true;
            // Don't exit immediately - continue processing stdin for validation
        }
        
        waiting_for_response = false;
        
        // After receiving any response, try to send pending PUTs
        while (!pending_puts.empty() && !waiting_for_response && !game_ended) {
            auto [point, value] = pending_puts.front();
            pending_puts.pop();
            
            std::string put_msg = "PUT " + std::to_string(point) + " " + format_double(value);
            send_message(client_socket, put_msg);
            waiting_for_response = true;
            
            print_diagnostic("Putting " + format_double(value) + " in " + std::to_string(point) + ".");
            break; // Send only one PUT at a time
        }
    }
    
    void run_manual_mode() {
        fd_set read_fds;
        int max_fd = std::max(client_socket, STDIN_FILENO);
        
        while (true) {
            FD_ZERO(&read_fds);
            
            // Only monitor client socket if game hasn't ended
            if (!game_ended) {
                FD_SET(client_socket, &read_fds);
            }
            FD_SET(STDIN_FILENO, &read_fds);
            
            int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
            
            if (activity > 0) {
                if (!game_ended && FD_ISSET(client_socket, &read_fds)) {
                    std::string response;
                    if (!receive_message(client_socket, response)) {
                        error_exit("unexpected server disconnect");
                    }
                    handle_server_response(response);
                }
                
                if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                    std::string line;
                    if (std::getline(std::cin, line)) {
                        handle_input_line(line);
                    } else {
                        // EOF reached on stdin
                        if (game_ended) {
                            close(client_socket);
                            exit(0);
                        }
                        // If game hasn't ended, continue waiting for server messages
                    }
                }
            }
        }
    }
    
    void handle_input_line(const std::string& line) {
        // Check for empty line or only whitespace
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
            std::cerr << "ERROR: invalid input line " << line << std::endl;
            return;
        }
        
        // Check for leading/trailing whitespace or multiple consecutive spaces
        if (line[0] == ' ' || line.back() == ' ') {
            std::cerr << "ERROR: invalid input line " << line << std::endl;
            return;
        }
        
        // Find the space separator
        size_t space_pos = line.find(' ');
        if (space_pos == std::string::npos) {
            std::cerr << "ERROR: invalid input line " << line << std::endl;
            return;
        }
        
        // Check for multiple consecutive spaces or space at end
        if (line.find("  ") != std::string::npos) {
            std::cerr << "ERROR: invalid input line " << line << std::endl;
            return;
        }
        
        // Split into point and value strings
        std::string point_str = line.substr(0, space_pos);
        std::string value_str = line.substr(space_pos + 1);
        
        // Check for empty parts
        if (point_str.empty() || value_str.empty()) {
            std::cerr << "ERROR: invalid input line " << line << std::endl;
            return;
        }
        
        // Check for plus signs (not allowed)
        if (point_str.find('+') != std::string::npos || value_str.find('+') != std::string::npos) {
            std::cerr << "ERROR: invalid input line " << line << std::endl;
            return;
        }
        
        // Check for commas (decimal separator should be dot)
        if (point_str.find(',') != std::string::npos || value_str.find(',') != std::string::npos) {
            std::cerr << "ERROR: invalid input line " << line << std::endl;
            return;
        }
        
        int point;
        double value;
        
        // Parse and validate
        if (!parse_int(point_str, &point) || !parse_double(value_str, &value)) {
            std::cerr << "ERROR: invalid input line " << line << std::endl;
            return;
        }
        
        // Check for excessive decimal precision (more than 7 digits after decimal point)
        size_t dot_pos = value_str.find('.');
        if (dot_pos != std::string::npos) {
            std::string decimal_part = value_str.substr(dot_pos + 1);
            if (decimal_part.length() > 7) {
                std::cerr << "ERROR: invalid input line " << line << std::endl;
                return;
            }
        }
        
        send_put(point, value);
    }
    
    void run_automatic_mode() {
        // Start automatic strategy after getting coefficients
        if (got_coefficients && !waiting_for_response && !game_ended) {
            auto [point, value] = get_next_move();
            send_put(point, value);
        }
        
        // Main loop: wait for server responses and send next moves
        while (!game_ended) {
            std::string response;
            if (!receive_message(client_socket, response)) {
                error_exit("unexpected server disconnect");
            }
            handle_server_response(response);
            
            // After each response, send next move if game hasn't ended
            if (got_coefficients && !waiting_for_response && !game_ended) {
                auto [point, value] = get_next_move();
                send_put(point, value);
            }
        }
    }
    
    std::pair<int, double> get_next_move() {
        // Simple strategy: try to approximate the polynomial values
        // Find the point with the largest error
        int K = current_state.size() - 1;
        int best_point = 0;
        double max_error = 0;
        
        for (int x = 0; x <= K; ++x) {
            double expected = evaluate_polynomial(coefficients, x);
            double current = (static_cast<size_t>(x) < current_state.size()) ? current_state[x] : 0.0;
            double error = std::abs(expected - current);
            
            if (error > max_error) {
                max_error = error;
                best_point = x;
            }
        }
        
        // Calculate how much to add
        double expected = evaluate_polynomial(coefficients, best_point);
        double current = (static_cast<size_t>(best_point) < current_state.size()) ? current_state[best_point] : 0.0;
        double needed = expected - current;
        
        // Clamp to allowed range [-5, 5]
        double value = std::max(-5.0, std::min(5.0, needed));
        
        return {best_point, value};
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
};

void print_usage() {
    std::cout << "Usage: approx-client -u player_id -s server -p port [-4|-6] [-a]" << std::endl;
}

int main(int argc, char* argv[]) {
    ClientConfig config;
    config.force_ipv4 = false;
    config.force_ipv6 = false;
    config.automatic_mode = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) {
            config.player_id = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            config.server_address = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            if (!parse_int(argv[++i], &config.port) || config.port < 1 || config.port > 65535) {
                error_exit("Invalid port number");
            }
        } else if (strcmp(argv[i], "-4") == 0) {
            config.force_ipv4 = true;
        } else if (strcmp(argv[i], "-6") == 0) {
            config.force_ipv6 = true;
        } else if (strcmp(argv[i], "-a") == 0) {
            config.automatic_mode = true;
        } else {
            error_exit("Invalid command line arguments");
        }
    }
    
    if (config.player_id.empty() || config.server_address.empty() || config.port == 0) {
        error_exit("Missing required arguments: -u, -s, and -p are required");
    }
    
    if (config.force_ipv4 && config.force_ipv6) {
        error_exit("Cannot specify both -4 and -6");
    }
    
    // Validate player_id
    for (char c : config.player_id) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
            error_exit("Invalid player ID format");
        }
    }
    
    ApproxClient client(config);
    client.start();
    
    return 0;
}
