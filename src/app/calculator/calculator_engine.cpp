#include "calculator_engine.h"
#include <cmath>

CalculatorEngine::CalculatorEngine() {
    reset();
}

void CalculatorEngine::reset() {
    currentInput_ = "0";
    expression_ = "";
    storedValue_ = 0.0;
    pendingOp_ = 0;
    isNewNumber_ = true;
    hasError_ = false;
}

String CalculatorEngine::formatNumber(double val) const {
    if (std::isnan(val) || std::isinf(val)) {
        return "Error";
    }

    // 整数かどうか確認
    if (std::floor(val) == val && std::abs(val) < 1e11) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f", val);
        return String(buf);
    }

    // 小数点付きのフォーマット
    char buf[32];
    snprintf(buf, sizeof(buf), "%.6f", val);
    String s(buf);

    // 末尾の冗長な 0 と必要に応じた小数点を除去
    while (s.endsWith("0")) {
        s.remove(s.length() - 1);
    }
    if (s.endsWith(".")) {
        s.remove(s.length() - 1);
    }

    return s;
}

void CalculatorEngine::calculate() {
    if (pendingOp_ == 0 || hasError_) return;

    double currentVal = currentInput_.toDouble();
    double result = 0.0;

    switch (pendingOp_) {
        case '+':
            result = storedValue_ + currentVal;
            break;
        case '-':
            result = storedValue_ - currentVal;
            break;
        case '*':
            result = storedValue_ * currentVal;
            break;
        case '/':
            if (std::abs(currentVal) < 1e-12) {
                hasError_ = true;
                currentInput_ = "Error: Div/0";
                pendingOp_ = 0;
                return;
            }
            result = storedValue_ / currentVal;
            break;
        default:
            return;
    }

    storedValue_ = result;
    currentInput_ = formatNumber(result);
    if (currentInput_ == "Error") {
        hasError_ = true;
    }
}

void CalculatorEngine::processKey(const String& key) {
    if (key == "C") {
        // 現在の入力をクリア
        currentInput_ = "0";
        isNewNumber_ = true;
        hasError_ = false;
        return;
    }

    if (key == "AC") {
        reset();
        return;
    }

    if (key == "B") { // Backspace
        if (hasError_) {
            reset();
            return;
        }
        if (!isNewNumber_ && currentInput_.length() > 0) {
            currentInput_.remove(currentInput_.length() - 1);
            if (currentInput_.length() == 0 || currentInput_ == "-") {
                currentInput_ = "0";
                isNewNumber_ = true;
            }
        }
        return;
    }

    // エラー状態なら、数字または演算子が押されたらリセットして開始
    if (hasError_) {
        reset();
    }

    if (key == "+/-") {
        if (currentInput_ != "0") {
            if (currentInput_.startsWith("-")) {
                currentInput_.remove(0, 1);
            } else {
                currentInput_ = "-" + currentInput_;
            }
        }
        return;
    }

    if (key == "%") {
        double val = currentInput_.toDouble() / 100.0;
        currentInput_ = formatNumber(val);
        isNewNumber_ = true;
        return;
    }

    // 数字キー (0〜9)
    if (key.length() == 1 && key[0] >= '0' && key[0] <= '9') {
        if (isNewNumber_) {
            currentInput_ = key;
            isNewNumber_ = false;
        } else {
            // 最大桁数制限 (10桁)
            if (currentInput_.length() < 10) {
                if (currentInput_ == "0") {
                    currentInput_ = key;
                } else {
                    currentInput_ += key;
                }
            }
        }
        return;
    }

    if (key == "00") {
        if (isNewNumber_) {
            currentInput_ = "0";
        } else if (currentInput_ != "0" && currentInput_.length() < 9) {
            currentInput_ += "00";
        }
        return;
    }

    if (key == ".") {
        if (isNewNumber_) {
            currentInput_ = "0.";
            isNewNumber_ = false;
        } else if (currentInput_.indexOf('.') == -1) {
            currentInput_ += ".";
        }
        return;
    }

    // 演算子キー (+, -, *, /)
    if (key == "+" || key == "-" || key == "*" || key == "/") {
        char op = key[0];
        String opSymbol = (op == '*') ? "x" : (op == '/') ? "/" : String(op);

        if (pendingOp_ != 0 && !isNewNumber_) {
            calculate();
        } else {
            storedValue_ = currentInput_.toDouble();
        }

        pendingOp_ = op;
        expression_ = formatNumber(storedValue_) + " " + opSymbol;
        isNewNumber_ = true;
        return;
    }

    // イコール (=)
    if (key == "=") {
        if (pendingOp_ != 0) {
            char op = pendingOp_;
            String opSymbol = (op == '*') ? "x" : (op == '/') ? "/" : String(op);
            String rightOperandStr = currentInput_;
            double leftOperand = storedValue_;

            calculate();

            if (!hasError_) {
                expression_ = formatNumber(leftOperand) + " " + opSymbol + " " + rightOperandStr + " =";
            }
            pendingOp_ = 0;
            isNewNumber_ = true;
        }
        return;
    }
}
