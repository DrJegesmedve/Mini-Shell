#define _USE_MATH_DEFINES

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <stack>
#include <queue>
#include <map>
#include <algorithm>
#include <cctype>
#include <limits>
#include "math.h" // Contains the Module interface (e.g. Module class definition)

//------------------------------------------------------------
// Expression Classes
//------------------------------------------------------------

// Base Expression – now includes evaluateWithXY and evaluateWithXYZ
class Expression {
public:
    virtual ~Expression() {}
    virtual double evaluate() = 0;
    virtual double evaluateWithX(double x) { return evaluate(); }
    virtual double evaluateWithXY(double x, double y) { return evaluate(); }
    virtual double evaluateWithXYZ(double x, double y, double z) { return evaluate(); }
};

// Multivariable expression (for graphing)
class MultiVarExpression : public Expression {
public:
    MultiVarExpression() {}
    virtual ~MultiVarExpression() {}
    double evaluate() override { return 0; } // Default
    virtual double evaluateWithX(double x) override { return evaluateWithXY(x, 0); }
    virtual double evaluateWithXY(double x, double y) { return 0; }
    virtual double evaluateWithXYZ(double x, double y, double z) { return 0; }
};

// Variable expressions
class VariableXExpression : public MultiVarExpression {
public:
    double evaluateWithX(double x) override { return x; }
    double evaluateWithXY(double x, double y) override { return x; }
    double evaluateWithXYZ(double x, double y, double z) override { return x; }
};

class VariableYExpression : public MultiVarExpression {
public:
    double evaluate() override { return 0; }
    double evaluateWithX(double x) override { return 0; }
    double evaluateWithXY(double x, double y) override { return y; }
    double evaluateWithXYZ(double x, double y, double z) override { return y; }
};

class VariableZExpression : public MultiVarExpression {
public:
    double evaluate() override { return 0; }
    double evaluateWithX(double x) override { return 0; }
    double evaluateWithXY(double x, double y) override { return 0; }
    double evaluateWithXYZ(double x, double y, double z) override { return z; }
};

class ParameterTExpression : public MultiVarExpression {
public:
    double evaluateWithX(double t) override { return t; }
    double evaluateWithXY(double x, double y) override { return x; } // Here x is treated as the parameter
};

// Number
class NumberExpression : public MultiVarExpression {
public:
    NumberExpression(double value) : m_value(value) {}
    double evaluate() override { return m_value; }
    double evaluateWithX(double x) override { return m_value; }
    double evaluateWithXY(double x, double y) override { return m_value; }
    double evaluateWithXYZ(double x, double y, double z) override { return m_value; }
private:
    double m_value;
};

//------------------------------------------------------------
// Binary and Unary Operations
//------------------------------------------------------------

// Binary expression in a multivariable context
class BinaryExpression : public MultiVarExpression {
public:
    BinaryExpression(Expression* left, Expression* right)
        : m_left(left), m_right(right) {
    }
    virtual ~BinaryExpression() {
        delete m_left;
        delete m_right;
    }
    double evaluateWithX(double x) override {
        double l = dynamic_cast<MultiVarExpression*>(m_left) ?
            dynamic_cast<MultiVarExpression*>(m_left)->evaluateWithX(x) :
            m_left->evaluateWithX(x);
        double r = dynamic_cast<MultiVarExpression*>(m_right) ?
            dynamic_cast<MultiVarExpression*>(m_right)->evaluateWithX(x) :
            m_right->evaluateWithX(x);
        return evaluateOperation(l, r);
    }
    double evaluate() override {
        return evaluateOperation(m_left->evaluate(), m_right->evaluate());
    }
    double evaluateWithXY(double x, double y) override {
        double l = dynamic_cast<MultiVarExpression*>(m_left) ?
            dynamic_cast<MultiVarExpression*>(m_left)->evaluateWithXY(x, y) :
            m_left->evaluateWithX(x);
        double r = dynamic_cast<MultiVarExpression*>(m_right) ?
            dynamic_cast<MultiVarExpression*>(m_right)->evaluateWithXY(x, y) :
            m_right->evaluateWithX(x);
        return evaluateOperation(l, r);
    }
    double evaluateWithXYZ(double x, double y, double z) override {
        double l = dynamic_cast<MultiVarExpression*>(m_left) ?
            dynamic_cast<MultiVarExpression*>(m_left)->evaluateWithXYZ(x, y, z) :
            m_left->evaluateWithX(x);
        double r = dynamic_cast<MultiVarExpression*>(m_right) ?
            dynamic_cast<MultiVarExpression*>(m_right)->evaluateWithXYZ(x, y, z) :
            m_right->evaluateWithX(x);
        return evaluateOperation(l, r);
    }
protected:
    virtual double evaluateOperation(double left, double right) = 0;
    Expression* m_left;
    Expression* m_right;
};

// Unary expression in a multivariable context
class UnaryExpression : public MultiVarExpression {
public:
    UnaryExpression(Expression* operand) : m_operand(operand) {}
    virtual ~UnaryExpression() { delete m_operand; }
    double evaluateWithX(double x) override {
        double val = dynamic_cast<MultiVarExpression*>(m_operand) ?
            dynamic_cast<MultiVarExpression*>(m_operand)->evaluateWithX(x) :
            m_operand->evaluateWithX(x);
        return evaluateOperation(val);
    }
    double evaluate() override {
        return evaluateOperation(m_operand->evaluate());
    }
    double evaluateWithXY(double x, double y) override {
        double val = dynamic_cast<MultiVarExpression*>(m_operand) ?
            dynamic_cast<MultiVarExpression*>(m_operand)->evaluateWithXY(x, y) :
            m_operand->evaluateWithX(x);
        return evaluateOperation(val);
    }
    double evaluateWithXYZ(double x, double y, double z) override {
        double val = dynamic_cast<MultiVarExpression*>(m_operand) ?
            dynamic_cast<MultiVarExpression*>(m_operand)->evaluateWithXYZ(x, y, z) :
            m_operand->evaluateWithX(x);
        return evaluateOperation(val);
    }
protected:
    virtual double evaluateOperation(double value) = 0;
    Expression* m_operand;
};

// Addition
class AddExpression : public BinaryExpression {
public:
    AddExpression(Expression* left, Expression* right)
        : BinaryExpression(left, right) {
    }
protected:
    double evaluateOperation(double left, double right) override { return left + right; }
};

// Subtraction
class SubtractExpression : public BinaryExpression {
public:
    SubtractExpression(Expression* left, Expression* right)
        : BinaryExpression(left, right) {
    }
protected:
    double evaluateOperation(double left, double right) override { return left - right; }
};

// Multiplication
class MultiplyExpression : public BinaryExpression {
public:
    MultiplyExpression(Expression* left, Expression* right)
        : BinaryExpression(left, right) {
    }
protected:
    double evaluateOperation(double left, double right) override { return left * right; }
};

// Division
class DivideExpression : public BinaryExpression {
public:
    DivideExpression(Expression* left, Expression* right)
        : BinaryExpression(left, right) {
    }
protected:
    double evaluateOperation(double left, double right) override { return left / right; }
};

// Power
class PowerExpression : public BinaryExpression {
public:
    PowerExpression(Expression* left, Expression* right)
        : BinaryExpression(left, right) {
    }
protected:
    double evaluateOperation(double left, double right) override { return std::pow(left, right); }
};

//------------------------------------------------------------
// Unary Functions
//------------------------------------------------------------
class SqrtExpression : public UnaryExpression {
public:
    SqrtExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return std::sqrt(value); }
};

class LnExpression : public UnaryExpression {
public:
    LnExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return std::log(value); }
};

class Log10Expression : public UnaryExpression {
public:
    Log10Expression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return std::log10(value); }
};

class LogBaseExpression : public UnaryExpression {
public:
    LogBaseExpression(Expression* operand, double base)
        : UnaryExpression(operand), m_base(base) {
    }
protected:
    double evaluateOperation(double value) override { return std::log(value) / std::log(m_base); }
private:
    double m_base;
};

class SinExpression : public UnaryExpression {
public:
    SinExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return std::sin(value); }
};

class CosExpression : public UnaryExpression {
public:
    CosExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return std::cos(value); }
};

class TanExpression : public UnaryExpression {
public:
    TanExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return std::tan(value); }
};

class CtgExpression : public UnaryExpression {
public:
    CtgExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return 1.0 / std::tan(value); }
};

class ArcsinExpression : public UnaryExpression {
public:
    ArcsinExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return std::asin(value); }
};

class ArccosExpression : public UnaryExpression {
public:
    ArccosExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return std::acos(value); }
};

class ArctanExpression : public UnaryExpression {
public:
    ArctanExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return std::atan(value); }
};

class ArcctgExpression : public UnaryExpression {
public:
    ArcctgExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return M_PI / 2.0 - std::atan(value); }
};

//------------------------------------------------------------
// NEW: Absolute Value Expression
//------------------------------------------------------------
class AbsExpression : public UnaryExpression {
public:
    AbsExpression(Expression* operand) : UnaryExpression(operand) {}
protected:
    double evaluateOperation(double value) override { return std::fabs(value); }
};

//------------------------------------------------------------
// Expression Parser
//------------------------------------------------------------
class ExpressionParser {
public:
    ExpressionParser(const std::string& expression)
        : m_expression(expression), m_pos(0), m_hasX(false), m_hasY(false), m_hasZ(false), m_hasT(false) {
    }

    Expression* parse() {
        removeWhitespace();
        Expression* expr = parseExpression();
        if (m_pos < m_expression.size()) {
            delete expr;
            return nullptr;
        }
        return expr;
    }

    bool hasX() const { return m_hasX; }
    bool hasY() const { return m_hasY; }
    bool hasZ() const { return m_hasZ; }
    bool hasT() const { return m_hasT; }

private:
    std::string m_expression;
    size_t m_pos;
    bool m_hasX, m_hasY, m_hasZ, m_hasT;

    void removeWhitespace() {
        std::string clean;
        for (char c : m_expression) {
            if (!std::isspace(c))
                clean.push_back(c);
        }
        m_expression = clean;
    }

    bool isEnd() const { return m_pos >= m_expression.size(); }
    char current() const { return isEnd() ? '\0' : m_expression[m_pos]; }
    void advance() { if (!isEnd()) m_pos++; }
    char peek() const { return (m_pos + 1 < m_expression.size()) ? m_expression[m_pos + 1] : '\0'; }

    Expression* parseExpression() { return parseAddSub(); }

    Expression* parseAddSub() {
        Expression* left = parseMulDiv();
        if (!left) return nullptr;
        while (!isEnd() && (current() == '+' || current() == '-')) {
            char op = current();
            advance();
            Expression* right = parseMulDiv();
            if (!right) { delete left; return nullptr; }
            if (op == '+')
                left = new AddExpression(left, right);
            else
                left = new SubtractExpression(left, right);
        }
        return left;
    }

    Expression* parseMulDiv() {
        Expression* left = parsePower();
        if (!left) return nullptr;
        while (!isEnd() && (current() == '*' || current() == '/')) {
            char op = current();
            advance();
            Expression* right = parsePower();
            if (!right) { delete left; return nullptr; }
            if (op == '*')
                left = new MultiplyExpression(left, right);
            else
                left = new DivideExpression(left, right);
        }
        return left;
    }

    Expression* parsePower() {
        Expression* left = parseFactor();
        if (!left) return nullptr;
        if (!isEnd() && current() == '^') {
            advance();
            Expression* right = parseFactor();
            if (!right) { delete left; return nullptr; }
            left = new PowerExpression(left, right);
        }
        return left;
    }

    Expression* parseFactor() {
        // Absolute value: |expression|
        if (current() == '|') {
            advance();
            Expression* expr = parseExpression();
            if (!expr || current() != '|') { delete expr; return nullptr; }
            advance();
            return new AbsExpression(expr);
        }

        // Number
        if (std::isdigit(current()) || (current() == '.' && std::isdigit(peek()))) {
            return parseNumber();
        }

        // Constants
        if (current() == 'e' && (isEnd() || !std::isalnum(peek()))) {
            advance();
            return new NumberExpression(std::exp(1.0));
        }
        if (current() == 'm' && peek() == '_' && (m_pos + 3 < m_expression.size()) && m_expression.substr(m_pos, 4) == "m_PI") {
            m_pos += 4;
            return new NumberExpression(M_PI);
        }

        // Variables: x, y, z, t
        if (current() == 'x' && (isEnd() || !std::isalnum(peek()))) {
            advance();
            m_hasX = true;
            return new VariableXExpression();
        }
        if (current() == 'y' && (isEnd() || !std::isalnum(peek()))) {
            advance();
            m_hasY = true;
            return new VariableYExpression();
        }
        if (current() == 'z' && (isEnd() || !std::isalnum(peek()))) {
            advance();
            m_hasZ = true;
            return new VariableZExpression();
        }
        if (current() == 't' && (isEnd() || !std::isalnum(peek()))) {
            advance();
            m_hasT = true;
            return new ParameterTExpression();
        }

        // Parentheses
        if (current() == '(') {
            advance();
            Expression* expr = parseExpression();
            if (!expr || current() != ')') { delete expr; return nullptr; }
            advance();
            return expr;
        }

        // Functions
        if (std::isalpha(current())) {
            std::string func;
            while (!isEnd() && (std::isalnum(current()) || current() == '_')) {
                func.push_back(current());
                advance();
            }
            // Special case: "V" for square root (sqrt)
            if (func == "V") {
                if (current() == '(') {
                    advance();
                    Expression* arg = parseExpression();
                    if (!arg || current() != ')') { delete arg; return nullptr; }
                    advance();
                    return new SqrtExpression(arg);
                }
                return nullptr;
            }
            // Logarithm with given base, e.g. log2(x)
            if (func.size() > 3 && func.substr(0, 3) == "log") {
                try {
                    double base = std::stod(func.substr(3));
                    if (current() == '(') {
                        advance();
                        Expression* arg = parseExpression();
                        if (!arg || current() != ')') { delete arg; return nullptr; }
                        advance();
                        return new LogBaseExpression(arg, base);
                    }
                }
                catch (const std::invalid_argument&) {
                    // Failed conversion, continue.
                }
            }
            // Other functions if followed by '('
            if (current() == '(') {
                advance();
                Expression* arg = parseExpression();
                if (!arg || current() != ')') { delete arg; return nullptr; }
                advance();
                if (func == "sin") return new SinExpression(arg);
                else if (func == "cos") return new CosExpression(arg);
                else if (func == "tan") return new TanExpression(arg);
                else if (func == "ctg") return new CtgExpression(arg);
                else if (func == "arcsin") return new ArcsinExpression(arg);
                else if (func == "arccos") return new ArccosExpression(arg);
                else if (func == "arctan") return new ArctanExpression(arg);
                else if (func == "arcctg") return new ArcctgExpression(arg);
                else if (func == "ln") return new LnExpression(arg);
                else if (func == "lg") return new Log10Expression(arg);
                else { delete arg; return nullptr; }
            }
            return nullptr;
        }
        return nullptr;
    }

    Expression* parseNumber() {
        size_t start = m_pos;
        bool hasDecimal = false;
        if (current() == '-' || current() == '+') {
            advance();
        }
        while (!isEnd() && (std::isdigit(current()) || (current() == '.' && !hasDecimal))) {
            if (current() == '.') hasDecimal = true;
            advance();
        }
        double value = std::stod(m_expression.substr(start, m_pos - start));
        return new NumberExpression(value);
    }
};

//------------------------------------------------------------
// Graph Drawing
//------------------------------------------------------------

// 1D graph for functions of x only, with automatic y-range adjustment.
void drawGraph1D(Expression* expr) {
    double xMin = -10.0;
    double xMax = 10.0;
    const int width = 80;
    const int height = 25;

    std::vector<std::string> grid(height, std::string(width, ' '));

    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    for (int i = 0; i < width; i++) {
        double x = xMin + i * (xMax - xMin) / (width - 1);
        double y = expr->evaluateWithX(x);
        yMin = std::min(yMin, y);
        yMax = std::max(yMax, y);
    }
    if (yMin == yMax) { yMin -= 1; yMax += 1; }

    for (int i = 0; i < width; i++) {
        double x = xMin + i * (xMax - xMin) / (width - 1);
        double y = expr->evaluateWithX(x);
        int row = static_cast<int>((y - yMin) / (yMax - yMin) * (height - 1));
        row = height - 1 - row;
        if (row >= 0 && row < height) {
            grid[row][i] = '*';
        }
    }
    // Draw x-axis if 0 in range.
    if (yMin <= 0 && yMax >= 0) {
        int xAxisRow = height - 1 - static_cast<int>((0 - yMin) / (yMax - yMin) * (height - 1));
        for (int i = 0; i < width; i++) {
            if (grid[xAxisRow][i] == ' ')
                grid[xAxisRow][i] = '-';
        }
    }
    // Draw y-axis if 0 in range.
    if (xMin <= 0 && xMax >= 0) {
        int yAxisCol = static_cast<int>((0 - xMin) / (xMax - xMin) * (width - 1));
        for (int i = 0; i < height; i++) {
            if (grid[i][yAxisCol] == ' ')
                grid[i][yAxisCol] = '|';
        }
    }
    for (const auto& line : grid)
        std::cout << line << std::endl;
}

// Implicit function graph for functions of x and y (f(x,y)=0)
void drawGraphImplicit2D(Expression* expr) {
    double xMin = -10.0, xMax = 10.0;
    double yMin = -10.0, yMax = 10.0;
    const int width = 80, height = 25;
    std::vector<std::string> grid(height, std::string(width, ' '));

    const double threshold = 0.5; // threshold for considering f(x,y) near zero
    for (int i = 0; i < width; i++) {
        double x = xMin + i * (xMax - xMin) / (width - 1);
        for (int j = 0; j < height; j++) {
            double y = yMax - j * (yMax - yMin) / (height - 1);
            double val = expr->evaluateWithXY(x, y);
            if (std::fabs(val) < threshold) {
                grid[j][i] = '*';
            }
        }
    }
    // Draw x-axis
    if (yMin <= 0 && yMax >= 0) {
        int xAxisRow = static_cast<int>((yMax - 0) / (yMax - yMin) * (height - 1));
        for (int i = 0; i < width; i++) {
            if (grid[xAxisRow][i] == ' ')
                grid[xAxisRow][i] = '-';
        }
    }
    // Draw y-axis
    if (xMin <= 0 && xMax >= 0) {
        int yAxisCol = static_cast<int>((0 - xMin) / (xMax - xMin) * (width - 1));
        for (int j = 0; j < height; j++) {
            if (grid[j][yAxisCol] == ' ')
                grid[j][yAxisCol] = '|';
        }
    }
    for (const auto& line : grid)
        std::cout << line << std::endl;
}

// Basic 3D graph projection using isometric projection.
void drawGraph3D(Expression* expr) {
    std::cout << "3D graphing: Basic 3D projection (not fully implemented)." << std::endl;
    const int width = 80, height = 25;
    std::vector<std::string> grid(height, std::string(width, ' '));
    double xMin = -5, xMax = 5, yMin = -5, yMax = 5, zMin = -5, zMax = 5;

    const double threshold = 0.5;
    for (int i = 0; i < width; i++) {
        double x = xMin + i * (xMax - xMin) / (width - 1);
        for (int j = 0; j < height; j++) {
            double y = yMax - j * (yMax - yMin) / (height - 1);
            double z = 0; // For simplicity, using z=0 slice
            double val = expr->evaluateWithXYZ(x, y, z);
            if (std::fabs(val) < threshold) {
                grid[j][i] = '*';
            }
        }
    }
    for (const auto& line : grid)
        std::cout << line << std::endl;
}

//------------------------------------------------------------
// Math Module Implementation
//------------------------------------------------------------
class MathModule : public Module {
public:
    MathModule() {}
    ~MathModule() {}

    // The execute() method supports:
    // 1. "?" command: display detailed help text.
    // 2. "graph" command: draws the graph for the given expression.
    //    - If only x variable is used: 1D graph.
    //    - If x and y variables: implicit graph f(x,y)=0.
    //    - If x, y, and z variables: basic 3D projection.
    // 3. Otherwise: evaluate the expression (old method).
    void execute(const std::vector<std::string>& args) override {
        if (args.empty()) {
            std::cerr << "Usage:" << std::endl;
            std::cerr << "  help[/h/?]               - Display detailed help" << std::endl;
            std::cerr << "  graph <expression>  - Draw graph of the expression" << std::endl;
            std::cerr << "  <expression>        - Evaluate the expression" << std::endl;
            return;
        }

        // Help command: ?
        if (args[0] == "help" || args[0] == "h" || args[0] == "?") {
            std::cout << "Math Module Help - Detailed Description:" << std::endl;
            std::cout << "Supported operators: +, -, *, /, ^" << std::endl;
            std::cout << "Supported functions:" << std::endl;
            std::cout << "  sin(x), cos(x), tan(x), ctg(x)" << std::endl;
            std::cout << "  arcsin(x), arccos(x), arctan(x), arcctg(x)" << std::endl;
            std::cout << "  ln(x)   - natural logarithm" << std::endl;
            std::cout << "  lg(x)   - base-10 logarithm" << std::endl;
            std::cout << "  log<base>(x)  - logarithm with given base (e.g. log2(x))" << std::endl;
            std::cout << "  V(x)    - square root (alternative notation)" << std::endl;
            std::cout << "  |x|     - absolute value" << std::endl;
            std::cout << "Variables:" << std::endl;
            std::cout << "  x : independent variable" << std::endl;
            std::cout << "  y : secondary variable (for implicit functions)" << std::endl;
            std::cout << "  z : third variable (for 3D functions)" << std::endl;
            std::cout << "  t : parameter (for parametric functions)" << std::endl;
            std::cout << std::endl;
            std::cout << "Usage:" << std::endl;
            std::cout << "  To evaluate an expression, simply type it." << std::endl;
            std::cout << "  To graph an expression, use:" << std::endl;
            std::cout << "      graph <expression>" << std::endl;
            std::cout << "  The graph command automatically adjusts the view based on function values." << std::endl;
            std::cout << "  The module supports 2D graphs for explicit (y=f(x)) and implicit functions (f(x,y)=0)," << std::endl;
            std::cout << "  and basic 3D projection for functions of three variables." << std::endl;
            return;
        }

        // Graph mode: if first argument is "graph"
        if (args[0] == "graph") {
            if (args.size() < 2) {
                std::cerr << "Error: graph command requires an expression." << std::endl;
                return;
            }
            std::string exprStr;
            for (size_t i = 1; i < args.size(); i++) {
                if (!exprStr.empty()) exprStr += " ";
                exprStr += args[i];
            }
            ExpressionParser parser(exprStr);
            Expression* expr = parser.parse();
            if (!expr) {
                std::cerr << "Error: Failed to parse expression." << std::endl;
                return;
            }
            std::cout << "Drawing graph for: " << exprStr << std::endl;
            if (!parser.hasY() && !parser.hasZ()) {
                drawGraph1D(expr);
            }
            else if (parser.hasY() && !parser.hasZ()) {
                drawGraphImplicit2D(expr);
            }
            else if (parser.hasZ()) {
                drawGraph3D(expr);
            }
            delete expr;
            return;
        }

        // Default: evaluate the expression (old method)
        std::string exprStr;
        for (size_t i = 0; i < args.size(); i++) {
            if (!exprStr.empty()) exprStr += " ";
            exprStr += args[i];
        }
        ExpressionParser parser(exprStr);
        Expression* expr = parser.parse();
        if (!expr) {
            std::cerr << "Error: Failed to parse expression." << std::endl;
            return;
        }
        std::cout << std::fixed << std::setprecision(6) << expr->evaluate() << std::endl;
        delete expr;
    }

    std::string getVersion() const override {
        return "Math Module Version 1.1.0";
    }
};

// Exported function to create the module instance
extern "C" __declspec(dllexport) Module* createModule() {
    return new MathModule();
}
