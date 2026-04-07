#include "lexer.h"
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
    
    if (current() == '0') {
        result += current();
        advance();
        
        if (pos < input.length() && std::isdigit(current())) {
            pos = startPos;
            throw std::runtime_error("Invalid number: leading zeros not allowed");
        }
        
        if (current() == '.') {
            result += current();
            advance();
            
            bool hasDigits = false;
            while (pos < input.length() && std::isdigit(input[pos])) {
                result += input[pos];
                advance();
                hasDigits = true;
            }
            
            if (!hasDigits) {
                pos = startPos;
                throw std::runtime_error("Invalid number format");
            }
        }
    } else {
        while (pos < input.length() && std::isdigit(input[pos])) {
            result += input[pos];
            advance();
        }
        
        if (current() == '.') {
            result += current();
            advance();
            
            bool hasDigits = false;
            while (pos < input.length() && std::isdigit(input[pos])) {
                result += input[pos];
                advance();
                hasDigits = true;
            }
            
            if (!hasDigits) {
                pos = startPos;
                throw std::runtime_error("Invalid number format");
            }
        }
    }
    
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
    
    if (pos < input.length() && isIdentStart(input[pos])) {
        pos = startPos;
        throw std::runtime_error("Invalid token: number followed by letter");
    }
    
    return Token(lexem_t::NUMBER, result);
}

Token Lexer::readIdentifier() {
    std::string result;
    
    if (!isIdentStart(current())) {
        throw std::runtime_error("Identifier must start with a letter or underscore");
    }
    
    while (pos < input.length() && isIdentPart(input[pos])) {
        result += input[pos];
        advance();
    }
    
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
        return readNumber();
    } else if (isIdentStart(c)) {
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