#include "mainwindow.h"
#include <gtest/gtest.h>
#include <QString>
#include <QCryptographicHash>
#include "gmock/gmock.h"
#include <boost/asio.hpp>
#include <memory>

// Test case for stringToMD5 function
TEST(MD5Test, HandlesEmptyString) {
    EXPECT_EQ(stringToMD5(""), QString(QCryptographicHash::hash("", QCryptographicHash::Md5).toHex()));
}

TEST(MD5Test, HandlesSimpleString) {
    QString input = "hello";
    EXPECT_EQ(stringToMD5(input), QString(QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Md5).toHex()));
}

// Test cases for containsSpace function
TEST(ContainsSpaceTest, HandlesNoSpace) {
    EXPECT_FALSE(containsSpace("HelloWorld"));
}

TEST(ContainsSpaceTest, HandlesSpaceInString) {
    EXPECT_TRUE(containsSpace("Hello World"));
}

// Test cases for containsUpperCase function
TEST(ContainsUpperCaseTest, HandlesNoUpperCase) {
    EXPECT_FALSE(containsUpperCase("hello world"));
}

TEST(ContainsUpperCaseTest, HandlesUpperCase) {
    EXPECT_TRUE(containsUpperCase("Hello world"));
}

// Test cases for containsDigit function
TEST(ContainsDigitTest, HandlesNoDigit) {
    EXPECT_FALSE(containsDigit("Hello World"));
}

TEST(ContainsDigitTest, HandlesDigit) {
    EXPECT_TRUE(containsDigit("Hello2World"));
}

// Test cases for containsSpecialCharacter function
TEST(ContainsSpecialCharacterTest, HandlesNoSpecialCharacter) {
    EXPECT_FALSE(containsSpecialCharacter("HelloWorld"));
}

TEST(ContainsSpecialCharacterTest, HandlesSpecialCharacter) {
    EXPECT_TRUE(containsSpecialCharacter("Hello@World"));
}


using namespace testing;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
using tcp = asio::ip::tcp;

class IStream {
public:
    virtual ~IStream() {}
    virtual void handshake(ssl::stream_base::handshake_type type) = 0;
    virtual void async_read(boost::beast::flat_buffer& buffer,
                            boost::beast::http::response<boost::beast::http::string_body>& res,
                            std::function<void(boost::system::error_code, std::size_t)> handler) = 0;
    virtual void write(const boost::beast::http::request<boost::beast::http::string_body>& req) = 0;
    virtual void shutdown() = 0;
    virtual void close() = 0;
};

class MockStream : public IStream {
public:
    MOCK_METHOD(void, handshake, (ssl::stream_base::handshake_type type), (override));
    MOCK_METHOD(void, async_read, (boost::beast::flat_buffer&, boost::beast::http::response<boost::beast::http::string_body>&, std::function<void(boost::system::error_code, std::size_t)>), (override));
    MOCK_METHOD(void, write, (const boost::beast::http::request<boost::beast::http::string_body>&), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(void, close, (), (override));
};

class MockHandler {
public:
    MOCK_METHOD(void, Call, (const std::string& message));
};

class HttpClientTestable : public HttpClient {
public:
    std::shared_ptr<MockStream> mockStream;

    HttpClientTestable(const std::string& host, const std::string& port, const std::string& uname, const std::string& upass, MessageHandler handler, std::shared_ptr<MockStream> mockStream)
        : HttpClient(host, port, uname, upass, handler), mockStream(mockStream) {}

    void sendRequest(const std::string& message, const std::string& path = "/") override {
        if (!message.empty()) {
            boost::beast::http::request<boost::beast::http::string_body> req;
            req.body() = message;
            req.target(path);
            mockStream->write(req);
        }
    }

    void startListening() override {
        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::string_body> res;
        std::function<void(boost::system::error_code, std::size_t)> handler = [this, &res](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec && bytes_transferred > 0) {
                this->handleMessage(res.body().data());
            } else {
                std::cerr << "Read error: " << ec.message() << std::endl;
            }
        };
        mockStream->async_read(buffer, res, handler);
    }

    ~HttpClientTestable() {
        mockStream->shutdown();
        mockStream->close();
    }
};

class HttpClientTests : public Test {
protected:
    std::unique_ptr<HttpClientTestable> client;
    std::shared_ptr<MockStream> mockStream;
    MockHandler mockHandler;

    void SetUp() override {
        mockStream = std::make_shared<MockStream>();
        client = std::make_unique<HttpClientTestable>("185.178.45.18", "3202", "main", "1234", std::bind(&MockHandler::Call, &mockHandler, std::placeholders::_1), mockStream);
    }

    void TearDown() override {
        // Так как методы вызываются в деструкторе, проверка идет здесь
        EXPECT_CALL(*mockStream, shutdown()).Times(AtLeast(1));
        EXPECT_CALL(*mockStream, close()).Times(AtLeast(1));
    }
};

TEST_F(HttpClientTests, SendValidRequest) {
    EXPECT_CALL(*mockStream, write(_)).Times(1);
    client->sendRequest("Hello, World!");
}

TEST_F(HttpClientTests, SendEmptyRequest) {
    EXPECT_CALL(*mockStream, write(_)).Times(0);
    client->sendRequest("");
}

TEST_F(HttpClientTests, SuccessfulRead) {
    EXPECT_CALL(*mockStream, async_read(_, _, _))
        .WillOnce(InvokeArgument<2>(boost::system::error_code{}, 1));
    EXPECT_CALL(mockHandler, Call(_)).Times(1);
    client->startListening();
}

TEST_F(HttpClientTests, ReadError) {
    EXPECT_CALL(*mockStream, async_read(_, _, _))
        .WillOnce(InvokeArgument<2>(boost::asio::error::eof, 0));
    client->startListening();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
