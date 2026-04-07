#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <cctype>
#include <stdexcept>

enum class lexem_t {
    IDENT,      
    NUMBER,     
    OPERATOR,  
    LPAREN,   
    RPAREN,    
    EOEX
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

#endif