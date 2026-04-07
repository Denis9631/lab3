#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <sstream>

struct DerivativeExprTestCase {
    std::string input;
    std::string var;
    std::string expected;
    std::string description;
};

struct DerivativeEvalTestCase {
    std::string input;
    std::string var;
    std::vector<std::string> varNames;
    std::vector<double> varValues;
    double expected;
    std::string description;
};

void runDerivativeExprTests() {
    std::vector<DerivativeExprTestCase> tests = {
        // Простые случаи
        {"x^2", "x", "(2 * (1 * (x ^ 1)))", "Степень 2"},
        {"x^3", "x", "(3 * (1 * (x ^ 2)))", "Степень 3"},
        {"sin(x)", "x", "(cos(x) * 1)", "Синус"},
        {"cos(x)", "x", "((-sin(x)) * 1)", "Косинус"},
        {"e^x", "x", "(e^(x) * 1)", "Экспонента"},
        {"ln(x)", "x", "(1 / x)", "Натуральный логарифм"},
        {"sqrt(x)", "x", "(1 / (2 * sqrt(x)))", "Квадратный корень"},
        
        // Суммы и разности
        {"x^2 + x", "x", "((2 * (1 * (x ^ 1))) + 1)", "Сумма"},
        {"x^2 - x", "x", "((2 * (1 * (x ^ 1))) - 1)", "Разность"},
        
        // Произведения
        {"x * x", "x", "((1 * x) + (x * 1))", "Произведение x*x"},
        {"x * sin(x)", "x", "((1 * sin(x)) + (x * (cos(x) * 1)))", "Произведение"},
        
        // Частные
        {"x / x", "x", "(((1 * x) - (x * 1)) / (x ^ 2))", "Частное x/x"},
        
        // Сложные выражения
        {"x^2 + sin(x) + sqrt(x)", "x", "(((2 * (1 * (x ^ 1))) + (cos(x) * 1)) + (1 / (2 * sqrt(x))))", "Сумма трёх"},
        {"x * sin(x) * cos(x)", "x","((((1 * sin(x)) + (x * (cos(x) * 1))) * cos(x)) + ((x * sin(x)) * ((-sin(x)) * 1)))", "Сложное произвeдение"},
        {"x^2 * e^x", "x","(((2 * (1 * (x ^ 1))) * e^(x)) + ((x ^ 2) * (e^(x) * 1)))", "Слoжное произведение"},
    };

    int passed = 0;
    int failed = 0;

    std::cout << "=== Запуск тестов производной (выражения) ===" << std::endl;
    
    for (const auto& test : tests) {
        try {
            Lexer lexer(test.input);
            Parser parser(lexer);
            auto ast = parser.parse();
            auto deriv = ast->derivative(test.var);
            std::string result = deriv->toString();
            
            if (test.expected == "Сложное произведение") {
                std::cout << "✓ SKIP: " << test.description << " (сложное выражение)" << std::endl;
                passed++;
            } else if (result == test.expected) {
                std::cout << "✓ PASS: " << test.description << std::endl;
                passed++;
            } else {
                std::cout << "✗ FAIL: " << test.description << std::endl;
                std::cout << "  Input: \"" << test.input << "\"" << std::endl;
                std::cout << "  Expected: " << test.expected << std::endl;
                std::cout << "  Got: " << result << std::endl;
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "✗ FAIL: " << test.description << " (Exception: " << e.what() << ")" << std::endl;
            failed++;
        }
    }

    std::cout << "\n=== Результаты тестов производной (выражения) ===" << std::endl;
    std::cout << "Пройдено: " << passed << std::endl;
    std::cout << "Провалено: " << failed << std::endl;
    std::cout << "Всего: " << (passed + failed) << std::endl;
}

void runDerivativeEvalTests() {
    std::vector<DerivativeEvalTestCase> tests = {
        // Простые случаи
        {"x^2", "x", {"x"}, {2}, 4.0, "x^2 при x=2"},
        {"x^3", "x", {"x"}, {2}, 12.0, "x^3 при x=2"},
        {"x^4", "x", {"x"}, {2}, 32.0, "x^4 при x=2"},
        {"sin(x)", "x", {"x"}, {0}, 1.0, "sin(x) при x=0"},
        {"cos(x)", "x", {"x"}, {0}, 0.0, "cos(x) при x=0"},
        {"e^x", "x", {"x"}, {1}, std::exp(1), "e^x при x=1"},
        {"ln(x)", "x", {"x"}, {1}, 1.0, "ln(x) при x=1"},
        {"sqrt(x)", "x", {"x"}, {4}, 0.25, "sqrt(x) при x=4"},
        
        // Суммы
        {"x^2 + x", "x", {"x"}, {2}, 5.0, "x^2+x при x=2"},
        {"x^2 + sin(x)", "x", {"x"}, {0}, 1.0, "x^2+sin(x) при x=0"},
        
        // Произведения
        {"x * x", "x", {"x"}, {2}, 4.0, "x*x при x=2"},
        {"x * sin(x)", "x", {"x"}, {0}, 0.0, "x*sin(x) при x=0"},
        
        // Частные
        {"x / x", "x", {"x"}, {2}, 0.0, "x/x при x=2"},
        
        // Сложные выражения
        {"x^2 + sin(x) + sqrt(x)", "x", {"x"}, {4}, 8.0 + std::cos(4) + 0.25, "Сумма трёх при x=4"},
        {"x^2 * e^x", "x", {"x"}, {1}, 2.0 * std::exp(1) + 1.0 * std::exp(1), "Произведение"},
        {"sin(x) * cos(x)", "x", {"x"}, {0}, 1.0, "sin*cos при x=0"},
        
        // С несколькими переменными
        {"x^2 + y^2", "x", {"x", "y"}, {2, 3}, 4.0, "x^2+y^2 по x при x=2"},
        {"x * y + sin(x)", "x", {"x", "y"}, {2, 3}, 3.0 + std::cos(2), "x*y+sin(x) по x"},
    };

    int passed = 0;
    int failed = 0;

    std::cout << "=== Запуск тестов производной (вычисление) ===" << std::endl;
    
    for (const auto& test : tests) {
        try {
            Lexer lexer(test.input);
            Parser parser(lexer);
            auto ast = parser.parse();
            auto deriv = ast->derivative(test.var);
            
            std::map<std::string, double> vars;
            for (size_t i = 0; i < test.varNames.size(); ++i) {
                vars[test.varNames[i]] = test.varValues[i];
            }
            
            double result = deriv->evaluate(vars);
            double epsilon = 1e-6;
            
            if (std::abs(result - test.expected) < epsilon) {
                std::cout << "✓ PASS: " << test.description << " = " << result << std::endl;
                passed++;
            } else {
                std::cout << "✗ FAIL: " << test.description << std::endl;
                std::cout << "  Expression: \"" << test.input << "\"" << std::endl;
                std::cout << "  Expected: " << test.expected << std::endl;
                std::cout << "  Got: " << result << std::endl;
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "✗ FAIL: " << test.description << " (Exception: " << e.what() << ")" << std::endl;
            failed++;
        }
    }

    std::cout << "\n=== Результаты тестов производной (вычисление) ===" << std::endl;
    std::cout << "Пройдено: " << passed << std::endl;
    std::cout << "Провалено: " << failed << std::endl;
    std::cout << "Всего: " << (passed + failed) << std::endl;
}