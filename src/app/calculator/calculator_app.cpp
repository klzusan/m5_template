#include "calculator_app.h"
#include "system/system_manager.h"
#include "utils/logger.h"

static const char* TAG = "CalculatorApp";

struct KeyConfig {
    const char* label;
    const char* key;
    uint16_t bg;
    uint16_t pressed;
};

static const KeyConfig KEY_CONFIGS[20] = {
    // Row 0
    {"C",   "C",   Theme::KEY_FUNC_BG, Theme::BTN_PRESSED},
    {"+/-", "+/-", Theme::KEY_FUNC_BG, Theme::BTN_PRESSED},
    {"%",   "%",   Theme::KEY_FUNC_BG, Theme::BTN_PRESSED},
    {"/",   "/",   Theme::KEY_OP_BG,   Theme::KEY_OP_PRESS},
    // Row 1
    {"7",   "7",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"8",   "8",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"9",   "9",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"x",   "*",   Theme::KEY_OP_BG,   Theme::KEY_OP_PRESS},
    // Row 2
    {"4",   "4",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"5",   "5",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"6",   "6",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"-",   "-",   Theme::KEY_OP_BG,   Theme::KEY_OP_PRESS},
    // Row 3
    {"1",   "1",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"2",   "2",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"3",   "3",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"+",   "+",   Theme::KEY_OP_BG,   Theme::KEY_OP_PRESS},
    // Row 4
    {"0",   "0",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"00",  "00",  Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {".",   ".",   Theme::KEY_NUM_BG,  Theme::BTN_PRESSED},
    {"=",   "=",   Theme::ACCENT_GREEN,Theme::BTN_PRESSED}
};

CalculatorApp::CalculatorApp() {
    initKeypad();
}

void CalculatorApp::initKeypad() {
    const int startX = 7;
    const int startY = 72; // 画面全体でのY座標
    const int keyW = 72;
    const int keyH = 29;
    const int gapX = 6;
    const int gapY = 4;

    for (int i = 0; i < NUM_KEYS; ++i) {
        int col = i % 4;
        int row = i / 4;
        int x = startX + col * (keyW + gapX);
        int y = startY + row * (keyH + gapY);

        keyValues_[i] = KEY_CONFIGS[i].key;
        keys_[i] = TouchButton(x, y, keyW, keyH, 
                               KEY_CONFIGS[i].label, 
                               KEY_CONFIGS[i].bg, 
                               KEY_CONFIGS[i].pressed, 
                               Theme::TEXT_MAIN);
    }
}

void CalculatorApp::onEnter() {
    LOG_I(TAG, "Entered CalculatorApp");
    calc_.reset();
}

void CalculatorApp::onExit() {
    LOG_I(TAG, "Exited CalculatorApp");
}

void CalculatorApp::playFeedback() {
    M5.Speaker.tone(1800, 15);
    SystemManager::instance().reportUserActivity();
}

void CalculatorApp::update() {
    auto touchDetail = M5.Touch.getDetail();

    // 画面内キーパッドのタッチ判定
    for (int i = 0; i < NUM_KEYS; ++i) {
        if (keys_[i].update(touchDetail)) {
            playFeedback();
            calc_.processKey(keyValues_[i]);
            LOG_D(TAG, "Key pressed: %s", keyValues_[i]);
            break;
        }
    }

    // 画面下部のハードウェア/静電タッチボタン
    // BtnA: AC
    if (M5.BtnA.wasClicked()) {
        playFeedback();
        calc_.processKey("AC");
        LOG_I(TAG, "BtnA clicked -> AC");
    }
    // BtnC: Backspace (1文字削除)
    if (M5.BtnC.wasClicked()) {
        playFeedback();
        calc_.processKey("B");
        LOG_I(TAG, "BtnC clicked -> Backspace");
    }
    // 注: BtnB は UIManager 側で 10秒長押し（ホーム画面復帰）用に使用
}

void CalculatorApp::render(M5Canvas& canvas, int offsetY) {
    canvas.fillSprite(Theme::BG_DARK);

    // ディスプレイパネル描画
    drawDisplayPanel(canvas, offsetY);

    // キーパッドボタン描画
    for (int i = 0; i < NUM_KEYS; ++i) {
        keys_[i].draw(canvas, offsetY);
    }
}

void CalculatorApp::drawDisplayPanel(M5Canvas& canvas, int offsetY) {
    const int panelX = 7;
    const int panelY = 4;
    const int panelW = SCREEN_WIDTH - 14;
    const int panelH = 40;

    // ディスプレイ背景
    canvas.fillRoundRect(panelX, panelY, panelW, panelH, 6, Theme::STATUS_BG);
    canvas.drawRoundRect(panelX, panelY, panelW, panelH, 6, Theme::BG_PANEL);

    // 上段: 計算履歴 / 保留中の式 (右寄せ、小フォント)
    const String& expr = calc_.getExpression();
    if (expr.length() > 0) {
        canvas.setFont(&fonts::Font0);
        canvas.setTextColor(Theme::TEXT_MUTED, Theme::STATUS_BG);
        canvas.setTextDatum(textdatum_t::top_right);
        canvas.drawString(expr.c_str(), panelX + panelW - 8, panelY + 4);
    }

    // 下段: 現在の入力値 / 計算結果 (右寄せ、大フォント)
    const String& currentVal = calc_.getCurrentInput();
    canvas.setTextDatum(textdatum_t::bottom_right);

    if (calc_.hasError()) {
        canvas.setTextColor(Theme::ACCENT_ALERT, Theme::STATUS_BG);
        canvas.setFont(&fonts::Font2);
        canvas.drawString(currentVal.c_str(), panelX + panelW - 8, panelY + panelH - 4);
    } else {
        canvas.setTextColor(Theme::TEXT_MAIN, Theme::STATUS_BG);
        if (currentVal.length() > 9) {
            canvas.setFont(&fonts::Font2);
        } else {
            canvas.setFont(&fonts::Font4);
        }
        canvas.drawString(currentVal.c_str(), panelX + panelW - 8, panelY + panelH - 3);
    }

    // 左下に操作ヘルプ表示
    canvas.setFont(&fonts::Font0);
    canvas.setTextColor(Theme::TEXT_MUTED, Theme::STATUS_BG);
    canvas.setTextDatum(textdatum_t::bottom_left);
    canvas.drawString("[A:AC  C:Del  Hold B:Home]", panelX + 6, panelY + panelH - 4);
}
