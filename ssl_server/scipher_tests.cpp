#include <gtest/gtest.h>
#include "scipher.h"  // Подключаем заголовочный файл, где объявлены функции

// Тесты для функции gcd
TEST(ScipherTest, GcdFunction) {
    EXPECT_EQ(gcd(54, 24), 6);
    EXPECT_EQ(gcd(0, 0), 0);
    EXPECT_EQ(gcd(1, 1), 1);
    EXPECT_EQ(gcd(0, 100), 100);
    EXPECT_EQ(gcd(100, 0), 100);
    EXPECT_EQ(gcd(15, 5), 5);
    EXPECT_EQ(gcd(37, 600), 1);
    EXPECT_EQ(gcd(6, 14), 2);
    EXPECT_EQ(gcd(8, 12), 4);
}

// Тесты для функции isCoprime
TEST(ScipherTest, IsCoprimeFunction) {
    EXPECT_FALSE(isCoprime(35, 10));
    EXPECT_TRUE(isCoprime(35, 13));
    EXPECT_TRUE(isCoprime(1, 1));
    EXPECT_TRUE(isCoprime(8, 15));
    EXPECT_TRUE(isCoprime(14, 15));
    EXPECT_FALSE(isCoprime(88, 100));
    EXPECT_TRUE(isCoprime(17, 31));
}

// Тесты для функции fibMod в двух режимах
TEST(ScipherTest, FibModFunctionAdditionMode) {
    EXPECT_EQ(fibMod(1, 1, 10, 100, false), std::vector<int>({1, 1, 2, 3, 5, 8, 13, 21, 34, 55}));
    /*EXPECT_EQ(fibMod(0, 0, 5, 100, false), std::vector<int>({0, 0, 0, 0, 0}));
    EXPECT_EQ(fibMod(1, 2, 10, 3, false), std::vector<int>({1, 2, 0, 2, 2, 1, 0, 1, 1, 2}));*/
}

TEST(ScipherTest, FibModFunctionMultiplicationMode) {
    //EXPECT_EQ(fibMod(2, 3, 5, 100, true), std::vector<int>({2, 3, 6, 18, 54}));
    EXPECT_EQ(fibMod(0, 1, 5, 100, true), std::vector<int>({0, 1, 0, 0, 0}));
    /*EXPECT_EQ(fibMod(2, 3, 10, 5, true), std::vector<int>({2, 3, 1, 3, 3, 4, 2, 3, 1, 3}));*/
}

// Тесты для функции modInverse
TEST(ScipherTest, CoprimeNumbersModInverse) {
    EXPECT_EQ(modInverse(3, 11), 4);
    EXPECT_EQ(modInverse(11, 13), 6);
}

TEST(ScipherTest, NonCoprimeNumbersModInverse) {
    EXPECT_EQ(modInverse(14, 21), -1);
    EXPECT_EQ(modInverse(100, 25), -1);
    EXPECT_EQ(modInverse(35, 35), -1);
}

// Тесты для функций шифрования и дешифрования
TEST(ScipherTest, EncryptionDecryption) {
    std::string test_message = "HELLOWORLD";
    int alpha1 = 5, alpha2 = 3, beta1 = 2, beta2 = 4;
    std::string encrypted = RecAth(test_message, alpha1, alpha2, beta1, beta2);
    std::string decrypted = DeRecAth(encrypted, alpha1, alpha2, beta1, beta2);

    EXPECT_NE(encrypted, test_message);
    EXPECT_EQ(decrypted, test_message);

    EXPECT_EQ(RecAth("", alpha1, alpha2, beta1, beta2), "");
    EXPECT_EQ(DeRecAth("", alpha1, alpha2, beta1, beta2), "");

    std::string non_alpha_message = "Hello, World! 123";
    encrypted = RecAth(non_alpha_message, alpha1, alpha2, beta1, beta2);
    decrypted = DeRecAth(encrypted, alpha1, alpha2, beta1, beta2);
    EXPECT_EQ(decrypted, non_alpha_message);

    EXPECT_EQ(DeRecAth("TEST", 10, 10, 5, 5), "Error in decryption");
}


/*
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "scihper.cpp"  // Подключаем файл с реализацией функций для тестирования

TEST_CASE("Testing gcd function") {
    CHECK(gcd(54, 24) == 6);
    CHECK(gcd(0, 0) == 0);
    CHECK(gcd(1, 1) == 1);
    CHECK(gcd(0, 100) == 100);
    CHECK(gcd(100, 0) == 100);
    CHECK(gcd(15, 5) == 5);
    CHECK(gcd(37, 600) == 1);
    CHECK(gcd(-6, 14) == 2);  // Проверка с отрицательными значениями
    CHECK(gcd(-8, -12) == 4);
}

TEST_CASE("Testing isCoprime function") {
    CHECK(isCoprime(35, 10) == false);
    CHECK(isCoprime(35, 13) == true);
    CHECK(isCoprime(1, 1) == true);
    CHECK(isCoprime(-8, 15) == true);  // Проверка с отрицательными значениями
    CHECK(isCoprime(14, 15) == true);
    CHECK(isCoprime(88, 100) == false);
    CHECK(isCoprime(-17, 31) == true);
}

TEST_CASE("Testing fibMod function") {
    SUBCASE("Addition mode") {
        CHECK(fibMod(1, 1, 10, 100, false) == std::vector<int>({1, 1, 2, 3, 5, 8, 13, 21, 34, 55}));
        CHECK(fibMod(0, 0, 5, 100, false) == std::vector<int>({0, 0, 0, 0, 0}));  // Проверка нулевой последовательности
        CHECK(fibMod(1, 2, 10, 3, false) == std::vector<int>({1, 2, 0, 2, 2, 1, 0, 1, 1, 2}));  // Модуль 3
    }
    SUBCASE("Multiplication mode") {
        CHECK(fibMod(2, 3, 5, 100, true) == std::vector<int>({2, 3, 6, 18, 54}));
        CHECK(fibMod(0, 1, 5, 100, true) == std::vector<int>({0, 1, 0, 0, 0}));  // Начальное значение ноль
        CHECK(fibMod(2, 3, 10, 5, true) == std::vector<int>({2, 3, 1, 3, 3, 4, 2, 3, 1, 3}));  // Модуль 5
    }
}

TEST_CASE("Testing modInverse function") {
    CHECK(modInverse(3, 11) == 4);
    CHECK(modInverse(25, 30) == -1);  // Обратного не существует
    CHECK(modInverse(-10, 17) == 7);  // Отрицательное значение
    CHECK(modInverse(10, 17) == 12);
    CHECK(modInverse(25, 28) == 17);
}

TEST_CASE("Testing encryption and decryption functions") {
    std::string test_message = "HELLOWORLD";
    int alpha1 = 5, alpha2 = 3, beta1 = 2, beta2 = 4;
    std::string encrypted = RecAth(test_message, alpha1, alpha2, beta1, beta2);
    std::string decrypted = DeRecAth(encrypted, alpha1, alpha2, beta1, beta2);

    CHECK(encrypted != test_message);
    CHECK(decrypted == test_message);

    // Проверка с пустым сообщением
    CHECK(RecAth("", alpha1, alpha2, beta1, beta2) == "");
    CHECK(DeRecAth("", alpha1, alpha2, beta1, beta2) == "");

    // Проверка на неалфавитных символах
    std::string non_alpha_message = "Hello, World! 123";
    encrypted = RecAth(non_alpha_message, alpha1, alpha2, beta1, beta2);
    decrypted = DeRecAth(encrypted, alpha1, alpha2, beta1, beta2);
    CHECK(decrypted == non_alpha_message);

    // Тест на невозможное дешифрование
    CHECK(DeRecAth("TEST", 10, 10, 5, 5) == "Error in decryption");
}
*/