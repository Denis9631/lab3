#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include <iostream>
#include <string>
#include <vector>

struct TreeTestCase {
    std::string input;
    std::string expectedTree;
    std::string description;
};

void runTreeTests() {
    std::vector<TreeTestCase> tests = {
        // Базовые выражения
        {"2+3", 
         "+\n  2\n  3", 
         "Простое сложение"},
        
        {"2*3+4", 
         "+\n  *\n    2\n    3\n  4", 
         "Приоритет операций"},
        
        {"(2+3)*4", 
         "*\n  +\n    2\n    3\n  4", 
         "Скобки"},
        
        {"-5", 
         "-\n  5", 
         "Унарный минус"},
        
        {"sin(x)", 
         "sin()\n  x", 
         "Функция sin"},
        
        {"e^2", 
         "exp()\n  2", 
         "Степень с e"},
        
        {"2+3*4-5", 
         "-\n  +\n    2\n    *\n      3\n      4\n  5", 
         "Сложное выражение"},
    };

    int passed = 0;
    int failed = 0;

    std::cout << "=== Запуск тестов AST дерева ===" << std::endl;
    
    for (const auto& test : tests) {
        try {
            Lexer lexer(test.input);
            Parser parser(lexer);
            auto ast = parser.parse();
            std::string result = ast->toTree();
            
            if (result == test.expectedTree) {
                std::cout << "✓ PASS: " << test.description << std::endl;
                passed++;
            } else {
                std::cout << "✗ FAIL: " << test.description << std::endl;
                std::cout << "  Input: \"" << test.input << "\"" << std::endl;
                std::cout << "  Expected tree:\n" << test.expectedTree << std::endl;
                std::cout << "  Got tree:\n" << result << std::endl;
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "✗ FAIL: " << test.description << " (Exception: " << e.what() << ")" << std::endl;
            failed++;
        }
    }

    std::cout << "\n=== Результаты тестов AST ===" << std::endl;
    std::cout << "Пройдено: " << passed << std::endl;
    std::cout << "Провалено: " << failed << std::endl;
    std::cout << "Всего: " << (passed + failed) << std::endl;
}