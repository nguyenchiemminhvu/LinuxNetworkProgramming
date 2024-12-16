#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

// Forward declarations
class HTTPRequest;
class HTTPResponse;
class Router;

// HTTPParser Class
class HTTPParser {
public:
    HTTPRequest parse(const std::string& raw_request);
};

// HTTPRequest Class
class HTTPRequest {
public:
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    HTTPRequest() = default;
};

// HTTPResponse Class
class HTTPResponse {
private:
    int status_code;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

public:
    HTTPResponse(int code = 200, const std::string& body = "")
        : status_code(code), body(body) {}

    void setHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    std::string toString() const {
        std::ostringstream response;
        response << "HTTP/1.1 " << status_code << " " << getStatusMessage() << "\r\n";
        for (const auto& header : headers) {
            response << header.first << ": " << header.second << "\r\n";
        }
        response << "Content-Length: " << body.size() << "\r\n";
        response << "\r\n" << body;
        return response.str();
    }

private:
    std::string getStatusMessage() const {
        switch (status_code) {
            case 200: return "OK";
            case 404: return "Not Found";
            case 500: return "Internal Server Error";
            default:  return "Unknown";
        }
    }
};

// Router Class
class Router {
public:
    HTTPResponse route(const HTTPRequest& request) {
        if (request.path == "/") {
            return HTTPResponse(200, "Welcome to the Home Page!");
        } else if (request.path == "/hello") {
            return HTTPResponse(200, "Hello, World!");
        } else {
            return HTTPResponse(404, "Page Not Found.");
        }
    }
};

// ConnectionHandler Class
class ConnectionHandler {
private:
    int client_socket;
    HTTPParser parser;
    Router router;

public:
    explicit ConnectionHandler(int client_socket) : client_socket(client_socket) {}

    void handle() {
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));

        // Read request
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            close(client_socket);
            return;
        }

        std::string raw_request(buffer);
        HTTPRequest request = parser.parse(raw_request);

        // Route request and generate response
        HTTPResponse response = router.route(request);

        // Send response
        std::string response_str = response.toString();
        send(client_socket, response_str.c_str(), response_str.size(), 0);

        // Close connection
        close(client_socket);
    }
};

// Server Class
class Server {
private:
    int server_socket;
    int port;

public:
    explicit Server(int port) : port(port), server_socket(-1) {}

    void start() {
        setupSocket();
        bindSocket();
        listenSocket();
        acceptConnections();
    }

private:
    void setupSocket() {
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }
    }

    void bindSocket() {
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            perror("Socket bind failed");
            exit(EXIT_FAILURE);
        }
    }

    void listenSocket() {
        if (listen(server_socket, 10) == -1) {
            perror("Socket listen failed");
            exit(EXIT_FAILURE);
        }
        std::cout << "Server is running on port " << port << std::endl;
    }

    void acceptConnections() {
        while (true) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
            if (client_socket == -1) {
                perror("Failed to accept connection");
                continue;
            }
            ConnectionHandler handler(client_socket);
            handler.handle();
        }
    }
};

// HTTPParser Class Implementation
HTTPRequest HTTPParser::parse(const std::string& raw_request) {
    HTTPRequest request;
    std::istringstream request_stream(raw_request);

    // Parse request line
    std::string request_line;
    std::getline(request_stream, request_line);
    std::istringstream request_line_stream(request_line);
    request_line_stream >> request.method >> request.path;

    // Parse headers
    std::string header_line;
    while (std::getline(request_stream, header_line) && header_line != "\r") {
        size_t colon_pos = header_line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = header_line.substr(0, colon_pos);
            std::string value = header_line.substr(colon_pos + 1);
            if (!value.empty() && value[0] == ' ') value.erase(0, 1);
            request.headers[key] = value;
        }
    }

    // Parse body
    if (request.headers.count("Content-Length") > 0) {
        size_t content_length = std::stoi(request.headers["Content-Length"]);
        request.body.resize(content_length);
        request_stream.read(&request.body[0], content_length);
    }

    return request;
}

// Application Class
class Application {
public:
    void run(int port) {
        Server server(port);
        server.start();
    }
};

// Main Function
int main() {
    Application app;
    app.run(8080); // Start the server on port 8080
    return 0;
}