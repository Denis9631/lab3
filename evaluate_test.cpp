#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <limits>
#include <cmath>

struct EvaluateTestCase {
    std::string description;
    std::vector<std::string> varNames;
    std::vector<double> varValues;
    std::string expression;
    double expected;
    bool expectError;
};

void runEvaluateTests() {
    std::vector<EvaluateTestCase> tests = {
        // Базовые арифметические операции
        {"Простое сложение", {}, {}, "2+3", 5.0, false},
        {"Простое вычитание", {}, {}, "5-2", 3.0, false},
        {"Простое умножение", {}, {}, "3*4", 12.0, false},
        {"Простое деление", {}, {}, "10/2", 5.0, false},
        {"Возведение в степень", {}, {}, "2^3", 8.0, false},
        
        // Приоритет операций
        {"Приоритет * над +", {}, {}, "2+3*4", 14.0, false},
        {"Приоритет ^ над *", {}, {}, "2*3^2", 18.0, false},
        {"Скобки", {}, {}, "(2+3)*4", 20.0, false},
        
        // Унарные операторы
        {"Унарный минус", {}, {}, "-5", -5.0, false},
        {"Унарный плюс", {}, {}, "+5", 5.0, false},
        {"Двойной унарный минус", {}, {}, "--5", 5.0, false},
        {"Унарный минус в выражении", {}, {}, "3+-2", 1.0, false},
        
        // Десятичные числа
        {"Десятичное число", {}, {}, "3.14", 3.14, false},
        
        // Числа в научной нотации
        {"1e10", {}, {}, "1e10", 10000000000.0, false},
        {"1e-5", {}, {}, "1e-5", 0.00001, false},
        {"2.5e3", {}, {}, "2.5e3", 2500.0, false},
        {"0.5e-2", {}, {}, "0.5e-2", 0.005, false},
        {"1e+10", {}, {}, "1e+10", 10000000000.0, false},
        {"5e-1", {}, {}, "5e-1", 0.5, false},
        {"3.14e2", {}, {}, "3.14e2", 314.0, false},
        {"1.23e-4", {}, {}, "1.23e-4", 0.000123, false},
        
        // Переменные
        {"Одна переменная", {"x"}, {5}, "x", 5.0, false},
        {"Две переменные", {"x", "y"}, {10, 20}, "x+y", 30.0, false},
        {"Выражение с переменными", {"x", "y"}, {2, 3}, "x*y+5", 11.0, false},
        {"Переменная в степени", {"x"}, {2}, "x^3", 8.0, false},
        
        // Функции
        {"sin(0)", {}, {}, "sin(0)", 0.0, false},
        {"sin(pi/2)", {}, {}, "sin(1.57079632679)", 1.0, false},
        {"cos(0)", {}, {}, "cos(0)", 1.0, false},
        {"cos(pi)", {}, {}, "cos(3.14159265359)", -1.0, false},
        {"tg(0)", {}, {}, "tg(0)", 0.0, false},
        {"log(1)", {}, {}, "log(1)", 0.0, false},
        {"log(e)", {}, {}, "log(2.71828182846)", 1.0, false},
        {"sqrt(0)", {}, {}, "sqrt(0)", 0.0, false},
        {"sqrt(4)", {}, {}, "sqrt(4)", 2.0, false},
        
        // Вложенные функции
        {"sin(cos(0))", {}, {}, "sin(cos(0))", std::sin(1.0), false},
        
        // Комплексные выражения с научной нотацией
        {"(1e3 + 2e3)", {}, {}, "(1e3 + 2e3)", 3000.0, false},
        {"(5e-2 * 2e1)", {}, {}, "(5e-2 * 2e1)", 1.0, false},
        {"(1e10 / 1e5)", {}, {}, "(1e10 / 1e5)", 100000.0, false},
        {"e^e^e-e^e^e+0+0+0+0+0+0+0",{},{},"e^e^e-e^e^e+0+0+0+0+0+0+0",0.0,false},
        {"e^x^e^y",{"x","y"},{2,1},"e^x^e^y",721.178,false},

        // Комплексные выражения
        {"(x+y)*(x-y)", {"x", "y"}, {5, 3}, "(x+y)*(x-y)", 16.0, false},
        {"x^2 + y^2", {"x", "y"}, {3, 4}, "x^2 + y^2", 25.0, false},
        {"sin(x)^2 + cos(x)^2", {"x"}, {1.5}, "sin(x)^2 + cos(x)^2", 1.0, false},
        
        // Крайние случаи
        {"Деление на ноль", {}, {}, "1/0", std::numeric_limits<double>::infinity(), false},
        {"log(0)", {}, {}, "log(0)", -std::numeric_limits<double>::infinity(), false},
        {"sqrt(-1)", {}, {}, "sqrt(-1)", 0.0, true},
        {"log(-1)", {}, {}, "log(-1)", 0.0, true},
        {"Неизвестная переменная", {"x"}, {5}, "y", 0.0, true},
        {"Неизвестная функция", {}, {}, "unknown(5)", 0.0, true},
    };
    
    int passed = 0;
    int failed = 0;
    
    std::cout << "=== Запуск тестов evaluate ===" << std::endl;
    
    for (const auto& test : tests) {
        // Создаём словарь переменных
        std::map<std::string, double> vars;
        for (size_t i = 0; i < test.varNames.size(); ++i) {
            vars[test.varNames[i]] = test.varValues[i];
        }
        
        try {
            Lexer lexer(test.expression);
            Parser parser(lexer);
            auto ast = parser.parse();
            double result = ast->evaluate(vars);

            if (test.expectError) {
                std::cout << "✗ FAIL: " << test.description << " (должна быть ошибка, но получили " << result << ")" << std::endl;
                failed++;
            } else {
                // Преобразуем результат в строку
                std::ostringstream resultStream;
                resultStream << result;
                std::string resultStr = resultStream.str();
                
                // Преобразуем ожидаемое значение в строку
                std::ostringstream expectedStream;
                expectedStream << test.expected;
                std::string expectedStr = expectedStream.str();
                
                if (resultStr == expectedStr) {
                    std::cout << "✓ PASS: " << test.description << " = " << resultStr << std::endl;
                    passed++;
                } else {
                    std::cout << "✗ FAIL: " << test.description << std::endl;
                    std::cout << "  Expression: \"" << test.expression << "\"" << std::endl;
                    std::cout << "  Expected: " << expectedStr << std::endl;
                    std::cout << "  Got: " << resultStr << std::endl;
                    failed++;
                }
            }
        } catch (const std::exception& e) {
            if (test.expectError) {
                std::cout << "✓ PASS: " << test.description << " (ERROR: " << e.what() << ")" << std::endl;
                passed++;
            } else {
                std::cout << "✗ FAIL: " << test.description << " (Exception: " << e.what() << ")" << std::endl;
                failed++;
            }
        }
    }
    
    std::cout << "\n=== Результаты тестов evaluate ===" << std::endl;
    std::cout << "Пройдено: " << passed << std::endl;
    std::cout << "Провалено: " << failed << std::endl;
    std::cout << "Всего: " << (passed + failed) << std::endl;
}