#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <conio.h>

const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;
const int MAX_STRINGS = 1;
const int STRING_LENGTH = 5;

class StringGenerator {
public:
    virtual std::string generateString() = 0;
};

class AlphabetGenerator : public StringGenerator {
public:
    std::string generateString() override {
        std::string result;
        static const char alphabet[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(alphabet) - 2);

        for (int i = 0; i < STRING_LENGTH; ++i) {
            result += alphabet[dis(gen)];
        }

        return result;
    }
};

class AlphabetNumberGenerator : public AlphabetGenerator {
public:
    std::string generateString() override {
        std::string result = AlphabetGenerator::generateString().substr(0, STRING_LENGTH / 2); // 문자열 절반은 알파벳으로 채움

        static const char numbers[] = "0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(numbers) - 2);

        // 나머지 절반은 숫자로 채움
        for (int i = result.size(); i < STRING_LENGTH; ++i) {
            result += numbers[dis(gen)];
        }

        return result;
    }
};

class AlphabetSpecialCharGenerator : public AlphabetGenerator {
public:
    std::string generateString() override {
        std::string result = AlphabetGenerator::generateString().substr(0, STRING_LENGTH / 2); // 문자열 절반은 알파벳으로 채움

        static const char specialChars[] = "!@#$%^&*";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(specialChars) - 2);

        // 나머지 절반은 특수문자로 채움
        for (int i = result.size(); i < STRING_LENGTH; ++i) {
            result += specialChars[dis(gen)];
        }

        return result;
    }
};


class Game {
    struct FallingString {
        std::string value;

        int x;
        int y;
    };

    std::string currentInput;
    std::vector<FallingString> strings;
    std::mutex mtx;
    bool gameOver;
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution;
    StringGenerator* stringGenerator;
    int score;

public:
    Game(StringGenerator* gen) : gameOver(false), distribution(0, SCREEN_WIDTH - STRING_LENGTH), stringGenerator(gen), score(10) {
        for (int i = 0; i < MAX_STRINGS; ++i) {
            addRandomString();
        }
    }

    void addRandomString() {
        FallingString newString;
        newString.value = stringGenerator->generateString();
        newString.x = distribution(generator);
        newString.y = 0;

        strings.push_back(newString);
    }

    void run() {
        std::thread inputThread(&Game::handleInput, this);
        std::thread gameThread(&Game::gameLoop, this);

        inputThread.join();
        gameThread.join();
    }

    void handleInput() {
        char ch;
        while (!gameOver) {
            if (_kbhit()) {
                ch = _getch();
                if (ch == '\r') {
                    if (!currentInput.empty()) {
                        mtx.lock();
                        bool found = false;
                        for (auto it = strings.begin(); it != strings.end(); ++it) {
                            if (it->value == currentInput) {
                                strings.erase(it);
                                found = true;
                                break;
                            }
                        }
                        currentInput.clear();
                        mtx.unlock();
                        if (found) {
                            score += 1;
                        }
                        std::cout << std::endl << "Input: " << std::endl;
                    }
                    else {
                        std::cout << std::endl << "Input: " << std::endl;
                    }
                }
                else if (ch == '\b') {
                    if (!currentInput.empty()) {
                        currentInput.pop_back();
                        std::cout << "\b \b";
                    }
                }
                else if (isprint(ch)) {
                    currentInput += ch;
                    std::cout << ch;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }


    void gameLoop() {
         std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delayDistribution(500, 2000);

    while (!gameOver) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delayDistribution(gen)));
            mtx.lock();
            auto it = strings.begin();
            while (it != strings.end()) {
                if (++it->y > SCREEN_HEIGHT - 2) {
                    it = strings.erase(it);
                    score -= 1;
                }
                else {
                    ++it;
                }
            }
            mtx.unlock();
            addRandomString();
            drawScreen(currentInput);

            if (score <= 0) {
                gameOver = true;
            }
        }
    }

    void drawScreen(const std::string& currentInput) {
        system("cls");
        std::vector<std::string> display(SCREEN_HEIGHT - 1, std::string(SCREEN_WIDTH, ' '));

        for (const auto& str : strings) {
            if (str.y < SCREEN_HEIGHT - 1) {
                for (size_t i = 0; i < str.value.size(); ++i) {
                    display[str.y][str.x + i] = str.value[i];
                }
            }
        }

        for (const auto& line : display) {
            std::cout << line << std::endl;
        }

        std::cout << std::string(SCREEN_WIDTH, '-') << std::endl;
        std::cout << "Score: " << score << std::endl;
        std::cout << "Input: " << currentInput;
    }
};

int main() {
    std::cout << "Choose string type (1: Alphabet, 2: Alphabet + Number, 3: Alphabet + Special Character): ";
    int choice;
    std::cin >> choice;

    StringGenerator* generator;
    switch (choice) {
    case 1:
        generator = new AlphabetGenerator();
        break;
    case 2:
        generator = new AlphabetNumberGenerator();
        break;
    case 3:
        generator = new AlphabetSpecialCharGenerator();
        break;
    default:
        std::cout << "Invalid choice. Defaulting to Alphabet.\n";
        generator = new AlphabetGenerator();
    }

    Game game(generator);
    game.run();

    delete generator;
    return 0;
}