#pragma once
#include <Arduino.h>
#include <M5Unified.h>
#include "config.h"

class TouchButton {
public:
    TouchButton() = default;
    TouchButton(int x, int y, int w, int h, const char* label, 
                uint16_t bgColor = Theme::BTN_DEFAULT, 
                uint16_t pressedColor = Theme::BTN_PRESSED,
                uint16_t textColor = Theme::TEXT_MAIN)
        : x_(x), y_(y), w_(w), h_(h), label_(label),
          bgColor_(bgColor), pressedColor_(pressedColor), textColor_(textColor) {}

    void setBounds(int x, int y, int w, int h) {
        x_ = x; y_ = y; w_ = w; h_ = h;
    }
    void setLabel(const char* label) { label_ = label; }
    void setColors(uint16_t bg, uint16_t pressed, uint16_t text) {
        bgColor_ = bg; pressedColor_ = pressed; textColor_ = text;
    }

    // タッチ更新 (M5.Touch.getDetail() を渡す)
    // 戻り値: クリック(タップして離された)されたら true
    bool update(const m5::touch_detail_t& detail);

    // M5Canvas に描画 (offsetY: 分割キャンバス用のY座標オフセット)
    void draw(M5Canvas& canvas, int offsetY = 0);

    bool isPressed() const { return isPressed_; }

private:
    int x_ = 0;
    int y_ = 0;
    int w_ = 0;
    int h_ = 0;
    const char* label_ = "";
    uint16_t bgColor_ = Theme::BTN_DEFAULT;
    uint16_t pressedColor_ = Theme::BTN_PRESSED;
    uint16_t textColor_ = Theme::TEXT_MAIN;

    bool isPressed_ = false;
    bool wasClicked_ = false;

    bool contains(int px, int py) const {
        return (px >= x_ && px < x_ + w_ && py >= y_ && py < y_ + h_);
    }
};
