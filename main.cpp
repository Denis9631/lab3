#include <iostream>
#include <string>
#include <memory>
#include <sstream>
#include <map>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

// Объявления функций тестирования
extern void runLexerTests();
extern void runParserTests();
extern void runEvaluateTests();
extern void runTreeTests();
extern void runDerivativeExprTests();
extern void runDerivativeEvalTests();



// Функция для режима evaluate
void runEvaluate() {
    std::string line;
    
    // 1. Читаем количество переменных
    std::getline(std::cin, line);
    int varCount;
    try {
        varCount = std::stoi(line);
    } catch (...) {
        std::cout << "ERROR" << std::endl;
        return;
    }
    
    // 2. Читаем имена переменных
    std::getline(std::cin, line);
    std::istringstream varStream(line);
    std::vector<std::string> varNames;
    std::string varName;
    for (int i = 0; i < varCount; ++i) {
        if (varStream >> varName) {
            varNames.push_back(varName);
        } else {
            std::cout << "ERROR" << std::endl;
            return;
        }
    }
    
    // 3. Читаем значения переменных
    std::getline(std::cin, line);
    std::istringstream valStream(line);
    std::vector<double> varValues;
    double val;
    for (int i = 0; i < varCount; ++i) {
        if (valStream >> val) {
            varValues.push_back(val);
        } else {
            std::cout << "ERROR" << std::endl;
            return;
        }
    }
    
    // 4. Читаем выражение
    std::string expression;
    std::getline(std::cin, expression);
    
    // 5. Создаём словарь переменных
    std::map<std::string, double> vars;
    for (int i = 0; i < varCount; ++i) {
        vars[varNames[i]] = varValues[i];
    }
    
    try {
        Lexer lexer(expression);
        Parser parser(lexer);
        auto ast = parser.parse();
        double result = ast->evaluate(vars);
        std::cout << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
    }
}

int main() {
    std::string mode;
    std::getline(std::cin, mode);
    
    if (mode == "testlexer") {
        runLexerTests();
    } else if (mode == "testparser") {
        runParserTests();
    } else if (mode == "evaluate") {
        runEvaluate();
    }
    else if (mode == "testevaluate") {
        runEvaluateTests();
    }
    else if (mode == "lexer") {
        std::string expression;
        std::getline(std::cin, expression);
        try {
            Lexer lexer(expression);
            Token tok = lexer.next();
            while (tok.type() != lexem_t::EOEX) {
                std::cout << tok.view() << std::endl;
                tok = lexer.next();
            }
        } catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }}
    else if (mode == "tree") {
        std::string expression;
        std::getline(std::cin, expression);
        try {
            Lexer lexer(expression);
            Parser parser(lexer);
            auto ast = parser.parse();
            std::cout << ast->toTree() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    }
    else if (mode == "testtree") {
        runTreeTests();
        return 0;
    }
     else if (mode == "parser") {
        std::string expression;
        std::getline(std::cin, expression);
        try {
            Lexer lexer(expression);
            Parser parser(lexer);
            auto ast = parser.parse();
            std::cout << ast->toString() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    } 
    else if (mode == "derivative") {
        std::string line;
        
        std::getline(std::cin, line);
        int varCount = std::stoi(line);
        
        std::getline(std::cin, line);
        std::istringstream varStream(line);
        std::vector<std::string> varNames;
        std::string varName;
        for (int i = 0; i < varCount; ++i) {
            varStream >> varName;
            varNames.push_back(varName);
        }
        
        std::string diffVar = varNames[0];  
        
        std::getline(std::cin, line);
        
        std::string expression;
        std::getline(std::cin, expression);
        
        try {
            Lexer lexer(expression);
            Parser parser(lexer);
            auto ast = parser.parse();
            auto deriv = ast->derivative(diffVar);  
            std::cout << deriv->toString() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    }
    else if (mode == "evaluate_derivative") {
        std::string line;
        
        std::getline(std::cin, line);
        int varCount = std::stoi(line);
        
        std::getline(std::cin, line);
        std::istringstream varStream(line);
        std::vector<std::string> varNames;
        std::string varName;
        for (int i = 0; i < varCount; ++i) {
            varStream >> varName;
            varNames.push_back(varName);
        }
        
        std::getline(std::cin, line);
        std::istringstream valStream(line);
        std::vector<double> varValues;
        double val;
        for (int i = 0; i < varCount; ++i) {
            valStream >> val;
            varValues.push_back(val);
        }
        
        std::string expression;
        std::getline(std::cin, expression);
        
        std::map<std::string, double> vars;
        for (int i = 0; i < varCount; ++i) {
            vars[varNames[i]] = varValues[i];
        }
        
        try {
            Lexer lexer(expression);
            Parser parser(lexer);
            auto ast = parser.parse();
            
            std::string diffVar = varNames[0];
            
            auto deriv = ast->derivative(diffVar);
            double result = deriv->evaluate(vars);
            std::cout << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    }
    else if (mode == "test_derivative_expr") {
        runDerivativeExprTests();
    }
    else if (mode == "test_derivative_eval") {
        runDerivativeEvalTests();
    }
    else {
        std::cout << "ERROR: wrong mode activation" << std::endl;
    }
    
    return 0;
}