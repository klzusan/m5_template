#include "status_bar.h"

void StatusBar::init(M5Canvas& canvas) {
    canvas.setColorDepth(16);
    canvas.createSprite(SCREEN_WIDTH, STATUS_BAR_HEIGHT);
}

void StatusBar::update(const BatteryStatus& battery, const NetworkState& network) {
    battery_ = battery;
    network_ = network;

    auto dt = M5.Rtc.getDateTime();
    if (dt.time.seconds != lastSec_) {
        lastSec_ = dt.time.seconds;
        snprintf(timeStr_, sizeof(timeStr_), "%02d:%02d:%02d",
                 dt.time.hours, dt.time.minutes, dt.time.seconds);
    }
}

void StatusBar::draw(M5Canvas& canvas) {
    canvas.fillSprite(Theme::STATUS_BG);

    // 下部ボーダーライン
    canvas.drawFastHLine(0, STATUS_BAR_HEIGHT - 1, SCREEN_WIDTH, Theme::BG_PANEL);

    // 1. 左側: Wi-Fi ステータス
    drawWifiIcon(canvas, 8, 4, network_.status, network_.rssi);

    // 2. 中央: デジタル時計
    canvas.setTextDatum(textdatum_t::middle_center);
    canvas.setTextColor(Theme::TEXT_MAIN, Theme::STATUS_BG);
    canvas.setFont(&fonts::Font2);
    canvas.drawString(timeStr_, SCREEN_WIDTH / 2, STATUS_BAR_HEIGHT / 2);

    // 3. 右側: バッテリーステータス
    drawBatteryIcon(canvas, SCREEN_WIDTH - 64, 4, battery_.level, battery_.isCharging);
}

void StatusBar::drawWifiIcon(M5Canvas& canvas, int x, int y, NetworkStatus status, int8_t rssi) {
    uint16_t iconColor = Theme::TEXT_MUTED;
    const char* statusText = "Offline";

    switch (status) {
        case NetworkStatus::CONNECTED:
            iconColor = Theme::ACCENT_GREEN;
            statusText = "Wi-Fi";
            break;
        case NetworkStatus::CONNECTING:
            iconColor = Theme::ACCENT_WARN;
            statusText = "Sync...";
            break;
        case NetworkStatus::NOT_CONFIGURED:
        case NetworkStatus::DISCONNECTED:
        default:
            iconColor = Theme::TEXT_MUTED;
            statusText = "Offline";
            break;
    }

    // アイコン丸印
    canvas.fillCircle(x + 4, y + 7, 3, iconColor);

    // ラベル
    canvas.setTextDatum(textdatum_t::middle_left);
    canvas.setTextColor(iconColor, Theme::STATUS_BG);
    canvas.setFont(&fonts::Font0); // 小さいフォント
    canvas.drawString(statusText, x + 12, y + 8);
}

void StatusBar::drawBatteryIcon(M5Canvas& canvas, int x, int y, int level, bool isCharging) {
    // バッテリー枠 (幅20, 高さ11)
    int bx = x + 36;
    int by = y + 2;
    canvas.drawRoundRect(bx, by, 22, 12, 2, Theme::TEXT_MUTED);
    canvas.fillRect(bx + 22, by + 3, 2, 6, Theme::TEXT_MUTED); // 端子突起

    // 残量バー
    int fillW = map(constrain(level, 0, 100), 0, 100, 0, 18);
    uint16_t barColor = (level > 20) ? Theme::ACCENT_GREEN : Theme::ACCENT_ALERT;
    if (isCharging) {
        barColor = Theme::ACCENT_WARN;
    }
    if (fillW > 0) {
        canvas.fillRect(bx + 2, by + 2, fillW, 8, barColor);
    }

    // 残量パーセント数値
    canvas.setTextDatum(textdatum_t::middle_right);
    canvas.setTextColor(Theme::TEXT_MUTED, Theme::STATUS_BG);
    canvas.setFont(&fonts::Font0);

    char batStr[16];
    if (isCharging) {
        snprintf(batStr, sizeof(batStr), "CHG %d%%", level);
    } else {
        snprintf(batStr, sizeof(batStr), "%d%%", level);
    }
    canvas.drawString(batStr, bx - 4, by + 6);
}
