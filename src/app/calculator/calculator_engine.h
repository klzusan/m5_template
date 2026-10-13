#pragma once
#include <Arduino.h>

class CalculatorEngine {
public:
    CalculatorEngine();

    // キー入力処理
    // key: '0'-'9', '.', '+', '-', '*', '/', '=', 'C', 'A'(All Clear), 'P'(+/-), '%', 'B'(Backspace)
    void processKey(const String& key);

    // 表示用文字列
    const String& getCurrentInput() const { return currentInput_; }
    const String& getExpression() const { return expression_; }
    bool hasError() const { return hasError_; }

    void reset();

private:
    String currentInput_;
    String expression_;
    double storedValue_ = 0.0;
    char pendingOp_ = 0;
    bool isNewNumber_ = true;
    bool hasError_ = false;

    void calculate();
    String formatNumber(double val) const;
};
