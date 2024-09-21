/**
 * @file mainwindow.cpp
 * @brief Реализация главного окна для SSL-чата с сервером и визуальной частью на Qt.
 */

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <functional>
#include <thread>
#include <QInputDialog>
#include <QDir>
#include <QMessageBox>
#include <QTimer>

#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCryptographicHash>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
using tcp = asio::ip::tcp;
using ssl_stream = ssl::stream<tcp::socket>;
using MessageHandler = std::function<void(const std::string&)>;

QString stringToMD5(const QString &input) {
    QCryptographicHash hash(QCryptographicHash::Md5);

    hash.addData(input.toUtf8());

    return hash.result().toHex();
}

bool containsSpace(const std::string& str) {
    return str.find(' ') != std::string::npos;
}

bool containsUpperCase(const QString& str) {
    foreach (QChar c, str) {
        if (c.isUpper()) {
            return true;
        }
    }
    return false;
}

bool containsDigit(const QString& str) {
    foreach (QChar c, str) {
        if (c.isDigit()) {
            return true;
        }
    }
    return false;
}

bool containsSpecialCharacter(const QString& str) {
    const QString specialChars = "!@#$%^&*()_-+=[]{}|;:',.<>/?";
    foreach (QChar c, str) {
        if (specialChars.contains(c)) {
            return true;
        }
    }
    return false;
}

class RegistrationDialog : public QDialog {
public:
    std::unique_ptr<HttpClient> client;

    explicit RegistrationDialog(QWidget *parent = nullptr) 
    : QDialog(parent),
      usernameLineEdit(new QLineEdit(this)),
      passwordLineEdit(new QLineEdit(this)),
      confirmPasswordLineEdit(new QLineEdit(this)) 
    {
        this->setWindowTitle(tr("Регистрация пользователя"));

        QFormLayout *layout = new QFormLayout(this);

        passwordLineEdit->setEchoMode(QLineEdit::Password);
        confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);

        layout->addRow(tr("Логин:"), usernameLineEdit);
        layout->addRow(tr("Пароль:"), passwordLineEdit);
        layout->addRow(tr("Подтвердите пароль:"), confirmPasswordLineEdit);

        QPushButton *registerButton = new QPushButton(tr("Зарегистрироваться"));
        QPushButton *cancelButton = new QPushButton(tr("Отмена"));

        layout->addRow(registerButton, cancelButton);

        connect(registerButton, &QPushButton::clicked, this, &RegistrationDialog::registerUser);
        connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    }


private:
    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QLineEdit *confirmPasswordLineEdit;

    void registerUser() {
        QString username = usernameLineEdit->text();
        QString password = passwordLineEdit->text();
        QString confirmPassword = confirmPasswordLineEdit->text();

        if (password != confirmPassword) {
            QMessageBox::warning(this, tr("Ошибка регистрации"), tr("Пароли не совпадают."));
            return;
        }
        else if (username.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, tr("Ошибка регистрации"), tr("Логин и пароль не должны быть пустыми."));
            return;
        }
        else if (containsSpace(username.toStdString()) && containsSpace(password.toStdString())) {
            QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Логин и пароль не должны содержать пробелов."));
            return;
        } 
        else if (containsSpace(username.toStdString())) {
            QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Логин не должен содержать пробелов."));
            return;
        } 
        else if (containsSpace(password.toStdString())) {
            QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Пароль не должен содержать пробелов."));
            return;
        } 
        else if (username.isEmpty() && password.isEmpty()) {
            QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Логин и пароль не должны быть пустыми."));
            return;
        } 
        else if (username.isEmpty()) {
            QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Логин не должен быть пустым."));
            return;
        } 
        else if (password.isEmpty()) {
            QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Пароль не должен быть пустым."));
            return;
        }
        else if (!containsUpperCase(password)) {
            QMessageBox::warning(this, tr("Ошибка регистрации"), tr("Пароль должен содержать хотя бы одну заглавную букву."));
            return;
        }
        else if (!containsDigit(password)) {
            QMessageBox::warning(this, tr("Ошибка регистрации"), tr("Пароль должен содержать хотя бы одну цифру."));
            return;
        }
        else if (!containsSpecialCharacter(password)) {
            QMessageBox::warning(this, tr("Ошибка регистрации"), tr("Пароль должен содержать хотя бы один специальный символ."));
            return;
        }

        client = std::make_unique<HttpClient>("185.178.45.18", "3202", username.toStdString(), password.toStdString(),
                                     [this](const std::string& message) {
                                        QMetaObject::invokeMethod(this, [this, message]() {
                                            if (message == "Ok") {
                                                QMessageBox::information(this, tr("Регистрация"), tr("Пользователь успешно зарегистрирован."));
                                                accept();
                                            } else {
                                                QMessageBox::information(this, tr("Регистрация"), QString::fromStdString(message));
                                            }

                                        }, Qt::QueuedConnection);
                                     }, "/reg");
    }
};

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent),
    usernameLineEdit(new QLineEdit(this)),
    passwordLineEdit(new QLineEdit(this)) {
    this->setWindowTitle(tr("Вход в систему"));

    QFormLayout *layout = new QFormLayout(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    layout->addRow(tr("Логин:"), usernameLineEdit);
    layout->addRow(tr("Пароль:"), passwordLineEdit);

    QPushButton *loginButton = new QPushButton(tr("Войти"));
    QPushButton *registerButton = new QPushButton(tr("Зарегистрироваться"));
    QPushButton *cancelButton = new QPushButton(tr("Отмена"));

    layout->addRow(loginButton, registerButton);
    layout->addRow(cancelButton);

    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::validateInput);
    connect(registerButton, &QPushButton::clicked, this, &LoginDialog::openRegistrationDialog);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString LoginDialog::getUsername() const {
    return usernameLineEdit->text();
}

QString LoginDialog::getPassword() const {
    return passwordLineEdit->text();
}

void LoginDialog::openRegistrationDialog() {
    RegistrationDialog regDialog(this);
    regDialog.exec();
}

void LoginDialog::validateInput() {
    if (containsSpace(usernameLineEdit->text().toStdString()) && containsSpace(passwordLineEdit->text().toStdString())) {
        QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Логин и пароль не должны содержать пробелов."));
        return;
    }
    else if (containsSpace(usernameLineEdit->text().toStdString())) {
        QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Логин не должен содержать пробелов."));
        return;
    } 
    else if (containsSpace(passwordLineEdit->text().toStdString())) {
        QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Пароль не должен содержать пробелов."));
        return;
    } 
    else if (usernameLineEdit->text().isEmpty() && passwordLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Логин и пароль не должны быть пустыми."));
        return;
    } 
    else if (usernameLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Логин не должен быть пустым."));
        return;
    } 
    else if (passwordLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Недопустимый ввод"), tr("Пароль не должен быть пустым."));
        return;
    } 
    else {
        accept();
    }
}

HttpClient::HttpClient(const std::string& host, const std::string& port, const std::string& uname, const std::string& upass, MessageHandler handler, const std::string& path)
    : ssl_context(ssl::context::tlsv12_client), stream(ioc, ssl_context), messageHandler(handler) {
    ssl_context.set_default_verify_paths();
    ssl_context.set_verify_mode(ssl::verify_peer);

    tcp::resolver resolver(ioc);
    auto const results = resolver.resolve(host, port);
    asio::connect(stream.next_layer(), results.begin(), results.end());
    stream.handshake(ssl_stream::client);
    sendRequest(uname + " " + stringToMD5(QString::fromStdString(upass)).toStdString(), path);
    startListening();
    std::thread([this]() { ioc.run(); }).detach(); // Запускаем io_context в отдельном потоке
}

HttpClient::~HttpClient() {
    try {
        stream.shutdown();
        stream.next_layer().close();
    } catch (std::exception const& e) {
        std::cerr << "Error closing socket: " << e.what() << std::endl;
    }
    ioc.stop();
}

void HttpClient::sendRequest(const std::string& message, const std::string& path) {
    http::request<http::string_body> req(http::verb::post, path, 11);
    req.set(http::field::host, "185.178.45.18");
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.body() = message;
    req.prepare_payload();
    http::write(stream, req);
}

void HttpClient::startListening() {
    http::async_read(stream, buffer, res,
                     [this](beast::error_code ec, std::size_t bytes_transferred) {
                         if (!ec) {
                             messageHandler(res.body());
                             res = {}; // Очищаем ответ для следующего чтения
                             startListening(); // Слушаем следующее сообщение
                         } else {
                             std::cerr << "Read error: " << ec.message() << std::endl;
                         }
                     });
}

/**
 * @brief Конструктор главного окна.
 * @param parent Родительский виджет.
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Отображаем диалог входа
    LoginDialog loginDialog(this);

    if (loginDialog.exec() == QDialog::Accepted) {
        // Пользователь успешно вошел, получаем имя пользователя и пароль
        QString username = loginDialog.getUsername();
        QString password = loginDialog.getPassword();

        // Инициализация клиента с использованием полученных данных
        client = std::make_unique<HttpClient>("185.178.45.18", "3202",
                                               username.toStdString(), password.toStdString(),
                                               [this](const std::string& message) {
                                                    QMetaObject::invokeMethod(this, "updateUI", Qt::QueuedConnection,
                                                                Q_ARG(QString, QString::fromStdString(message)));
                                               }
        );

        connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
    } else {
        // Если диалог был отменен или не успешен
        QMessageBox::warning(this, tr("Ошибка входа"), tr("Вход в систему не выполнен."));
        QTimer::singleShot(0, this, &QWidget::close); // Запланировать закрытие окна
    }
}

/**
 * @brief Деструктор главного окна.
 */
MainWindow::~MainWindow() {
    delete ui;
}

/**
 * @brief Обновление пользовательского интерфейса в зависимости от сообщения.
 * @param message Сообщение для отображения.
 */
void MainWindow::updateUI(QString message) {
    if (message == "Invalid username.") {
        QMessageBox::warning(this, tr("Ошибка доступа"), tr("Ошибка в учетных данных. Зарегистрируйтесь при сдующем входе!"));
        //QMessageBox::warning(this, tr("Ошибка доступа"), tr("قضيب"));
        QTimer::singleShot(0, this, &QWidget::close); // Запланировать закрытие окна
    } else if (message == "Username already taken.") {
        QMessageBox::warning(this, tr("Ошибка доступа"), tr("Имя пользователя уже занято."));
        QTimer::singleShot(0, this, &QWidget::close); // Запланировать закрытие окна
    } else {
        ui->textBrowser->append(message);
    }
}

/**
 * @brief Обработчик нажатия кнопки отправки сообщения.
 */
void MainWindow::onButtonClicked() {
    QString text = ui->textEdit->toPlainText();
    if (!text.isEmpty()) {
        client->sendRequest(text.toStdString());
        ui->textBrowser->append("You: " + text);
        ui->textEdit->clear();
    }
}
