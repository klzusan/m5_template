#pragma once
#include <Arduino.h>
#include <M5Unified.h>
#include "config.h"
#include "status_bar.h"
#include "touch_button.h"
#include "system/system_manager.h"
#include "network/network_task.h"

class UIManager {
public:
    static UIManager& instance() {
        static UIManager inst;
        return inst;
    }

    void init();
    void update();
    void render();

private:
    UIManager() = default;

    M5Canvas statusCanvas_{&M5.Display};
    M5Canvas mainCanvas_{&M5.Display};

    StatusBar statusBar_;

    // サンプル用UIコンポーネント
    TouchButton btnCounter_;
    TouchButton btnSleep_;
    int counter_ = 0;
    String lastAction_ = "Ready";

    void drawMainContent();
    void drawBottomButtonIndicators();
};
