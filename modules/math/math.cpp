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
#include "math.h"  // A modul interfészét tartalmazza (például a Module osztálydefinícióját)

//////////////////////////
// Kifejezés osztályok  //
//////////////////////////

// Absztrakt kifejezés
class Expression {
public:
    virtual ~Expression() {}
    virtual double evaluate() = 0;
    virtual double evaluateWithX(double x) { return evaluate(); }
};

// Többváltozós kifejezés (grafikhoz)
class MultiVarExpression : public Expression {
public:
    MultiVarExpression() {}
    virtual ~MultiVarExpression() {}
    double evaluate() override { return 0; } // Alapértelmezett érték
    virtual double evaluateWithX(double x) override { return evaluateWithXY(x, 0); }
    virtual double evaluateWithXY(double x, double y) { return 0; }
    virtual double evaluateWithXYZ(double x, double y, double z) { return 0; }
};

// Változók
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
    double evaluateWithXY(double x, double y) override { return x; } // itt x értelmezhető mint a paraméter
};

// Számok
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

///////////////////////////////
// Bináris és unáris műveletek //
///////////////////////////////

// Bináris kifejezés többváltozós környezetben
class BinaryExpression : public MultiVarExpression {
public:
    BinaryExpression(Expression* left, Expression* right) : m_left(left), m_right(right) {}
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

// Unáris kifejezés többváltozós környezetben
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

// Összeadás
class AddExpression : public BinaryExpression {
public:
    AddExpression(Expression* left, Expression* right) : BinaryExpression(left, right) {}
protected:
    double evaluateOperation(double left, double right) override { return left + right; }
};

// Kivonás
class SubtractExpression : public BinaryExpression {
public:
    SubtractExpression(Expression* left, Expression* right) : BinaryExpression(left, right) {}
protected:
    double evaluateOperation(double left, double right) override { return left - right; }
};

// Szorzás
class MultiplyExpression : public BinaryExpression {
public:
    MultiplyExpression(Expression* left, Expression* right) : BinaryExpression(left, right) {}
protected:
    double evaluateOperation(double left, double right) override { return left * right; }
};

// Osztás
class DivideExpression : public BinaryExpression {
public:
    DivideExpression(Expression* left, Expression* right) : BinaryExpression(left, right) {}
protected:
    double evaluateOperation(double left, double right) override { return left / right; }
};

// Hatványozás
class PowerExpression : public BinaryExpression {
public:
    PowerExpression(Expression* left, Expression* right) : BinaryExpression(left, right) {}
protected:
    double evaluateOperation(double left, double right) override { return std::pow(left, right); }
};

/////////////////////
// Unáris függvények
/////////////////////

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
    LogBaseExpression(Expression* operand, double base) : UnaryExpression(operand), m_base(base) {}
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

//////////////////////////////
// ExpressionParser (egyesített)
//////////////////////////////

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

    bool hasVariable() const { return m_hasX || m_hasY || m_hasZ || m_hasT; }
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

    Expression* parseExpression() {
        return parseAddSub();
    }

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
        // Szám
        if (std::isdigit(current()) || (current() == '.' && std::isdigit(peek()))) {
            return parseNumber();
        }
        // Állandók
        if (current() == 'e' && (isEnd() || !std::isalnum(peek()))) {
            advance();
            return new NumberExpression(std::exp(1.0));
        }
        if (current() == 'm' && peek() == '_' && (m_pos + 3 < m_expression.size()) && m_expression.substr(m_pos, 4) == "m_PI") {
            m_pos += 4;
            return new NumberExpression(M_PI);
        }
        // Változók: x, y, z, t
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
        // Zárójelek
        if (current() == '(') {
            advance();
            Expression* expr = parseExpression();
            if (!expr || current() != ')') { delete expr; return nullptr; }
            advance();
            return expr;
        }
        // Függvények
        if (std::isalpha(current())) {
            std::string func;
            size_t start = m_pos;
            while (!isEnd() && (std::isalnum(current()) || current() == '_')) {
                func.push_back(current());
                advance();
            }
            // Különleges eset: "V" = gyök (sqrt)
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
            // Logaritmus adott alappal: pl. log2(8)
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
                    // nem sikerült, továbblépünk
                }
            }
            // Egyéb függvények, ha '(' következik
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
        if (!isEnd() && (current() == 'e' || current() == 'E')) {
            advance();
            if (current() == '+' || current() == '-') advance();
            while (!isEnd() && std::isdigit(current())) {
                advance();
            }
        }
        try {
            double value = std::stod(m_expression.substr(start, m_pos - start));
            return new NumberExpression(value);
        }
        catch (const std::invalid_argument&) {
            return nullptr;
        }
    }
};

//////////////////////////
// Függvényrajzoló osztály
//////////////////////////

class FunctionGraph {
public:
    enum GraphType {
        EXPLICIT_2D,    // y = f(x)
        PARAMETRIC_2D,  // x = f(t), y = g(t)
        IMPLICIT_2D,    // f(x,y) = 0
        SURFACE_3D      // z = f(x,y)
    };

    FunctionGraph(Expression* expr, double xMin = -10, double xMax = 10,
        int width = 60, int height = 20, GraphType type = EXPLICIT_2D)
        : m_expression(expr), m_xMin(xMin), m_xMax(xMax), m_width(width), m_height(height),
        m_yMin(-10), m_yMax(10), m_zMin(-10), m_zMax(10), m_type(type),
        m_expressionY(nullptr), m_expressionZ(nullptr), m_autoscale(true) {
    }

    ~FunctionGraph() {
        // Az első kifejezést a hívó felelős törléséért
        if (m_expressionY) delete m_expressionY;
        if (m_expressionZ) delete m_expressionZ;
    }

    void setSecondExpression(Expression* exprY) {
        m_expressionY = exprY;
    }

    void setThirdExpression(Expression* exprZ) {
        m_expressionZ = exprZ;
    }

    void setBounds(double xMin, double xMax, double yMin, double yMax, double zMin = -10, double zMax = 10) {
        m_xMin = xMin;
        m_xMax = xMax;
        m_yMin = yMin;
        m_yMax = yMax;
        m_zMin = zMin;
        m_zMax = zMax;
        m_autoscale = false;
    }

    void setAutoscale(bool autoscale) {
        m_autoscale = autoscale;
    }

    void draw();

private:
    Expression* m_expression;     // Elsődleges kifejezés (f(x) vagy f(x,y) vagy x(t))
    Expression* m_expressionY;    // Másodlagos kifejezés (parametrikus vagy implicit esetén)
    Expression* m_expressionZ;    // Harmadlagos kifejezés (3D felület esetén)

    double m_xMin, m_xMax;
    double m_yMin, m_yMax;
    double m_zMin, m_zMax;
    int m_width, m_height;
    GraphType m_type;
    bool m_autoscale;

    double evaluateExplicit(double x);
    std::pair<double, double> evaluateParametric(double t);
    double evaluateImplicit(double x, double y);
    double evaluateSurface(double x, double y);

    std::tuple<double, double, double, double> findAutoRange();
    void drawExplicit2D();
    void drawParametric2D();
    void drawImplicit2D();
    void drawSurface3D();
};

double FunctionGraph::evaluateExplicit(double x) {
    return dynamic_cast<MultiVarExpression*>(m_expression) ?
        dynamic_cast<MultiVarExpression*>(m_expression)->evaluateWithX(x) :
        m_expression->evaluateWithX(x);
}

std::pair<double, double> FunctionGraph::evaluateParametric(double t) {
    double x = dynamic_cast<MultiVarExpression*>(m_expression) ?
        dynamic_cast<MultiVarExpression*>(m_expression)->evaluateWithX(t) :
        m_expression->evaluateWithX(t);
    double y = dynamic_cast<MultiVarExpression*>(m_expressionY) ?
        dynamic_cast<MultiVarExpression*>(m_expressionY)->evaluateWithX(t) :
        m_expressionY->evaluateWithX(t);
    return { x, y };
}

double FunctionGraph::evaluateImplicit(double x, double y) {
    return dynamic_cast<MultiVarExpression*>(m_expression) ?
        dynamic_cast<MultiVarExpression*>(m_expression)->evaluateWithXY(x, y) :
        m_expression->evaluateWithX(x);
}

double FunctionGraph::evaluateSurface(double x, double y) {
    return dynamic_cast<MultiVarExpression*>(m_expression) ?
        dynamic_cast<MultiVarExpression*>(m_expression)->evaluateWithXY(x, y) :
        m_expression->evaluateWithX(x);
}

std::tuple<double, double, double, double> FunctionGraph::findAutoRange() {
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();

    const int samples = 100;

    if (m_type == EXPLICIT_2D) {
        double step = (m_xMax - m_xMin) / samples;
        for (int i = 0; i <= samples; ++i) {
            double x = m_xMin + i * step;
            try {
                double y = evaluateExplicit(x);
                if (std::isfinite(y)) {
                    yMin = std::min(yMin, y);
                    yMax = std::max(yMax, y);
                }
            }
            catch (...) {}
        }
        return { m_xMin, m_xMax, yMin, yMax };
    }
    else if (m_type == PARAMETRIC_2D) {
        double tMin = m_xMin, tMax = m_xMax;
        double step = (tMax - tMin) / samples;
        for (int i = 0; i <= samples; ++i) {
            double t = tMin + i * step;
            try {
                auto [x, y] = evaluateParametric(t);
                if (std::isfinite(x) && std::isfinite(y)) {
                    xMin = std::min(xMin, x);
                    xMax = std::max(xMax, x);
                    yMin = std::min(yMin, y);
                    yMax = std::max(yMax, y);
                }
            }
            catch (...) {}
        }
        return { xMin, xMax, yMin, yMax };
    }
    else if (m_type == IMPLICIT_2D) {
        double xStep = (m_xMax - m_xMin) / samples;
        double yStep = (m_yMax - m_yMin) / samples;
        for (int i = 0; i <= samples; ++i) {
            for (int j = 0; j <= samples; ++j) {
                double x = m_xMin + i * xStep;
                double y = m_yMin + j * yStep;
                try {
                    double value = evaluateImplicit(x, y);
                    double nextValue = evaluateImplicit(x + xStep, y);
                    if (value * nextValue <= 0 && std::abs(value) < 1.0) {
                        xMin = std::min(xMin, x);
                        xMax = std::max(xMax, x);
                        yMin = std::min(yMin, y);
                        yMax = std::max(yMax, y);
                    }
                    nextValue = evaluateImplicit(x, y + yStep);
                    if (value * nextValue <= 0 && std::abs(value) < 1.0) {
                        xMin = std::min(xMin, x);
                        xMax = std::max(xMax, x);
                        yMin = std::min(yMin, y);
                        yMax = std::max(yMax, y);
                    }
                }
                catch (...) {}
            }
        }
        return { xMin, xMax, yMin, yMax };
    }
    else if (m_type == SURFACE_3D) {
        double xStep = (m_xMax - m_xMin) / samples;
        double yStep = (m_yMax - m_yMin) / samples;
        double zMin = std::numeric_limits<double>::max();
        double zMax = std::numeric_limits<double>::lowest();
        for (int i = 0; i <= samples; ++i) {
            for (int j = 0; j <= samples; ++j) {
                double x = m_xMin + i * xStep;
                double y = m_yMin + j * yStep;
                try {
                    double z = evaluateSurface(x, y);
                    if (std::isfinite(z)) {
                        zMin = std::min(zMin, z);
                        zMax = std::max(zMax, z);
                    }
                }
                catch (...) {}
            }
        }
        m_zMin = zMin;
        m_zMax = zMax;
        return { m_xMin, m_xMax, m_yMin, m_yMax };
    }

    return { m_xMin, m_xMax, m_yMin, m_yMax };
}

void FunctionGraph::drawExplicit2D() {
    if (m_autoscale) {
        auto [xMin, xMax, yMin, yMax] = findAutoRange();
        double xPadding = (xMax - xMin) * 0.1;
        double yPadding = (yMax - yMin) * 0.1;
        if (xMin < xMax && yMin < yMax) {
            m_xMin = xMin - xPadding;
            m_xMax = xMax + xPadding;
            m_yMin = yMin - yPadding;
            m_yMax = yMax + yPadding;
        }
    }

    std::vector<std::vector<char>> grid(m_height, std::vector<char>(m_width, ' '));

    int xAxisRow = static_cast<int>((0 - m_yMin) / (m_yMax - m_yMin) * (m_height - 1));
    if (xAxisRow >= 0 && xAxisRow < m_height)
        for (int j = 0; j < m_width; ++j)
            grid[xAxisRow][j] = '-';

    int yAxisCol = static_cast<int>((0 - m_xMin) / (m_xMax - m_xMin) * (m_width - 1));
    if (yAxisCol >= 0 && yAxisCol < m_width)
        for (int i = 0; i < m_height; ++i)
            grid[i][yAxisCol] = '|';

    const int samples = m_width * 2;
    double step = (m_xMax - m_xMin) / samples;

    for (int i = 0; i <= samples; ++i) {
        double x = m_xMin + i * step;
        try {
            double y = evaluateExplicit(x);
            if (std::isfinite(y)) {
                int col = static_cast<int>((x - m_xMin) / (m_xMax - m_xMin) * (m_width - 1));
                int row = static_cast<int>((m_yMax - y) / (m_yMax - m_yMin) * (m_height - 1));
                if (row >= 0 && row < m_height && col >= 0 && col < m_width)
                    grid[row][col] = '*';
            }
        }
        catch (...) {}
    }

    std::cout << "Graph for f(x) = [expression] in range x: [" << m_xMin << ", " << m_xMax
        << "], y: [" << m_yMin << ", " << m_yMax << "]\n";
    for (const auto& row : grid) {
        for (char cell : row)
            std::cout << cell;
        std::cout << "\n";
    }
    std::cout << std::left << std::setw(10) << m_yMin;
    std::cout << std::right << std::setw(m_width - 20) << "x";
    std::cout << std::right << std::setw(10) << m_yMax << "\n";
    std::cout << std::left << std::setw(m_width / 3) << m_xMin;
    std::cout << std::right << std::setw(m_width / 3) << "0";
    std::cout << std::right << std::setw(m_width / 3) << m_xMax << "\n";
}

void FunctionGraph::drawParametric2D() {
    if (m_autoscale) {
        auto [xMin, xMax, yMin, yMax] = findAutoRange();
        double xPadding = (xMax - xMin) * 0.1;
        double yPadding = (yMax - yMin) * 0.1;
        if (xMin < xMax && yMin < yMax) {
            m_xMin = xMin - xPadding;
            m_xMax = xMax + xPadding;
            m_yMin = yMin - yPadding;
            m_yMax = yMax + yPadding;
        }
    }

    std::vector<std::vector<char>> grid(m_height, std::vector<char>(m_width, ' '));

    int xAxisRow = static_cast<int>((0 - m_yMin) / (m_yMax - m_yMin) * (m_height - 1));
    if (xAxisRow >= 0 && xAxisRow < m_height)
        for (int j = 0; j < m_width; ++j)
            grid[xAxisRow][j] = '-';

    int yAxisCol = static_cast<int>((0 - m_xMin) / (m_xMax - m_xMin) * (m_width - 1));
    if (yAxisCol >= 0 && yAxisCol < m_width)
        for (int i = 0; i < m_height; ++i)
            grid[i][yAxisCol] = '|';

    const int samples = m_width * 2;
    double tMin = m_xMin, tMax = m_xMax;
    double step = (tMax - tMin) / samples;

    for (int i = 0; i <= samples; ++i) {
        double t = tMin + i * step;
        try {
            auto [x, y] = evaluateParametric(t);
            if (std::isfinite(x) && std::isfinite(y)) {
                int col = static_cast<int>((x - m_xMin) / (m_xMax - m_xMin) * (m_width - 1));
                int row = static_cast<int>((m_yMax - y) / (m_yMax - m_yMin) * (m_height - 1));
                if (row >= 0 && row < m_height && col >= 0 && col < m_width)
                    grid[row][col] = '*';
            }
        }
        catch (...) {}
    }

    std::cout << "Parametric curve x(t), y(t) in range t: [" << tMin << ", " << tMax
        << "], x: [" << m_xMin << ", " << m_xMax
        << "], y: [" << m_yMin << ", " << m_yMax << "]\n";
    for (const auto& row : grid) {
        for (char cell : row)
            std::cout << cell;
        std::cout << "\n";
    }
    std::cout << std::left << std::setw(10) << m_yMin;
    std::cout << std::right << std::setw(m_width - 20) << "x";
    std::cout << std::right << std::setw(10) << m_yMax << "\n";
    std::cout << std::left << std::setw(m_width / 3) << m_xMin;
    std::cout << std::right << std::setw(m_width / 3) << "0";
    std::cout << std::right << std::setw(m_width / 3) << m_xMax << "\n";
}

void FunctionGraph::drawImplicit2D() {
    if (m_autoscale) {
        auto [xMin, xMax, yMin, yMax] = findAutoRange();
        double xPadding = (xMax - xMin) * 0.1;
        double yPadding = (yMax - yMin) * 0.1;
        if (xMin < xMax && yMin < yMax) {
            m_xMin = xMin - xPadding;
            m_xMax = xMax + xPadding;
            m_yMin = yMin - yPadding;
            m_yMax = yMax + yPadding;
        }
    }

    std::vector<std::vector<char>> grid(m_height, std::vector<char>(m_width, ' '));

    int xAxisRow = static_cast<int>((0 - m_yMin) / (m_yMax - m_yMin) * (m_height - 1));
    if (xAxisRow >= 0 && xAxisRow < m_height)
        for (int j = 0; j < m_width; ++j)
            grid[xAxisRow][j] = '-';

    int yAxisCol = static_cast<int>((0 - m_xMin) / (m_xMax - m_xMin) * (m_width - 1));
    if (yAxisCol >= 0 && yAxisCol < m_width)
        for (int i = 0; i < m_height; ++i)
            grid[i][yAxisCol] = '|';

    const int xSamples = m_width;
    const int ySamples = m_height;
    double xStep = (m_xMax - m_xMin) / xSamples;
    double yStep = (m_yMax - m_yMin) / ySamples;

    std::vector<std::vector<double>> values(ySamples + 1, std::vector<double>(xSamples + 1));
    for (int j = 0; j <= ySamples; ++j) {
        for (int i = 0; i <= xSamples; ++i) {
            double x = m_xMin + i * xStep;
            double y = m_yMin + j * yStep;
            try {
                values[j][i] = evaluateImplicit(x, y);
            }
            catch (...) {
                values[j][i] = std::numeric_limits<double>::quiet_NaN();
            }
        }
    }

    for (int j = 0; j < ySamples; ++j) {
        for (int i = 0; i < xSamples; ++i) {
            if (!std::isfinite(values[j][i])) continue;
            // Jobb szomszéd vizsgálata
            if (i < xSamples && std::isfinite(values[j][i + 1])) {
                if (values[j][i] * values[j][i + 1] <= 0 && std::abs(values[j][i]) < 1.0) {
                    int col = i;
                    int row = j;
                    if (row >= 0 && row < m_height && col >= 0 && col < m_width)
                        grid[m_height - 1 - row][col] = '*';
                }
            }
            // Alsó szomszéd vizsgálata
            if (j < ySamples && std::isfinite(values[j + 1][i])) {
                if (values[j][i] * values[j + 1][i] <= 0 && std::abs(values[j][i]) < 1.0) {
                    int col = i;
                    int row = j;
                    if (row >= 0 && row < m_height && col >= 0 && col < m_width)
                        grid[m_height - 1 - row][col] = '*';
                }
            }
        }
    }

    std::cout << "Implicit graph for f(x,y)=0 in range x: [" << m_xMin << ", " << m_xMax
        << "], y: [" << m_yMin << ", " << m_yMax << "]\n";
    for (const auto& row : grid) {
        for (char cell : row)
            std::cout << cell;
        std::cout << "\n";
    }
    std::cout << std::left << std::setw(10) << m_yMin;
    std::cout << std::right << std::setw(m_width - 20) << "x";
    std::cout << std::right << std::setw(10) << m_yMax << "\n";
    std::cout << std::left << std::setw(m_width / 3) << m_xMin;
    std::cout << std::right << std::setw(m_width / 3) << "0";
    std::cout << std::right << std::setw(m_width / 3) << m_xMax << "\n";
}

void FunctionGraph::drawSurface3D() {
    if (m_autoscale) {
        auto [xMin, xMax, yMin, yMax] = findAutoRange();
        m_xMin = xMin;
        m_xMax = xMax;
        m_yMin = yMin;
        m_yMax = yMax;
    }
    int gridWidth = m_width;
    int gridHeight = m_height;
    std::vector<std::vector<double>> zValues(gridHeight, std::vector<double>(gridWidth, 0));
    double xStep = (m_xMax - m_xMin) / (gridWidth - 1);
    double yStep = (m_yMax - m_yMin) / (gridHeight - 1);
    double zMin = std::numeric_limits<double>::max();
    double zMax = std::numeric_limits<double>::lowest();
    for (int j = 0; j < gridHeight; ++j) {
        for (int i = 0; i < gridWidth; ++i) {
            double x = m_xMin + i * xStep;
            double y = m_yMin + j * yStep;
            try {
                double z = evaluateSurface(x, y);
                zValues[j][i] = z;
                if (std::isfinite(z)) {
                    zMin = std::min(zMin, z);
                    zMax = std::max(zMax, z);
                }
            }
            catch (...) {
                zValues[j][i] = std::numeric_limits<double>::quiet_NaN();
            }
        }
    }
    std::string gradient = " .:-=+*#%@";
    int gradSize = gradient.size();
    std::vector<std::vector<char>> grid(gridHeight, std::vector<char>(gridWidth, ' '));
    for (int j = 0; j < gridHeight; ++j) {
        for (int i = 0; i < gridWidth; ++i) {
            double z = zValues[j][i];
            if (!std::isfinite(z))
                grid[j][i] = ' ';
            else {
                int index = 0;
                if (zMax > zMin)
                    index = static_cast<int>(((z - zMin) / (zMax - zMin)) * (gradSize - 1));
                if (index < 0) index = 0;
                if (index >= gradSize) index = gradSize - 1;
                grid[j][i] = gradient[index];
            }
        }
    }
    std::cout << "3D Surface graph for z = f(x,y) in range x: [" << m_xMin << ", " << m_xMax
        << "], y: [" << m_yMin << ", " << m_yMax << "], z: [" << zMin << ", " << zMax << "]\n";
    for (const auto& row : grid) {
        for (char cell : row)
            std::cout << cell;
        std::cout << "\n";
    }
}

void FunctionGraph::draw() {
    switch (m_type) {
    case EXPLICIT_2D:
        drawExplicit2D();
        break;
    case PARAMETRIC_2D:
        drawParametric2D();
        break;
    case IMPLICIT_2D:
        drawImplicit2D();
        break;
    case SURFACE_3D:
        drawSurface3D();
        break;
    default:
        drawExplicit2D();
        break;
    }
}

//////////////////////
// MathModule modul //
//////////////////////

// A MathModule a Module interfész alapján készült (a "math.h" tartalmazza a Module definícióját)
class MathModule : public Module {
public:
    void execute(const std::vector<std::string>& args) override {
        if (args.empty()) {
            printHelp();
            return;
        }

        std::string command = args[0];

        // "graph" parancs: függvényrajz
        if (command == "graph") {
            if (args.size() < 2) {
                std::cout << "A 'graph' parancsnak legalább egy argumentumra van szüksége, például: graph 3*x^2+2*x+1\n";
                return;
            }
            std::string functionStr;
            for (size_t i = 1; i < args.size(); ++i)
                functionStr += args[i] + " ";
            ExpressionParser parser(functionStr);
            Expression* expr = parser.parse();
            if (!expr) {
                std::cout << "Hibás függvény kifejezés!\n";
                return;
            }
            if (!parser.hasX()) {
                std::cout << "A függvény nem tartalmaz 'x' változót!\n";
                delete expr;
                return;
            }
            // Az explicit 2D rajzot választjuk alapértelmezettként
            FunctionGraph graph(expr, -10, 10, 60, 20, FunctionGraph::EXPLICIT_2D);
            graph.draw();
            delete expr;
            return;
        }

        // "version" parancs: a modul verziószámának kiírása
        if (command == "version") {
            std::cout << "Math Module Version 1.0.0\n";
            return;
        }

        // Ha nem grafikus parancs, akkor a kifejezés értékelése
        std::string expressionStr;
        for (const auto& arg : args)
            expressionStr += arg + " ";
        ExpressionParser parser(expressionStr);
        Expression* expr = parser.parse();
        if (!expr) {
            std::cout << "Hibás kifejezés formátum!\n";
            return;
        }
        try {
            double result = expr->evaluate();
            std::cout << std::fixed << std::setprecision(8);
            if (std::abs(result - std::round(result)) < 1e-10)
                std::cout << std::setprecision(0);
            std::cout << result << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "Hiba a kifejezés kiértékelése közben: " << e.what() << std::endl;
        }
        delete expr;
    }
private:
    void printHelp() {
        std::cout << "Math modul használata:\n"
            << "  <szám1> + <szám2>    Összeadás\n"
            << "  <szám1> - <szám2>    Kivonás\n"
            << "  <szám1> * <szám2>    Szorzás\n"
            << "  <szám1> / <szám2>    Osztás\n"
            << "  <szám> ^ <hatvány>   Hatványozás\n"
            << "  V(<szám>)           Gyökvonás\n"
            << "  ln(<szám>)          Természetes alapú logaritmus\n"
            << "  log<alap>(<szám>)   Logaritmus adott alapra\n"
            << "  lg(<szám>)          10-es alapú logaritmus\n"
            << "  sin(<szám>)         Szinusz\n"
            << "  cos(<szám>)         Koszinusz\n"
            << "  tan(<szám>)         Tangens\n"
            << "  ctg(<szám>)         Kotangens\n"
            << "  arcsin(<szám>)      Arkusz szinusz\n"
            << "  arccos(<szám>)      Arkusz koszinusz\n"
            << "  arctan(<szám>)      Arkusz tangens\n"
            << "  arcctg(<szám>)      Arkusz kotangens\n"
            << "  e                   Euler-szám\n"
            << "  m_PI                Pi konstans\n"
            << "  x                   Változó (csak graph parancsban)\n"
            << "  graph <kifejezés>   Függvény rajzolása\n"
            << "  version             Modul verziószáma\n"
            << "\n"
            << "Példák:\n"
            << "  3 + 4 * 2           Eredmény: 11 (műveleti sorrend érvényesül)\n"
            << "  sin(m_PI/6)         Eredmény: 0.5\n"
            << "  graph x^2 - 4       Parabola rajzolása\n";
    }
};

///////////////////////////
// Modul létrehozása exportálással
///////////////////////////

extern "C" __declspec(dllexport) Module* createModule() {
    return new MathModule();
}
