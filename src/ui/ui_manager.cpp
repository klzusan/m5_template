#include "ui_manager.h"
#include "utils/logger.h"

static const char* TAG = "UIManager";

void UIManager::init() {
    // 1. ステータスバー Canvas (320x24)
    statusBar_.init(statusCanvas_);

    // 2. メイン領域 Canvas (320x216)
    mainCanvas_.setColorDepth(16);
    mainCanvas_.setPsram(true); // PSRAM優先で割り当て
    if (!mainCanvas_.createSprite(SCREEN_WIDTH, MAIN_CANVAS_HEIGHT)) {
        LOG_W(TAG, "Failed to create main canvas with PSRAM, falling back to SRAM...");
        mainCanvas_.setPsram(false);
        mainCanvas_.createSprite(SCREEN_WIDTH, MAIN_CANVAS_HEIGHT);
    }

    // 3. サンプルボタンの配置
    // (x, y, w, h, label, bg, pressed, text)
    btnCounter_ = TouchButton(20, 110, 130, 42, "Count +1", 
                              Theme::ACCENT_BLUE, Theme::BTN_PRESSED, Theme::TEXT_MAIN);
    btnSleep_   = TouchButton(170, 110, 130, 42, "Sleep Test", 
                              Theme::BTN_DEFAULT, Theme::BTN_PRESSED, Theme::TEXT_MAIN);

    LOG_I(TAG, "UIManager initialized.");
}

void UIManager::update() {
    auto touchDetail = M5.Touch.getDetail();

    // 画面内ボタンのタッチ判定
    if (btnCounter_.update(touchDetail)) {
        counter_++;
        lastAction_ = "Counter: " + String(counter_);
        SystemManager::instance().reportUserActivity();
        LOG_I(TAG, "Counter button clicked: %d", counter_);
    }

    if (btnSleep_.update(touchDetail)) {
        lastAction_ = "Entering Sleep...";
        SystemManager::instance().reportUserActivity();
        LOG_I(TAG, "Manual sleep requested via UI");
        SystemManager::instance().enterSleep();
    }

    // Core2 画面下部のハードウェア/静電タッチボタン (BtnA, BtnB, BtnC)
    if (M5.BtnA.wasClicked()) {
        counter_ -= 1;
        lastAction_ = "Btn A: Decrement (-1)";
        SystemManager::instance().reportUserActivity();
        LOG_I(TAG, "BtnA clicked");
    }
    if (M5.BtnB.wasClicked()) {
        counter_ = 0;
        lastAction_ = "Btn B: Reset (0)";
        SystemManager::instance().reportUserActivity();
        LOG_I(TAG, "BtnB clicked");
    }
    if (M5.BtnC.wasClicked()) {
        counter_ += 5;
        lastAction_ = "Btn C: Add (+5)";
        SystemManager::instance().reportUserActivity();
        LOG_I(TAG, "BtnC clicked");
    }

    // ステータスバーの状態更新
    statusBar_.update(
        SystemManager::instance().getBatteryStatus(),
        NetworkTaskManager::instance().getState()
    );
}

void UIManager::render() {
    // 1. ステータスバー描画
    statusBar_.draw(statusCanvas_);
    statusCanvas_.pushSprite(0, 0);

    // 2. メインコンテンツ描画
    drawMainContent();
    mainCanvas_.pushSprite(0, STATUS_BAR_HEIGHT);
}

void UIManager::drawMainContent() {
    mainCanvas_.fillSprite(Theme::BG_DARK);

    // システム情報カード (上部パネル)
    mainCanvas_.fillRoundRect(12, 10, SCREEN_WIDTH - 24, 76, 8, Theme::BG_PANEL);
    mainCanvas_.drawRoundRect(12, 10, SCREEN_WIDTH - 24, 76, 8, Theme::STATUS_BG);

    // タイトル
    mainCanvas_.setTextDatum(textdatum_t::top_left);
    mainCanvas_.setTextColor(Theme::ACCENT_BLUE, Theme::BG_PANEL);
    mainCanvas_.setFont(&fonts::Font2);
    mainCanvas_.drawString("M5Stack Core2 Template", 24, 18);

    // ネットワーク & メモリ情報
    auto netState = NetworkTaskManager::instance().getState();
    char infoBuf[64];
    snprintf(infoBuf, sizeof(infoBuf), "IP: %s | FreeRAM: %uKB", 
             netState.ipAddress.c_str(), 
             esp_get_free_heap_size() / 1024);

    mainCanvas_.setTextColor(Theme::TEXT_MUTED, Theme::BG_PANEL);
    mainCanvas_.setFont(&fonts::Font0);
    mainCanvas_.drawString(infoBuf, 24, 40);

    // 最終アクションログ
    String actionStr = "Status: " + lastAction_;
    mainCanvas_.setTextColor(Theme::ACCENT_GREEN, Theme::BG_PANEL);
    mainCanvas_.drawString(actionStr.c_str(), 24, 56);

    // ボタン描画 (メインキャンバス内なので Y オフセット STATUS_BAR_HEIGHT を考慮)
    btnCounter_.draw(mainCanvas_, STATUS_BAR_HEIGHT);
    btnSleep_.draw(mainCanvas_, STATUS_BAR_HEIGHT);

    // カウンター数値のプレビュー表示
    char countStr[32];
    snprintf(countStr, sizeof(countStr), "Current Count: %d", counter_);
    mainCanvas_.setTextDatum(textdatum_t::middle_center);
    mainCanvas_.setTextColor(Theme::TEXT_MAIN, Theme::BG_DARK);
    mainCanvas_.setFont(&fonts::Font2);
    mainCanvas_.drawString(countStr, SCREEN_WIDTH / 2, 94 - STATUS_BAR_HEIGHT + 74);

    // 画面下部 BtnA / BtnB / BtnC のガイド表示
    drawBottomButtonIndicators();
}

void UIManager::drawBottomButtonIndicators() {
    int guideY = MAIN_CANVAS_HEIGHT - 22;

    // 区切り線
    mainCanvas_.drawFastHLine(12, guideY - 4, SCREEN_WIDTH - 24, Theme::BG_PANEL);

    mainCanvas_.setFont(&fonts::Font0);
    mainCanvas_.setTextDatum(textdatum_t::middle_center);

    // BtnA (左)
    uint16_t colA = M5.BtnA.isPressed() ? Theme::ACCENT_BLUE : Theme::TEXT_MUTED;
    mainCanvas_.setTextColor(colA, Theme::BG_DARK);
    mainCanvas_.drawString("[A: -1]", 54, guideY + 8);

    // BtnB (中)
    uint16_t colB = M5.BtnB.isPressed() ? Theme::ACCENT_BLUE : Theme::TEXT_MUTED;
    mainCanvas_.setTextColor(colB, Theme::BG_DARK);
    mainCanvas_.drawString("[B: Reset]", SCREEN_WIDTH / 2, guideY + 8);

    // BtnC (右)
    uint16_t colC = M5.BtnC.isPressed() ? Theme::ACCENT_BLUE : Theme::TEXT_MUTED;
    mainCanvas_.setTextColor(colC, Theme::BG_DARK);
    mainCanvas_.drawString("[C: +5]", SCREEN_WIDTH - 54, guideY + 8);
}
