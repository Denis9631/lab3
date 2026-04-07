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