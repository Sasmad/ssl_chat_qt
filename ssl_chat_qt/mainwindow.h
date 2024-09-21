/**
 * @file mainwindow.h
 * @brief Определение класса главного окна для SSL-чата с сервером и визуальной частью на Qt, включая класс HttpClient.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <functional>
#include <string>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl.hpp>

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QMessageBox>

/**
 * @brief Возвращает MD5-хеш входной строки.
 *
 * @param input Строка, для которой необходимо вычислить MD5-хеш.
 * @return QString Возвращает MD5-хеш входной строки.
 */
QString stringToMD5(const QString &input);

/**
 * @brief Проверяет наличие пробелов в строке.
 *
 * @param str Строка для проверки.
 * @return bool Возвращает true, если в строке есть хотя бы один пробел.
 */
bool containsSpace(const std::string& str);

/**
 * @brief Проверяет, содержит ли строка символы в верхнем регистре.
 *
 * @param str Строка для проверки.
 * @return bool Возвращает true, если строка содержит хотя бы один символ в верхнем регистре.
 */
bool containsUpperCase(const QString& str);

/**
 * @brief Проверяет, содержит ли строка цифры.
 *
 * @param str Строка для проверки.
 * @return bool Возвращает true, если строка содержит хотя бы одну цифру.
 */
bool containsDigit(const QString& str);

/**
 * @brief Проверяет, содержит ли строка специальные символы.
 *
 * @param str Строка для проверки.
 * @return bool Возвращает true, если строка содержит хотя бы один специальный символ.
 */
bool containsSpecialCharacter(const QString& str);

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Предварительное объявление MessageHandler
using MessageHandler = std::function<void(const std::string&)>;

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
using tcp = asio::ip::tcp;
using ssl_stream = ssl::stream<tcp::socket>;

/**
 * @class HttpClient
 * @brief Класс для реализации HTTP клиента с использованием SSL.
 *
 * HttpClient используется для отправки и получения сообщений через зашифрованное соединение.
 * Этот класс управляет SSL-соединениями, отправляет HTTP-запросы и асинхронно принимает ответы.
 */
class HttpClient {
public:
    /**
     * @brief Конструктор класса HttpClient.
     * @param host Хост сервера.
     * @param port Порт сервера.
     * @param uname Имя пользователя.
     * @param handler Функция-обработчик сообщений.
     */
    HttpClient(const std::string& host, const std::string& port, const std::string& uname, const std::string& upass, MessageHandler handler, const std::string& path = "/");

    /**
     * @brief Деструктор класса HttpClient.
     */
    ~HttpClient();

    /**
     * @brief Отправка HTTP запроса.
     * @param message Сообщение для отправки на сервер.
     */
    virtual void sendRequest(const std::string& message, const std::string& path = "/");

    /**
     * @brief Начало асинхронного чтения сообщений с сервера.
     */
    virtual void startListening();

    void handleMessage(const std::string& message) {
        messageHandler(message);
    }

private:
    asio::io_context ioc;
    ssl::context ssl_context;
    ssl_stream stream;
    beast::flat_buffer buffer;
    MessageHandler messageHandler;
    http::response<http::string_body> res;
};

/**
 * @class MainWindow
 * @brief Класс главного окна приложения.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    std::unique_ptr<HttpClient> client = nullptr; ///< Уникальный указатель на экземпляр HttpClient, используемый для общения с сервером.
    /**
     * @brief Конструктор главного окна.
     * @param parent Родительский виджет.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор главного окна.
     */
    ~MainWindow();

private slots:
    /**
     * @brief Обновление пользовательского интерфейса с новым сообщением.
     * @param message Сообщение для отображения.
     */
    void updateUI(QString message);

    /**
     * @brief Обработчик нажатия кнопки отправки сообщения.
     */
    void onButtonClicked();

private:
    Ui::MainWindow *ui; ///< Указатель на пользовательский интерфейс.
};

/**
 * @class LoginDialog
 * @brief Класс диалога входа в систему, предоставляющий интерфейс для ввода имени пользователя и пароля.
 *
 * LoginDialog предоставляет диалоговое окно, в котором пользователь может ввести своё имя пользователя и пароль для входа в систему.
 */
class LoginDialog : public QDialog {
public:
    /**
     * @brief Конструктор класса LoginDialog.
     * @param parent Указатель на родительский виджет, по умолчанию nullptr.
     */
    explicit LoginDialog(QWidget *parent = nullptr);

    /**
     * @brief Возвращает имя пользователя, введенное в поле ввода.
     * @return QString Имя пользователя.
     */
    QString getUsername() const;

    /**
     * @brief Возвращает пароль, введенный в поле ввода.
     * @return QString Пароль.
     */
    QString getPassword() const;

private:
    QLineEdit *usernameLineEdit; ///< Поле ввода для имени пользователя.
    QLineEdit *passwordLineEdit; ///< Поле ввода для пароля.

    /**
     * @brief Открывает диалог регистрации нового пользователя.
     */
    void openRegistrationDialog();

    /**
     * @brief Проверяет введенные данные на валидность.
     *
     * Эта функция проверяет, что имя пользователя и пароль не пусты и соответствуют заданным требованиям безопасности.
     */
    void validateInput();
};

#endif // MAINWINDOW_H
