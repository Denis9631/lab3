#include "parser.h"
#include <stdexcept>
#include <string>

void Parser::nextToken() {
    currentToken = lexer.next();
}

bool Parser::match(lexem_t type) {
    return currentToken.type() == type;
}

void Parser::expect(lexem_t type) {
    if (!match(type)) {
        throw std::runtime_error("Expected " + std::to_string(static_cast<int>(type)));
    }
}

std::unique_ptr<ExprNode> Parser::parseExpression() {
    return parseAddSub();
}

std::unique_ptr<ExprNode> Parser::parseAddSub() {
    auto left = parseMulDiv();
    
    while (match(lexem_t::OPERATOR) && 
           (currentToken.view() == "+" || currentToken.view() == "-")) {
        std::string op = currentToken.view();
        nextToken();
        auto right = parseMulDiv();
        left = std::make_unique<BinaryOpNode>(op[0], std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<ExprNode> Parser::parseMulDiv() {
    auto left = parseUnary();
    
    while (match(lexem_t::OPERATOR) && 
           (currentToken.view() == "*" || currentToken.view() == "/")) {
        std::string op = currentToken.view();
        nextToken();
        auto right = parseUnary();
        left = std::make_unique<BinaryOpNode>(op[0], std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<ExprNode> Parser::parseUnary() {
    if (match(lexem_t::OPERATOR) && currentToken.view() == "-") {
        nextToken();
        auto operand = parseUnary();
        return std::make_unique<UnaryOpNode>('-', std::move(operand));
    }
    
    if (match(lexem_t::OPERATOR) && currentToken.view() == "+") {
        nextToken();
        auto operand = parseUnary();
        return operand;
    }
    
    return parsePower();
}

std::unique_ptr<ExprNode> Parser::parsePower() {
    auto left = parsePrimary();
    
    // Проверяем, не является ли левый операнд константой e
    if (auto num = dynamic_cast<NumberNode*>(left.get())) {
        if (std::abs(num->value() - std::exp(1.0)) < 1e-9) {
            if (match(lexem_t::OPERATOR) && currentToken.view() == "^") {
                nextToken();
                auto right = parseUnary();
                std::vector<std::unique_ptr<ExprNode>> args;
                args.push_back(std::move(right));
                return std::make_unique<FunctionNode>("exp", std::move(args));
            }
        }
    }
    
    if (match(lexem_t::OPERATOR) && currentToken.view() == "^") {
        nextToken();
        auto right = parseUnary();
        return std::make_unique<BinaryOpNode>('^', std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<ExprNode> Parser::parsePrimary() {
    if (match(lexem_t::NUMBER)) {
        double value = std::stod(currentToken.view());
        nextToken();
        return std::make_unique<NumberNode>(value);
    }
    
    if (match(lexem_t::IDENT)) {
        std::string name = currentToken.view();
        
        // Константа e
        if (name == "e") {
            nextToken();
            return std::make_unique<NumberNode>(std::exp(1.0));
        }
        
        // Функции
        if (name == "sin" || name == "cos" || name == "tg" || name == "ctg" || 
            name == "ln" || name == "sqrt" || name == "log") {
            nextToken();
            if (match(lexem_t::LPAREN)) {
                return parseFunctionCall(name);
            } else {
                throw std::runtime_error("Expected '(' after function name: " + name);
            }
        }
        
        // Переменная
        nextToken();
        return std::make_unique<VariableNode>(name);
    }
    
    if (match(lexem_t::LPAREN)) {
        nextToken();
        auto expr = parseExpression();
        expect(lexem_t::RPAREN);
        nextToken();
        return expr;
    }
    
    throw std::runtime_error("Unexpected token: " + currentToken.view());
}

std::unique_ptr<ExprNode> Parser::parseFunctionCall(const std::string& name) {
    nextToken();  // пропускаем '('
    
    std::vector<std::unique_ptr<ExprNode>> args;
    
    // Если следующий токен не ')', значит есть аргументы
    if (!match(lexem_t::RPAREN)) {
                    args.push_back(parseExpression());
    }
    
    expect(lexem_t::RPAREN);
    nextToken();
    
    return std::make_unique<FunctionNode>(name, std::move(args));
}

Parser::Parser(Lexer& lex) : lexer(lex), currentToken(lex.next()) {}

std::unique_ptr<ExprNode> Parser::parse() {
    auto result = parseExpression();
    
    if (!match(lexem_t::EOEX)) {
        throw std::runtime_error("Unexpected token after expression");
    }
    
    return result;
}