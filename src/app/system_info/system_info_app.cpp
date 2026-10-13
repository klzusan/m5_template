#include "system_info_app.h"
#include "system/system_manager.h"
#include "network/network_task.h"
#include "utils/logger.h"

static const char* TAG = "SystemInfoApp";

SystemInfoApp::SystemInfoApp() {
    btnCounter_ = TouchButton(20, 110, 130, 42, "Count +1", 
                              Theme::ACCENT_BLUE, Theme::BTN_PRESSED, Theme::TEXT_MAIN);
    btnSleep_   = TouchButton(170, 110, 130, 42, "Sleep Test", 
                              Theme::BTN_DEFAULT, Theme::BTN_PRESSED, Theme::TEXT_MAIN);
}

void SystemInfoApp::onEnter() {
    LOG_I(TAG, "Entered SystemInfoApp");
    lastAction_ = "Opened";
}

void SystemInfoApp::onExit() {
    LOG_I(TAG, "Exited SystemInfoApp");
}

void SystemInfoApp::playFeedback() {
    M5.Speaker.tone(1800, 15);
    SystemManager::instance().reportUserActivity();
}

void SystemInfoApp::update() {
    auto touchDetail = M5.Touch.getDetail();

    if (btnCounter_.update(touchDetail)) {
        counter_++;
        lastAction_ = "Counter: " + String(counter_);
        playFeedback();
        LOG_I(TAG, "Counter: %d", counter_);
    }

    if (btnSleep_.update(touchDetail)) {
        lastAction_ = "Sleep requested";
        playFeedback();
        LOG_I(TAG, "Entering sleep");
        SystemManager::instance().enterSleep();
    }

    if (M5.BtnA.wasClicked()) {
        counter_ -= 1;
        lastAction_ = "Btn A: -1";
        playFeedback();
    }
    if (M5.BtnC.wasClicked()) {
        counter_ += 5;
        lastAction_ = "Btn C: +5";
        playFeedback();
    }
}

void SystemInfoApp::render(M5Canvas& canvas, int offsetY) {
    canvas.fillSprite(Theme::BG_DARK);

    // システム情報カード
    canvas.fillRoundRect(12, 10, SCREEN_WIDTH - 24, 76, 8, Theme::BG_PANEL);
    canvas.drawRoundRect(12, 10, SCREEN_WIDTH - 24, 76, 8, Theme::STATUS_BG);

    canvas.setTextDatum(textdatum_t::top_left);
    canvas.setTextColor(Theme::ACCENT_BLUE, Theme::BG_PANEL);
    canvas.setFont(&fonts::Font2);
    canvas.drawString("Device Status Monitor", 24, 18);

    auto netState = NetworkTaskManager::instance().getState();
    char infoBuf[64];
    snprintf(infoBuf, sizeof(infoBuf), "IP: %s | Free: %uKB", 
             netState.ipAddress.c_str(), 
             esp_get_free_heap_size() / 1024);

    canvas.setTextColor(Theme::TEXT_MUTED, Theme::BG_PANEL);
    canvas.setFont(&fonts::Font0);
    canvas.drawString(infoBuf, 24, 40);

    String actionStr = "Status: " + lastAction_;
    canvas.setTextColor(Theme::ACCENT_GREEN, Theme::BG_PANEL);
    canvas.drawString(actionStr.c_str(), 24, 56);

    // ボタン描画
    btnCounter_.draw(canvas, offsetY);
    btnSleep_.draw(canvas, offsetY);

    // カウンター数値
    char countStr[32];
    snprintf(countStr, sizeof(countStr), "Current Count: %d", counter_);
    canvas.setTextDatum(textdatum_t::middle_center);
    canvas.setTextColor(Theme::TEXT_MAIN, Theme::BG_DARK);
    canvas.setFont(&fonts::Font2);
    canvas.drawString(countStr, SCREEN_WIDTH / 2, 160);

    // 下部ガイド
    canvas.setFont(&fonts::Font0);
    canvas.setTextColor(Theme::TEXT_MUTED, Theme::BG_DARK);
    canvas.drawString("[A: -1  C: +5  Hold B: Home]", SCREEN_WIDTH / 2, MAIN_CANVAS_HEIGHT - 12);
}
