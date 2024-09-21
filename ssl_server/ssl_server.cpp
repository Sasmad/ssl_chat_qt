/**
 * @file ssl_server.cpp
 * @brief Реализация серверной части SSL-чата с использованием Boost.Asio и Boost.Beast.
 */

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <algorithm>
#include <pqxx/pqxx>
#include <stdexcept>
#include "scipher.h"
#include "ssl_server.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace ip = asio::ip;
namespace ssl = asio::ssl;

DatabaseManager db("host=hse-server.tw1.ru dbname=chat_db user=main password=w^fw&*U267");
std::vector<Client> clients;
std::mutex clients_mutex;

int main() {
    try {
        asio::io_context ioc;
        ssl::context ssl_context(ssl::context::tlsv12_server);
        ssl_context.use_certificate_chain_file("../server.pem");
        ssl_context.use_private_key_file("../server.key", ssl::context::pem);

        tcp::acceptor acceptor(ioc, {tcp::v4(), 3202});
        while (true) {
            auto socket = std::make_shared<ssl_socket>(ioc, ssl_context);
            acceptor.accept(socket->next_layer());
            socket->handshake(ssl::stream_base::server);
            std::thread(&handle_session, socket).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Main exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void send_chat_history(const std::shared_ptr<ssl_socket>& socket, const std::string& clientName) {
    auto messages = db.fetch("SELECT name, message FROM messages");
    for (const auto& row : messages) {
        std::string name = row[0].as<std::string>();
        std::string message = row[1].as<std::string>();
        std::string full_message = "";

        if (name == clientName) {
            name = "You";
        }

        if (name == "") {
            full_message = message;
        } else {
            full_message = name + ": " + message;
        }

        http::response<http::string_body> response(http::status::ok, 11);
        response.set(http::field::content_type, "text/plain");
        response.body() = full_message;
        response.prepare_payload();
        http::write(*socket, response);
    }
}

void broadcast_message(const std::string& message, const std::string& senderName, const std::string& type) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto& client : clients) {
        if (client.name != senderName) {
            http::response<http::string_body> response(http::status::ok, 11);
            response.set(http::field::server, "Boost.Beast");
            response.set(http::field::content_type, "text/plain");
            response.keep_alive(true);

            if (type == "connect") {
                response.body() = "[+]\tNew client '" + senderName + "' connected.";
            } else if (type == "disconnect") {
                response.body() = "[-]\tClient '" + senderName + "' disconnected.";
            } else if (type == "common") {
                response.body() = senderName + ": " + message;
            }

            response.prepare_payload();

            try {
                http::write(*client.socket, response);
            } catch (const std::exception& e) {
                std::cerr << "Failed to send message: " << e.what() << std::endl;
            }
        }
    }
}

void extractLoginAndPassword(const std::string& input, std::string& login, std::string& password) {
    std::istringstream iss(input);
    iss >> login >> password;
}

void handle_session(std::shared_ptr<ssl_socket> socket) {
    std::string login, password;

    beast::flat_buffer buffer;
    http::request<http::string_body> request;

    // Read the first request which is expected to be the client's name
    http::read(*socket, buffer, request);

    std::string target_url = request.target().to_string();

    if (target_url == "/reg") {
        extractLoginAndPassword(request.body(), login, password);
        buffer.consume(buffer.size());
        auto isExist = db.execute("SELECT EXISTS (SELECT 1 FROM users WHERE name ='" + login + "');");
        if (login.empty() || login == "You" || (!isExist.empty() && isExist[0][0].as<bool>())) {
                http::response<http::string_body> response(http::status::ok, 11);
                response.set(http::field::content_type, "text/plain");
                response.body() = "Выберите другое имя пользователя!";
                response.prepare_payload();
                http::write(*socket, response);
        } else {
                db.execute("INSERT INTO users (name, password) VALUES ('" + login + "', '" + password + "')");
                http::response<http::string_body> response(http::status::ok, 11);
                response.set(http::field::content_type, "text/plain");
                response.body() = "Ok";
                response.prepare_payload();
                http::write(*socket, response);
        }
    } else {
        try {
                extractLoginAndPassword(request.body(), login, password);
                buffer.consume(buffer.size());

                auto isExist = db.execute("SELECT EXISTS (SELECT 1 FROM users WHERE name ='" + login + "' and password='" + password + "');");

                if (login.empty() || !(!isExist.empty() && isExist[0][0].as<bool>())) {
                        http::response<http::string_body> response(http::status::bad_request, 11);
                        response.set(http::field::content_type, "text/plain");
                        response.body() = "Invalid username.";
                        response.prepare_payload();
                        http::write(*socket, response);
                        return;
                }

                clients.push_back({socket, login});
                db.execute("INSERT INTO messages (name, message) VALUES ('', '[+]\tNew client " + login + " connected.')");
                send_chat_history(socket, login);
                std::cout << "New client " << login << " connected.\n";
                broadcast_message("", login, "connect");

                while (true) {
                        http::request<http::string_body> request;  // Создаем новый объект запроса для каждого сообщения
                        http::read(*socket, buffer, request);
                        std::string message = request.body();
                        db.execute("INSERT INTO messages (name, message) VALUES ('" + login + "', '" + message + "')");
                        std::cout << "Received message from " << login << ": " << message << std::endl;

                        broadcast_message(message, login, "common");
                        buffer.consume(buffer.size());  // Очищаем буфер после каждого чтения
                }
        } catch (const beast::system_error& e) {
                if (e.code() != beast::errc::not_connected) {
                        std::cerr << "Error in session: " << e.what() << std::endl;
                }

                // Handle client disconnection
                db.execute("INSERT INTO messages (name, message) VALUES ('', '[-]\tClient " + login + " disconnected.')");
                broadcast_message("", login, "disconnect");
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.erase(std::remove_if(clients.begin(), clients.end(),
                [&socket](const Client& c) { return c.socket == socket; }), clients.end());
                std::cout << "Client disconnected: " << login << std::endl;
        }
    }
}