#include "ast.h"
#include <string>
#include <limits>
#include <sstream>
#include <sstream> 
#include <cmath>
#include <iostream>

std::string formatDouble(double value) {
    std::ostringstream oss;
    oss << value;
    std::string str = oss.str();
    size_t e_pos = str.find('e');
    if (e_pos != std::string::npos && str[e_pos + 1] == '+') {
        str.erase(e_pos + 1, 1);
    }
    size_t dot_pos = str.find('.');
    if (dot_pos != std::string::npos) {
        while (str.size() > dot_pos + 1 && str.back() == '0') {
            str.pop_back();
        }
        if (str.back() == '.') {
            str.pop_back();
        }
    }
    return str;
}

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
            auto term1 = std::make_unique<BinaryOpNode>('*', left_->derivative(var), right_->clone());
            auto term2 = std::make_unique<BinaryOpNode>('*', left_->clone(), right_->derivative(var));
            auto numerator = std::make_unique<BinaryOpNode>('-', std::move(term1), std::move(term2));
            auto denominator = std::make_unique<BinaryOpNode>('^', right_->clone(), std::make_unique<NumberNode>(2));
            return std::make_unique<BinaryOpNode>('/', std::move(numerator), std::move(denominator));
        }
        case '^': {
    auto u = left_->clone();
    auto v = right_->clone();
    auto u_deriv = left_->derivative(var);
    
    if (auto num = dynamic_cast<NumberNode*>(right_.get())) {
        double c = num->value();
        
        bool isZero = false;
        if (auto zero = dynamic_cast<NumberNode*>(u_deriv.get())) {
            if (zero->value() == 0.0) isZero = true;
        }
        
        if (isZero) {
            return std::make_unique<NumberNode>(0.0);
        }
        
        auto power = std::make_unique<NumberNode>(c - 1);
        auto u_pow = std::make_unique<BinaryOpNode>('^', left_->clone(), std::move(power));
        auto mult = std::make_unique<BinaryOpNode>('*', std::move(u_deriv), std::move(u_pow));
        return std::make_unique<BinaryOpNode>('*', std::make_unique<NumberNode>(c), std::move(mult));
    }
    
    auto v_deriv = right_->derivative(var);
    
    std::vector<std::unique_ptr<ExprNode>> ln_args;
    ln_args.push_back(left_->clone());
    auto ln_u = std::make_unique<FunctionNode>("ln", std::move(ln_args));
    
    auto u_pow_v = std::make_unique<BinaryOpNode>('^', left_->clone(), right_->clone());
    
    auto term1 = std::make_unique<BinaryOpNode>('*', std::move(v_deriv), std::move(ln_u));
    
    auto v_mul_u_deriv = std::make_unique<BinaryOpNode>('*', std::move(v), std::move(u_deriv));
    auto term2 = std::make_unique<BinaryOpNode>('/', std::move(v_mul_u_deriv), left_->clone());
    
    auto sum = std::make_unique<BinaryOpNode>('+', std::move(term1), std::move(term2));
    
    return std::make_unique<BinaryOpNode>('*', std::move(u_pow_v), std::move(sum));
}
        default:
            throw std::runtime_error("Unknown operator");
    }
}

std::unique_ptr<ExprNode> BinaryOpNode::clone() const {
    return std::make_unique<BinaryOpNode>(op_, left_->clone(), right_->clone());
}

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
        if (name_ == "exp" && arguments_.size() == 1) {
            return "e^(" + arguments_[0]->toString() + ")";
        }
        
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
    std::vector<double> args;
    for (const auto& arg : arguments_) {
        args.push_back(arg->evaluate(vars));
    }
    
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
    
    std::vector<std::unique_ptr<ExprNode>> args;
    
    if (name_ == "sin") {
        args.push_back(arguments_[0]->clone());
        auto cos_u = std::make_unique<FunctionNode>("cos", std::move(args));
        return std::make_unique<BinaryOpNode>('*', std::move(cos_u), std::move(arg_deriv));
    }
    else if (name_ == "cos") {
        args.push_back(arguments_[0]->clone());
        auto sin_u = std::make_unique<FunctionNode>("sin", std::move(args));
        auto minus_sin = std::make_unique<UnaryOpNode>('-', std::move(sin_u));
        return std::make_unique<BinaryOpNode>('*', std::move(minus_sin), std::move(arg_deriv));
    }
    else if (name_ == "exp") {
        args.push_back(arguments_[0]->clone());
        auto exp_u = std::make_unique<FunctionNode>("exp", std::move(args));
        return std::make_unique<BinaryOpNode>('*', std::move(exp_u), std::move(arg_deriv));
    }
    else if (name_ == "ln" || name_ == "log") {
        auto div = std::make_unique<BinaryOpNode>('/', std::move(arg_deriv), arguments_[0]->clone());
        return div;
    }
    else if (name_ == "sqrt") {
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
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}