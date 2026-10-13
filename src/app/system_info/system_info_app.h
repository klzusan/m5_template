#pragma once
#include "app/app_base.h"
#include "ui/touch_button.h"
#include "config.h"

class SystemInfoApp : public AppBase {
public:
    SystemInfoApp();

    const char* getName() const override { return "System Info"; }
    const char* getDescription() const override { return "Device Status & Tests"; }
    uint16_t getThemeColor() const override { return Theme::ACCENT_BLUE; }

    void onEnter() override;
    void onExit() override;
    void update() override;
    void render(M5Canvas& canvas, int offsetY = 24) override;

private:
    TouchButton btnCounter_;
    TouchButton btnSleep_;
    int counter_ = 0;
    String lastAction_ = "Ready";

    void playFeedback();
};
