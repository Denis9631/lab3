#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

struct ParserTestCase {
    std::string input;
    std::string expected;
    std::string description;
};

void runParserTests() {
    std::vector<ParserTestCase> tests = {
        // Базовые выражения
        {"2+3", "(2 + 3)", "Простое сложение"},
        {"2-3", "(2 - 3)", "Простое вычитание"},
        {"2*3", "(2 * 3)", "Простое умножение"},
        {"2/3", "(2 / 3)", "Простое деление"},
        {"2^3", "(2 ^ 3)", "Простое возведение в степень"},
        
        // Приоритет операций
        {"2+3*4", "(2 + (3 * 4))", "Приоритет * над +"},
        {"2*3+4", "((2 * 3) + 4)", "Приоритет * над + (2)"},
        {"2^3*4", "((2 ^ 3) * 4)", "Приоритет ^ над *"},
        {"2+3-4", "((2 + 3) - 4)", "Левоассоциативность + и -"},
        {"2*3/4", "((2 * 3) / 4)", "Левоассоциативность * и /"},
        
        // Степень правоассоциативна
        {"2^3^2", "(2 ^ (3 ^ 2))", "Правоассоциативность ^"},
        
        // Скобки
        {"(2+3)*4", "((2 + 3) * 4)", "Скобки меняют приоритет"},
        {"2*(3+4)", "(2 * (3 + 4))", "Скобки в правом операнде"},
        {"(2+3)*(4+5)", "((2 + 3) * (4 + 5))", "Скобки с двух сторон"},
        {"((2+3)*4)", "((2 + 3) * 4)", "Двойные скобки"},
        
        // Унарные операторы
        {"-x", "(-x)", "Унарный минус"},
        {"-2", "(-2)", "Унарный минус с числом"},
        {"1+-2", "(1 + (-2))", "Унарный минус после +"},
        {"1-+2", "(1 - (+2))", "Унарный плюс после -"},
        {"-x*y", "((-x) * y)", "Унарный минус в умножении"},
        
        // Переменные
        {"x", "x", "Одна переменная"},
        {"x+y", "(x + y)", "Сложение переменных"},
        {"_x + _y", "(_x + _y)", "Переменные с подчеркиванием"},
        {"var123", "var123", "Переменная с цифрами"},
        
        // Числа
        {"123", "123", "Целое число"},
        {"3.14", "3.14", "Дробное число"},
        {"1e10", "1e10", "Число с экспонентой"},
        {"1.5e-5", "1.5e-05", "Дробное с экспонентой"},
        
        // Функции
        {"sin(x)", "sin(x)", "Функция с одним аргументом"},
        {"cos(2*x)", "cos((2 * x))", "Функция с выражением"},
        {"sin(cos(x))", "sin(cos(x))", "Вложенные функции"},
        
        // Комплексные выражения
        {"2*x + 3*y", "((2 * x) + (3 * y))", "Комплексное выражение 1"},
        {"x^2 + y^2", "((x ^ 2) + (y ^ 2))", "Комплексное выражение 2"},
        {"(x+y)*(x-y)", "((x + y) * (x - y))", "Комплексное выражение 3"},
        {"sin(x)^2 + cos(x)^2", "((sin(x) ^ 2) + (cos(x) ^ 2))", "Тригонометрическое тождество"},
    };

    std::vector<ParserTestCase> errorTests = {
        // Синтаксические ошибки
        {"+2", "ERROR", "Унарный плюс с числом"},
        {"+x", "ERROR", "Унарный плюс"},
        
        {"2+", "ERROR", "Оператор без правого операнда"},
        {"*2", "ERROR", "Оператор без левого операнда"},
        {"(2+3", "ERROR", "Незакрытая скобка"},
        {"2+3)", "ERROR", "Лишняя закрывающая скобка"},
        {"sin(x", "ERROR", "Незакрытая скобка функции"},
        {"max(1,)", "ERROR", "Пустой аргумент функции"},
        {"2++3", "ERROR", "Два оператора подряд"},  // или упрощается до (2 + (+3))
    };

    int passed = 0;
    int failed = 0;

    std::cout << "=== Запуск тестов парсера (успешные) ===" << std::endl;
    for (const auto& test : tests) {
        try {
            Lexer lexer(test.input);
            Parser parser(lexer);
            auto ast = parser.parse();
            std::string result = ast->toString();
            
            if (result == test.expected) {
                std::cout << "✓ PASS: " << test.description << std::endl;
                passed++;
            } else {
                std::cout << "✗ FAIL: " << test.description << std::endl;
                std::cout << "  Input: \"" << test.input << "\"" << std::endl;
                std::cout << "  Expected: \"" << test.expected << "\"" << std::endl;
                std::cout << "  Got:      \"" << result << "\"" << std::endl;
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "✗ FAIL: " << test.description << " (Exception: " << e.what() << ")" << std::endl;
            std::cout << "  Input: \"" << test.input << "\"" << std::endl;
            failed++;
        }
    }

    std::cout << "\n=== Запуск тестов парсера (ожидаемые ошибки) ===" << std::endl;
    for (const auto& test : errorTests) {
        try {
            Lexer lexer(test.input);
            Parser parser(lexer);
            auto ast = parser.parse();
            std::string result = ast->toString();
            
            std::cout << "✗ FAIL: " << test.description << " (должна быть ошибка)" << std::endl;
            std::cout << "  Input: \"" << test.input << "\"" << std::endl;
            std::cout << "  Got: \"" << result << "\" (ожидалась ошибка)" << std::endl;
            failed++;
            
        } catch (const std::exception& e) {
            std::cout << "✓ PASS: " << test.description << " (Exception: " << e.what() << ")" << std::endl;
            passed++;
        }
    }

    std::cout << "\n=== Результаты тестов парсера ===" << std::endl;
    std::cout << "Пройдено: " << passed << std::endl;
    std::cout << "Провалено: " << failed << std::endl;
    std::cout << "Всего: " << (passed + failed) << std::endl;
}