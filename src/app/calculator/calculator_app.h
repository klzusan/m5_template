#pragma once
#include "app/app_base.h"
#include "app/calculator/calculator_engine.h"
#include "ui/touch_button.h"
#include "config.h"

class CalculatorApp : public AppBase {
public:
    CalculatorApp();

    const char* getName() const override { return "Calculator"; }
    const char* getDescription() const override { return "Standard 4-Operation Calculator"; }
    uint16_t getThemeColor() const override { return Theme::KEY_OP_BG; }

    void onEnter() override;
    void onExit() override;
    void update() override;
    void render(M5Canvas& canvas, int offsetY = 24) override;

private:
    CalculatorEngine calc_;
    static constexpr int NUM_KEYS = 20;
    TouchButton keys_[NUM_KEYS];
    const char* keyValues_[NUM_KEYS] = {};

    void initKeypad();
    void drawDisplayPanel(M5Canvas& canvas, int offsetY);
    void playFeedback();
};
