#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <cctype>
#include <stdexcept>

enum class lexem_t {
    IDENT,      // идентификатор
    NUMBER,     // число
    OPERATOR,   // оператор
    LPAREN,     // левая скобка
    RPAREN,     // правая скобка
    EOEX        // конец выражения
};

class Token {
private:
    lexem_t type_;
    std::string view_;
    
public:
    Token(lexem_t type, const std::string& view);
    
    lexem_t type() const;
    std::string view() const;
};

class Lexer {
private:
    std::string input;
    size_t pos;
    
    char current() const;
    void advance();
    void skipWhitespace();
    bool isIdentStart(char c) const;
    bool isIdentPart(char c) const;
    Token readNumber();
    Token readIdentifier();
    Token readOperator();
    
public:
    Lexer(const std::string& expr);
    Token next();
};

std::string lexer_flow_concatenator(const std::string& expression);

#endif#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <cstring>

Token::Token(lexem_t type, const std::string& view) : type_(type), view_(view) {}

lexem_t Token::type() const { return type_; }
std::string Token::view() const { return view_; }

Lexer::Lexer(const std::string& expr) : input(expr), pos(0) {}

char Lexer::current() const {
    if (pos >= input.length()) return '\0';
    return input[pos];
}

void Lexer::advance() {
    if (pos < input.length()) pos++;
}

void Lexer::skipWhitespace() {
    while (pos < input.length() && std::isspace(input[pos])) {
        pos++;
    }
}

bool Lexer::isIdentStart(char c) const {
    return std::isalpha(c) || c == '_';
}

bool Lexer::isIdentPart(char c) const {
    return std::isalnum(c) || c == '_';
}

Token Lexer::readNumber() {
    std::string result;
    size_t startPos = pos;
    
    // Проверка на ведущий ноль
    if (current() == '0') {
        result += current();
        advance();
        
        // Если после нуля идёт цифра (не точка) - это ошибка
        if (pos < input.length() && std::isdigit(current())) {
            pos = startPos;
            throw std::runtime_error("Invalid number: leading zeros not allowed");
        }
        
        // Если после нуля идёт точка - читаем дробную часть
        if (current() == '.') {
            result += current();
            advance();
            
            // После точки должны быть цифры
            bool hasDigits = false;
            while (pos < input.length() && std::isdigit(input[pos])) {
                result += input[pos];
                advance();
                hasDigits = true;
            }
            
            // Если после точки нет цифр - это ошибка
            if (!hasDigits) {
                pos = startPos;
                throw std::runtime_error("Invalid number format");
            }
        }
    } else {
        // Читаем целую часть для чисел, начинающихся не с нуля
        while (pos < input.length() && std::isdigit(input[pos])) {
            result += input[pos];
            advance();
        }
        
        // Читаем дробную часть, если есть
        if (current() == '.') {
            result += current();
            advance();
            
            // После точки должны быть цифры
            bool hasDigits = false;
            while (pos < input.length() && std::isdigit(input[pos])) {
                result += input[pos];
                advance();
                hasDigits = true;
            }
            
            // Если после точки нет цифр - это ошибка
            if (!hasDigits) {
                pos = startPos;
                throw std::runtime_error("Invalid number format");
            }
        }
    }
    
    // Обработка экспоненциальной записи
    if (current() == 'e' || current() == 'E') {
        result += 'e';
        advance();
    
        if (current() == '+' || current() == '-') {
            result += current();
            advance();
        }
    
        bool hasDigits = false;
        while (pos < input.length() && std::isdigit(input[pos])) {
            result += input[pos];
            advance();
            hasDigits = true;
        }
        if (!hasDigits) {
            pos = startPos;
            throw std::runtime_error("Invalid number format: expected digits after exponent");
        }
    }
    
    // Проверяем, что после числа не идёт буква или идентификатор
    if (pos < input.length() && isIdentStart(input[pos])) {
        pos = startPos;
        throw std::runtime_error("Invalid token: number followed by letter");
    }
    
    return Token(lexem_t::NUMBER, result);
}

Token Lexer::readIdentifier() {
    std::string result;
    
    // Идентификатор должен начинаться с буквы или подчёркивания
    if (!isIdentStart(current())) {
        throw std::runtime_error("Identifier must start with a letter or underscore");
    }
    
    // Идентификатор может содержать буквы, цифры и подчёркивания
    while (pos < input.length() && isIdentPart(input[pos])) {
        result += input[pos];
        advance();
    }
    
    // Преобразуем в нижний регистр для единообразия
    for (char& c : result) {
        c = std::tolower(c);
    }
    
    return Token(lexem_t::IDENT, result);
}

Token Lexer::readOperator() {
    std::string result;
    result += current();
    advance();
    return Token(lexem_t::OPERATOR, result);
}

Token Lexer::next() {
    skipWhitespace();
    
    if (pos >= input.length()) {
        return Token(lexem_t::EOEX, "");
    }
    
    char c = current();
    
    if (std::isdigit(c)) {
        // Числа начинаются с цифры
        return readNumber();
    } else if (isIdentStart(c)) {
        // Идентификаторы начинаются с буквы или подчёркивания
        return readIdentifier();
    } else if (c == '(') {
        advance();
        return Token(lexem_t::LPAREN, "(");
    } else if (c == ')') {
        advance();
        return Token(lexem_t::RPAREN, ")");
    } else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
        return readOperator();
    } else {
        // Неизвестный символ
        throw std::runtime_error(std::string("Unknown character: ") + c);
    }
}

std::string lexer_flow_concatenator(const std::string& expression) {
    Lexer lexer(expression);
    std::string result = "";
    Token tok = lexer.next();
    while(tok.type() != lexem_t::EOEX) {
        result += (tok.view() + "\n");
        tok = lexer.next();
    }
    return result;
}
#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <vector>
#include <map>

// Базовый класс для всех узлов AST
class ExprNode {
public:
    virtual ~ExprNode() = default;
    virtual std::string toString() const = 0;
    virtual double evaluate(const std::map<std::string, double>& vars) const = 0;
    virtual std::string toTree(int depth = 0) const = 0;
    virtual std::unique_ptr<ExprNode> derivative(const std::string& var) const = 0;
    virtual std::unique_ptr<ExprNode> clone() const = 0;
};

// Число (лист)
class NumberNode : public ExprNode {
    double value_;
public:
    NumberNode(double value);
    std::string toString() const override;
    double value() const;
    std::string toTree(int depth = 0) const override;
    double evaluate(const std::map<std::string, double>& vars) const override;
    std::unique_ptr<ExprNode> derivative(const std::string& var) const override;
    std::unique_ptr<ExprNode> clone() const override;
};

// Переменная (лист)
class VariableNode : public ExprNode {
    std::string name_;
public:
    VariableNode(const std::string& name);
    std::string toString() const override;
    std::string name() const;
    std::string toTree(int depth = 0) const override;
    double evaluate(const std::map<std::string, double>& vars) const override;
    std::unique_ptr<ExprNode> derivative(const std::string& var) const override;
    std::unique_ptr<ExprNode> clone() const override;

};

// Бинарная операция (узел)
class BinaryOpNode : public ExprNode {
    char op_;
    std::unique_ptr<ExprNode> left_;
    std::unique_ptr<ExprNode> right_;
public:
    BinaryOpNode(char op, std::unique_ptr<ExprNode> left, std::unique_ptr<ExprNode> right);
    std::string toString() const override;
    char op() const;
    ExprNode* left() const;
    ExprNode* right() const;
    std::string toTree(int depth = 0) const override;
    double evaluate(const std::map<std::string, double>& vars) const override;
    std::unique_ptr<ExprNode> derivative(const std::string& var) const override;
    std::unique_ptr<ExprNode> clone() const override;
};

// Унарная операция (например, -x)
class UnaryOpNode : public ExprNode {
    char op_;
    std::unique_ptr<ExprNode> operand_;
public:
    UnaryOpNode(char op, std::unique_ptr<ExprNode> operand);
    std::string toString() const override;
    char op() const;
    ExprNode* operand() const;
    std::string toTree(int depth = 0) const override;
    double evaluate(const std::map<std::string, double>& vars) const override;
    std::unique_ptr<ExprNode> derivative(const std::string& var) const override;
    std::unique_ptr<ExprNode> clone() const override;
};

class FunctionNode : public ExprNode {
    std::string name_;
    std::vector<std::unique_ptr<ExprNode>> arguments_;
public:
    FunctionNode(const std::string& name, std::vector<std::unique_ptr<ExprNode>> args);
    std::string toString() const override;
    const std::string& name() const;
    size_t argCount() const;
    const ExprNode* getArg(size_t i) const;
    std::string toTree(int depth = 0) const override;
    double evaluate(const std::map<std::string, double>& vars) const override;
    std::unique_ptr<ExprNode> derivative(const std::string& var) const override;
    std::unique_ptr<ExprNode> clone() const override;
};

#endif#include "ast.h"
#include <string>
#include <limits>
#include <sstream>
#include <sstream> 
#include <cmath>
#include <iostream>

// Функция для красивого вывода чисел без лишних нулей
std::string formatDouble(double value) {
    std::ostringstream oss;
    oss << value;
    std::string str = oss.str();
    size_t e_pos = str.find('e');
    if (e_pos != std::string::npos && str[e_pos + 1] == '+') {
        str.erase(e_pos + 1, 1);
    }
    // Убираем лишние нули в конце, если есть точка
    size_t dot_pos = str.find('.');
    if (dot_pos != std::string::npos) {
        // Удаляем trailing zeros
        while (str.size() > dot_pos + 1 && str.back() == '0') {
            str.pop_back();
        }
        // Если после точки ничего не осталось, убираем точку
        if (str.back() == '.') {
            str.pop_back();
        }
    }
    return str;
}
// NumberNode
NumberNode::NumberNode(double value) 
    : value_(value) 
{}

std::string NumberNode::toString() const { 
    return formatDouble(value_);
}

double NumberNode::value() const { 
    return value_; 
}

double NumberNode::evaluate(const std::map<std::string, double>& vars) const {
    return value_;
}

std::string NumberNode::toTree(int depth) const {
    return std::string(depth * 2, ' ') + formatDouble(value_);
}

std::unique_ptr<ExprNode> NumberNode::derivative(const std::string& var) const {
    return std::make_unique<NumberNode>(0.0);
}

std::unique_ptr<ExprNode> NumberNode::clone() const {
    return std::make_unique<NumberNode>(value_);
}

// VariableNode
VariableNode::VariableNode(const std::string& name) 
    : name_(name) 
{}

std::string VariableNode::toString() const { 
    return name_; 
}

std::string VariableNode::name() const { 
    return name_; 
}

double VariableNode::evaluate(const std::map<std::string, double>& vars) const {
    auto it = vars.find(name_);
    if (it == vars.end()) {
        throw std::runtime_error("Unknown variable: " + name_);
    }
    return it->second;
}

std::string VariableNode::toTree(int depth) const {
    return std::string(depth * 2, ' ') + name_;
}

std::unique_ptr<ExprNode> VariableNode::derivative(const std::string& var) const {
    if (name_ == var) {
        return std::make_unique<NumberNode>(1.0);
    }
    return std::make_unique<NumberNode>(0.0);
}

std::unique_ptr<ExprNode> VariableNode::clone() const {
    return std::make_unique<VariableNode>(name_);
}

// BinaryOpNode
BinaryOpNode::BinaryOpNode(char op, std::unique_ptr<ExprNode> left, std::unique_ptr<ExprNode> right)
    : op_(op), left_(std::move(left)), right_(std::move(right)) 
{}

std::string BinaryOpNode::toString() const {
    return "(" + left_->toString() + " " + op_ + " " + right_->toString() + ")";
}

char BinaryOpNode::op() const { 
    return op_; 
}

ExprNode* BinaryOpNode::left() const { 
    return left_.get(); 
}

ExprNode* BinaryOpNode::right() const { 
    return right_.get(); 
}

double BinaryOpNode::evaluate(const std::map<std::string, double>& vars) const {
    double left_val = left_->evaluate(vars);
    double right_val = right_->evaluate(vars);
    
    switch (op_) {
        case '+': return left_val + right_val;
        case '-': return left_val - right_val;
        case '*': return left_val * right_val;
        case '/': 
    if (right_val == 0) {
        return std::numeric_limits<double>::infinity();
    }
    return left_val / right_val;
        case '^': return std::pow(left_val, right_val);
        default: throw std::runtime_error("Unknown operator: " + std::string(1, op_));
    }
}

std::string BinaryOpNode::toTree(int depth) const {
    std::string result = std::string(depth * 2, ' ') + std::string(1, op_) + "\n";
    result += left_->toTree(depth + 1) + "\n";
    result += right_->toTree(depth + 1);
    return result;
}

std::unique_ptr<ExprNode> BinaryOpNode::derivative(const std::string& var) const {
    auto left_deriv = left_->derivative(var);
    auto right_deriv = right_->derivative(var);
    
    switch (op_) {
        case '+':
            return std::make_unique<BinaryOpNode>('+', std::move(left_deriv), std::move(right_deriv));
        case '-':
            return std::make_unique<BinaryOpNode>('-', std::move(left_deriv), std::move(right_deriv));
       case '*': {
            auto u = left_->clone();
            auto v = right_->clone();
            auto u_deriv = left_->derivative(var);
            auto v_deriv = right_->derivative(var);
            
            // Проверяем на ноль
            bool u_deriv_zero = false;
            bool v_deriv_zero = false;
            
            if (auto num = dynamic_cast<NumberNode*>(u_deriv.get())) {
                if (num->value() == 0.0) u_deriv_zero = true;
            }
            if (auto num = dynamic_cast<NumberNode*>(v_deriv.get())) {
                if (num->value() == 0.0) v_deriv_zero = true;
            }
            
            if (u_deriv_zero && v_deriv_zero) {
                return std::make_unique<NumberNode>(0.0);
            }
            else if (u_deriv_zero) {
                return std::make_unique<BinaryOpNode>('*', std::move(u), std::move(v_deriv));
            }
            else if (v_deriv_zero) {
                return std::make_unique<BinaryOpNode>('*', std::move(u_deriv), std::move(v));
            }
            
            auto term1 = std::make_unique<BinaryOpNode>('*', std::move(u_deriv), std::move(v));
            auto term2 = std::make_unique<BinaryOpNode>('*', std::move(u), std::move(v_deriv));
            return std::make_unique<BinaryOpNode>('+', std::move(term1), std::move(term2));
        }
        case '/': {
            // (u/v)' = (u'*v - u*v')/v^2
            auto term1 = std::make_unique<BinaryOpNode>('*', left_->derivative(var), right_->clone());
            auto term2 = std::make_unique<BinaryOpNode>('*', left_->clone(), right_->derivative(var));
            auto numerator = std::make_unique<BinaryOpNode>('-', std::move(term1), std::move(term2));
            auto denominator = std::make_unique<BinaryOpNode>('^', right_->clone(), std::make_unique<NumberNode>(2));
            return std::make_unique<BinaryOpNode>('/', std::move(numerator), std::move(denominator));
        }
        case '^': {
            if (auto num = dynamic_cast<NumberNode*>(right_.get())) {
                double c = num->value();
                
                auto u_deriv = left_->derivative(var);
                
                if (auto zero = dynamic_cast<NumberNode*>(u_deriv.get())) {
                    if (zero->value() == 0.0) {
                        return std::make_unique<NumberNode>(0.0);
                    }
                }
                
                auto power = std::make_unique<NumberNode>(c - 1);
                auto u_pow = std::make_unique<BinaryOpNode>('^', left_->clone(), std::move(power));
                auto mult = std::make_unique<BinaryOpNode>('*', std::move(u_deriv), std::move(u_pow));
                return std::make_unique<BinaryOpNode>('*', std::make_unique<NumberNode>(c), std::move(mult));
            }
            
            throw std::runtime_error("Derivative of variable exponent not implemented");
        }
        default:
            throw std::runtime_error("Unknown operator");
    }
}

std::unique_ptr<ExprNode> BinaryOpNode::clone() const {
    return std::make_unique<BinaryOpNode>(op_, left_->clone(), right_->clone());
}

// UnaryOpNode
UnaryOpNode::UnaryOpNode(char op, std::unique_ptr<ExprNode> operand)
    : op_(op), operand_(std::move(operand)) 
{}

std::string UnaryOpNode::toString() const {
    return "(" + std::string(1, op_) + operand_->toString() + ")";
}

char UnaryOpNode::op() const { 
    return op_; 
}

ExprNode* UnaryOpNode::operand() const { 
    return operand_.get(); 
}

double UnaryOpNode::evaluate(const std::map<std::string, double>& vars) const {
    double val = operand_->evaluate(vars);
    
    if (op_ == '+') {
        return val;
    } else if (op_ == '-') {
        return -val;
    }
    throw std::runtime_error("Unknown unary operator: " + std::string(1, op_));
}

std::string UnaryOpNode::toTree(int depth) const {
    std::string result = std::string(depth * 2, ' ') + std::string(1, op_) + "\n";
    result += operand_->toTree(depth + 1);
    return result;
}

std::unique_ptr<ExprNode> UnaryOpNode::derivative(const std::string& var) const {
    if (op_ == '+') {
        return operand_->derivative(var);
    } else if (op_ == '-') {
        auto deriv = operand_->derivative(var);
        return std::make_unique<UnaryOpNode>('-', std::move(deriv));
    }
    throw std::runtime_error("Unknown unary operator");
}

std::unique_ptr<ExprNode> UnaryOpNode::clone() const {
    return std::make_unique<UnaryOpNode>(op_, operand_->clone());
}


// FunctionNode
FunctionNode::FunctionNode(const std::string& name, std::vector<std::unique_ptr<ExprNode>> args)
    : name_(name), arguments_(std::move(args)) 
{}

std::string FunctionNode::toString() const {
    std::string result = name_ + "(";
    for (size_t i = 0; i < arguments_.size(); ++i) {
        if (i > 0) result += ", ";
        result += arguments_[i]->toString();
    }
    return result + ")";
}

const std::string& FunctionNode::name() const { 
    return name_; 
}

size_t FunctionNode::argCount() const { 
    return arguments_.size(); 
}

const ExprNode* FunctionNode::getArg(size_t i) const {
    return (i < arguments_.size()) ? arguments_[i].get() : nullptr;
}

double FunctionNode::evaluate(const std::map<std::string, double>& vars) const {
    // Вычисляем аргументы
    std::vector<double> args;
    for (const auto& arg : arguments_) {
        args.push_back(arg->evaluate(vars));
    }
    
    // Вызываем соответствующую функцию
    if (name_ == "sin") {
        if (args.size() != 1) throw std::runtime_error("sin expects 1 argument");
        return std::sin(args[0]);
    }
    else if (name_ == "cos") {
        if (args.size() != 1) throw std::runtime_error("cos expects 1 argument");
        return std::cos(args[0]);
    }
    else if (name_ == "tan" || name_ == "tg") {
        if (args.size() != 1) throw std::runtime_error("tan expects 1 argument");
        return std::tan(args[0]);
    }
    else if (name_ == "log") {
        if (args.size() != 1) throw std::runtime_error("log expects 1 argument");
        if (args[0] < 0) throw std::runtime_error("log argument must be non-negative");
        if (args[0] == 0) return -std::numeric_limits<double>::infinity();
        return std::log(args[0]);
    }
    else if (name_ == "exp") {
        if (args.size() != 1) throw std::runtime_error("exp expects 1 argument");
        return std::exp(args[0]);
    }
    else if (name_ == "sqrt") {
        if (args.size() != 1) throw std::runtime_error("sqrt expects 1 argument");
        if (args[0] < 0) throw std::runtime_error("sqrt argument must be non-negative");
        return std::sqrt(args[0]);
    }
    else if (name_ == "^") {
        if (args.size() != 2) throw std::runtime_error("pow expects 2 arguments");
        return std::pow(args[0], args[1]);
    }
   else if (name_ == "ln") {
        if (args.size() != 1) throw std::runtime_error("ln expects 1 argument");
        if (args[0] <= 0) throw std::runtime_error("ln argument must be positive");
        return std::log(args[0]);
    } 
    else {
        throw std::runtime_error("Unknown function: " + name_);
    }
}

std::unique_ptr<ExprNode> FunctionNode::derivative(const std::string& var) const {
    if (arguments_.size() != 1) {
        throw std::runtime_error("Derivative of function with multiple args not implemented");
    }
    
    auto arg_deriv = arguments_[0]->derivative(var);
    
    // Создаём вектор для аргументов
    std::vector<std::unique_ptr<ExprNode>> args;
    
    if (name_ == "sin") {
        // sin(u)' = cos(u) * u'
        args.push_back(arguments_[0]->clone());
        auto cos_u = std::make_unique<FunctionNode>("cos", std::move(args));
        return std::make_unique<BinaryOpNode>('*', std::move(cos_u), std::move(arg_deriv));
    }
    else if (name_ == "cos") {
        // cos(u)' = -sin(u) * u'
        args.push_back(arguments_[0]->clone());
        auto sin_u = std::make_unique<FunctionNode>("sin", std::move(args));
        auto minus_sin = std::make_unique<UnaryOpNode>('-', std::move(sin_u));
        return std::make_unique<BinaryOpNode>('*', std::move(minus_sin), std::move(arg_deriv));
    }
    else if (name_ == "exp") {
        // exp(u)' = exp(u) * u'
        args.push_back(arguments_[0]->clone());
        auto exp_u = std::make_unique<FunctionNode>("exp", std::move(args));
        return std::make_unique<BinaryOpNode>('*', std::move(exp_u), std::move(arg_deriv));
    }
    else if (name_ == "ln" || name_ == "log") {
        // ln(u)' = u' / u
        auto div = std::make_unique<BinaryOpNode>('/', std::move(arg_deriv), arguments_[0]->clone());
        return div;
    }
    else if (name_ == "sqrt") {
        // sqrt(u)' = u' / (2 * sqrt(u))
        args.push_back(arguments_[0]->clone());
        auto sqrt_u = std::make_unique<FunctionNode>("sqrt", std::move(args));
        
        auto two = std::make_unique<NumberNode>(2.0);
        auto denominator = std::make_unique<BinaryOpNode>('*', std::move(two), std::move(sqrt_u));
        auto result = std::make_unique<BinaryOpNode>('/', std::move(arg_deriv), std::move(denominator));
        return result;
    }
    
    throw std::runtime_error("Derivative of function " + name_ + " not implemented");
}

std::unique_ptr<ExprNode> FunctionNode::clone() const {
    std::vector<std::unique_ptr<ExprNode>> cloned_args;
    for (const auto& arg : arguments_) {
        cloned_args.push_back(arg->clone());
    }
    return std::make_unique<FunctionNode>(name_, std::move(cloned_args));
}

std::string FunctionNode::toTree(int depth) const {
    std::string result = std::string(depth * 2, ' ') + name_ + "()\n";
    for (const auto& arg : arguments_) {
        result += arg->toTree(depth + 1) + "\n";
    }
    // Убираем последний лишний перевод строки, если есть
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}
#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <functional>
#include <map>
#include "lexer.h"
#include "ast.h"

class Parser {
private:
    Lexer& lexer;
    Token currentToken;
    
    void nextToken();
    bool match(lexem_t type);
    void expect(lexem_t type);
    
    bool isFunctionName(const std::string& name) {
        static const std::vector<std::string> functions = {
            "sin", "cos", "tg", "ctg", "ln", "sqrt", "log", "exp", "pow"
        };
        for (const auto& func : functions) {
            if (name == func) return true;
        }
        return false;
    }

    std::unique_ptr<ExprNode> parseExpression();
    std::unique_ptr<ExprNode> parseAddSub();
    std::unique_ptr<ExprNode> parseMulDiv();
    std::unique_ptr<ExprNode> parseUnary();
    std::unique_ptr<ExprNode> parsePower();
    std::unique_ptr<ExprNode> parsePrimary();
    std::unique_ptr<ExprNode> parseFunctionCall(const std::string& name);
    
public:
    Parser(Lexer& lex);
    std::unique_ptr<ExprNode> parse();
};

#endif
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
#include <iostream>
#include <string>
#include <memory>
#include <sstream>
#include <map>




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
        std::cout << "ERROR" << std::endl;
    }
}

int main() {
    std::string mode;
    std::getline(std::cin, mode);
    
    if (mode == "evaluate") {
        runEvaluate();
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
            std::cout << "ERROR" << std::endl;
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
            std::cout << "ERROR" << std::endl;
        }
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
            std::cout << "ERROR" << std::endl;
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
            std::cout << "ERROR" << std::endl;
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
            std::cout << "ERROR" << std::endl;
        }
    }
    else {
        std::cout << "ERROR" << std::endl;
    }
    
    return 0;
}