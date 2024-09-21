#include "scipher.h"
#include <iostream>
#include <vector>
#include <string>
#include <tuple>

int gcd(int a, int b) {
    while (b != 0) {
        int temp = a % b;
        a = b;
        b = temp;
    }
    return a;
}

bool isCoprime(int a, int m) {
    return gcd(a, m) == 1;
}

std::vector<int> fibMod(int a, int b, int l, int mod, bool isMult) {
    std::vector<int> result = {a, b};
    while (result.size() < l) {
        int next = isMult ? (result.back() * result[result.size() - 2]) % mod :
                       (result.back() + result[result.size() - 2]) % mod;
        result.push_back(next);
    }
    return result;
}

int modInverse(int a, int m) {
    int m0 = m, t, q;
    int x0 = 0, x1 = 1;
    if (!isCoprime(a, m)) {
        std::cerr << "No modular inverse exists (not coprime with modulus).\n";
        return -1;
    }
    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m, a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    return x1 < 0 ? x1 + m0 : x1;
}

std::string RecAth(const std::string& message, int alpha1, int alpha2, int beta1, int beta2) {
    const std::string BigAlph = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string SmallAlph = "abcdefghijklmnopqrstuvwxyz";
    std::string result;
    std::vector<int> AlphaM = fibMod(alpha1, alpha2, message.length(), 26, true);
    std::vector<int> BetaM = fibMod(beta1, beta2, message.length(), 26, false);

    for (int i = 0; i < message.length(); ++i) {
        char ch = message[i];
        size_t index;
        if ((index = SmallAlph.find(ch)) != std::string::npos) {
            int newID = (index * AlphaM[i] + BetaM[i]) % 26;
            result += SmallAlph[newID];
        } else if ((index = BigAlph.find(ch)) != std::string::npos) {
            int newID = (index * AlphaM[i] + BetaM[i]) % 26;
            result += BigAlph[newID];
        } else {
            result += ch;
        }
    }
    return result;
}

std::string DeRecAth(const std::string& encrypted, int alpha1, int alpha2, int beta1, int beta2) {
    const std::string BigAlph = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string SmallAlph = "abcdefghijklmnopqrstuvwxy";

    std::string decrypted;
    std::vector<int> AlphaM = fibMod(alpha1, alpha2, encrypted.length(), 26, true);
    std::vector<int> BetaM = fibMod(beta1, beta2, encrypted.length(), 26, false);

    for (int i = 0; i < encrypted.length(); ++i) {
        char ch = encrypted[i];
        size_t index;
        int invAlphaM = modInverse(AlphaM[i], 26);
        if (invAlphaM == -1) return "Error in decryption"; // Обработка ошибки

        if ((index = SmallAlph.find(ch)) != std::string::npos) {
            int newID = ((index - BetaM[i] + 26) * invAlphaM) % 26;
            decrypted += SmallAlph[newID];
        } else if ((index = BigAlph.find(ch)) != std::string::npos) {
            int newID = ((index - BetaM[i] + 26) * invAlphaM) % 26;
            decrypted += BigAlph[newID];
        } else {
            decrypted += ch;
        }
    }
    return decrypted;
}
/*
int main() {
    std::string message;
    int alpha1, alpha2, beta1, beta2; // (3, 5) (6, 7)

    std::cout << "Enter message to encrypt: ";
    getline(std::cin, message);
    std::cout << "Enter alpha1, alpha2 (coprime with 26), beta1, beta2: ";
    std::cin >> alpha1 >> alpha2 >> beta1 >> beta2;

    if (!isCoprime(alpha1, 26) || !isCoprime(alpha2, 26)) {
        std::cout << "Alpha values must be coprime with 26." << std::endl;
        return 1;
    }

    std::string encrypted = RecAth(message, alpha1, alpha2, beta1, beta2);
    std::cout << "Encrypted: " << encrypted << std::endl;

    std::string decrypted = DeRecAth(encrypted, alpha1, alpha2, beta1, beta2);
    std::cout << "Decrypted: " << decrypted << std::endl;

    return 0;
}
*/
