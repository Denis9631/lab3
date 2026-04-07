#include "lexer.h"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

struct TestCase {
    std::string input;
    std::string expected;
    std::string description;
};

// Объявляем, что функция есть в lexer.cpp
extern std::string lexer_flow_concatenator(const std::string& expression);

void runLexerTests() {
    std::vector<TestCase> tests = {
        // Базовые тесты
        {"42", "42\n", "Простое число"},
        {"abc", "abc\n", "Простой идентификатор"},
        {"2 + 3", "2\n+\n3\n", "Простое сложение"},
        {"", "", "Пустая строка"},
        
        // Тесты операторов
        {"1+2-3*4/5^6", "1\n+\n2\n-\n3\n*\n4\n/\n5\n^\n6\n", "Все операторы"},
        {"a+b", "a\n+\nb\n", "Операторы с идентификаторами"},
        
        // Тесты скобок
        {"(1+2)", "(\n1\n+\n2\n)\n", "Выражение в скобках"},
        {"((a))", "(\n(\na\n)\n)\n", "Вложенные скобки"},
        {"()", "(\n)\n", "Пустые скобки"},
        
        // Тесты пробелов
        {"  1  +  2  ", "1\n+\n2\n", "Множественные пробелы"},
        {"\t1\n+\r2", "1\n+\n2\n", "Разные виды пробелов"},
        {"   ", "", "Только пробелы"},
        
        // Тесты идентификаторов
        {"_var", "_var\n", "Идентификатор с подчеркиванием"},
        {"var123", "var123\n", "Идентификатор с цифрами"},
        {"_123abc", "_123abc\n", "Идентификатор начинается с _"},
        {"ABC", "abc\n", "Регистр приводится к нижнему"},
        {"VaRiAbLe", "variable\n", "Смешанный регистр"},
        
        // Тесты дробных чисел
        {"3.14", "3.14\n", "Простое дробное число"},
        {"0.5", "0.5\n", "Дробное число начинается с 0"},
        {"123.456", "123.456\n", "Дробное с несколькими цифрами"},
        {"1.0", "1.0\n", "Число с .0"},
        
        // Тесты экспоненциальной записи
        {"1e10", "1e10\n", "Экспонента без знака"},
        {"1e+10", "1e+10\n", "Экспонента с плюсом"},
        {"1e-10", "1e-10\n", "Экспонента с минусом"},
        {"2.5e3", "2.5e3\n", "Дробное с экспонентой"},
        {"0.5E-2", "0.5e-2\n", "Заглавная E в экспоненте"},
        
        // Тесты нуля
        {"0", "0\n", "Просто ноль"},
        {"0+1", "0\n+\n1\n", "Ноль в выражении"},
        
        // Комплексные выражения
        {"2*x + 3.14*y", "2\n*\nx\n+\n3.14\n*\ny\n", "Комплексное выражение"},
        {"(a+b)*(c-d)", "(\na\n+\nb\n)\n*\n(\nc\n-\nd\n)\n", "Скобки и операторы"},
    };

    std::vector<TestCase> errorTests = {
        // Тесты ошибок - ведущие нули
        {"01", "ERROR", "Ведущий ноль"},
        {"0123", "ERROR", "Число с ведущим нулем"},
        {"00", "ERROR", "Два нуля"},
        
        // Тесты ошибок - неправильный формат чисел
        {"1.", "ERROR", "Число заканчивается точкой"},
        {".", "ERROR", "Только точка"},
        {"1..2", "ERROR", "Две точки подряд"},
        {"0.", "ERROR", "Ноль с точкой без цифр"},
        
        // Тесты ошибок - экспонента
        {"1e", "ERROR", "Экспонента без цифр"},
        {"1e+", "ERROR", "Экспонента с знаком без цифр"},
        {"1E-", "ERROR", "Экспонента E с знаком без цифр"},
        
        // Тесты ошибок - число и буква
        {"123abc", "ERROR", "Число переходит в букву"},
        {"0x123", "ERROR", "Ноль с буквой x"},
        
        // Тесты ошибок - неизвестные символы
        {"1@2", "ERROR", "Неизвестный символ @"},
        {"a#b", "ERROR", "Неизвестный символ #"},
        {"1$", "ERROR", "Неизвестный символ $"},
        {"[1]", "ERROR", "Квадратные скобки"},
        {"{a}", "ERROR", "Фигурные скобки"},
    };

    int passed = 0;
    int failed = 0;

    std::cout << "=== Запуск успешных тестов ===" << std::endl;
    for (const auto& test : tests) {
        try {
            std::string result = lexer_flow_concatenator(test.input);
            if (result == test.expected) {
                std::cout << "✓ PASS: " << test.description << std::endl;
                passed++;
            } else {
                std::cout << "✗ FAIL: " << test.description << std::endl;
                std::cout << "  Input: \"" << test.input << "\"" << std::endl;
                std::cout << "  Expected: \"" << test.expected << "\"" << std::endl;
                std::cout << "  Got: \"" << result << "\"" << std::endl;
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "✗ FAIL: " << test.description << " (Exception: " << e.what() << ")" << std::endl;
            failed++;
        }
    }

    std::cout << "\n=== Запуск тестов ошибок ===" << std::endl;
    for (const auto& test : errorTests) {
        std::string result;
        try {
            result = lexer_flow_concatenator(test.input);
            std::cout << "✗ FAIL: " << test.description << " (должна быть ошибка)" << std::endl;
            std::cout << "  Input: \"" << test.input << "\"" << std::endl;
            std::cout << "  Got: \"" << result << "\" (ожидалась ошибка)" << std::endl;
            failed++;
        } catch (const std::exception& e) {
            std::cout << "✓ PASS: " << test.description << " (Exception caught: " << e.what() << ")" << std::endl;
            passed++;
        }
    }

    std::cout << "\n=== Результаты ===" << std::endl;
    std::cout << "Пройдено: " << passed << std::endl;
    std::cout << "Провалено: " << failed << std::endl;
    std::cout << "Всего: " << (passed + failed) << std::endl;
}