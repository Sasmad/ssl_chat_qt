#ifndef SSL_SERVER_H
#define SSL_SERVER_H

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

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace ip = asio::ip;
namespace ssl = asio::ssl;

/**
 * @brief Тип TCP-сокета.
 */
using tcp = ip::tcp;

/**
 * @brief Тип SSL-сокета на основе TCP.
 */
using ssl_socket = ssl::stream<tcp::socket>;

/**
 * @brief Класс для управления подключением к базе данных PostgreSQL.
 */
class DatabaseManager {
private:
    pqxx::connection conn;

public:
    /**
     * @brief Конструктор класса DatabaseManager.
     * @param conn_str Строка подключения к базе данных.
    */
    DatabaseManager(const std::string& conn_str);

    /**
     * @brief Выполняет запрос к базе данных.
     * @param query SQL-запрос.
    */
    pqxx::result execute(const std::string& query);

    /**
     * @brief Выполняет запрос к базе данных и возвращает результат.
     * @param query SQL-запрос.
     * @return Результат выполнения запроса.
    */
    pqxx::result fetch(const std::string& query);
};

DatabaseManager::DatabaseManager(const std::string& conn_str) : conn(conn_str) {
    if (conn.is_open()) {
        std::cout << "Opened database successfully: " << conn.dbname() << std::endl;
    } else {
        throw std::runtime_error("Can't open database");
    }
}

pqxx::result DatabaseManager::execute(const std::string& query) {
    pqxx::work W(conn);
    pqxx::result result = W.exec(query);
    W.commit();
    return result;
}

pqxx::result DatabaseManager::fetch(const std::string& query) {
    pqxx::nontransaction N(conn);
    return N.exec(query);
}

/**
 * @brief Структура для хранения информации о подключенном клиенте.
 */
struct Client {
    std::shared_ptr<ssl_socket> socket;
    std::string name;
};

extern DatabaseManager db;
extern std::vector<Client> clients;
extern std::mutex clients_mutex;

/**
 * @brief Отправляет историю чата подключенному клиенту.
 * @param socket SSL-сокет клиента.
 * @param clientName Имя клиента.
 */
void send_chat_history(const std::shared_ptr<ssl_socket>& socket, const std::string& clientName);

/**
 * @brief Широковещательная рассылка сообщения всем подключенным клиентам.
 * @param message Сообщение для отправки.
 * @param senderName Имя отправителя.
 * @param type Тип сообщения (connect, disconnect, common).
 */
void broadcast_message(const std::string& message, const std::string& senderName, const std::string& type);

/**
 * @brief Обрабатывает сессию клиента.
 * @param socket SSL-сокет клиента.
 */
void handle_session(std::shared_ptr<ssl_socket> socket);
void extractLoginAndPassword(const std::string& input, std::string& login, std::string& password);

#endif // SSL_SERVER_H