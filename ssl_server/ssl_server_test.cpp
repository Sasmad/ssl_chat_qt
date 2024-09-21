#include <gtest/gtest.h>
#include "ssl_server.h"
#include <pqxx/pqxx>                                   

class DatabaseManagerTest : public ::testing::Test {
protected:
    DatabaseManager* dbManager;

    void SetUp() override {
        // Подключение к тестовой базе данных
        dbManager = new DatabaseManager("host=hse-server.tw1.ru dbname=chat_db user=main password=w^fw&*U267");
    }

    void TearDown() override {
        delete dbManager;
    }
};

TEST_F(DatabaseManagerTest, ExecuteInsert) {
    std::string query = "INSERT INTO test (name) VALUES ('test_user');";
    dbManager->execute(query);

    // Проверяем, что данные вставлены
    pqxx::result result = dbManager->fetch("SELECT name FROM test WHERE name = 'test_user';");
    ASSERT_FALSE(result.empty());
    ASSERT_EQ(result[0][0].as<std::string>(), "test_user");
}

TEST_F(DatabaseManagerTest, FetchData) {
    std::string query = "SELECT name FROM test WHERE name = 'test_user';";
    pqxx::result result = dbManager->fetch(query);

    ASSERT_FALSE(result.empty());
    ASSERT_EQ(result[0][0].as<std::string>(), "test_user");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}