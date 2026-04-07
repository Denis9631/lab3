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

#endif