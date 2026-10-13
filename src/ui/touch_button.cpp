#include "touch_button.h"

bool TouchButton::update(const m5::touch_detail_t& detail) {
    wasClicked_ = false;

    if (detail.isPressed()) {
        if (contains(detail.x, detail.y)) {
            isPressed_ = true;
        } else {
            isPressed_ = false;
        }
    } else if (detail.wasReleased()) {
        if (isPressed_ && contains(detail.prev_x, detail.prev_y)) {
            wasClicked_ = true;
        }
        isPressed_ = false;
    } else {
        isPressed_ = false;
    }

    return wasClicked_;
}

void TouchButton::draw(M5Canvas& canvas, int offsetY) {
    int drawY = y_ - offsetY;
    uint16_t currentBg = isPressed_ ? pressedColor_ : bgColor_;

    // ボタンの角丸四角形
    canvas.fillRoundRect(x_, drawY, w_, h_, 6, currentBg);
    canvas.drawRoundRect(x_, drawY, w_, h_, 6, isPressed_ ? Theme::ACCENT_BLUE : Theme::TEXT_MUTED);

    // テキスト中央揃え
    canvas.setTextDatum(textdatum_t::middle_center);
    canvas.setTextColor(textColor_, currentBg);
    canvas.setFont(&fonts::Font2); // M5GFX 標準フォント
    canvas.drawString(label_, x_ + w_ / 2, drawY + h_ / 2);
}
