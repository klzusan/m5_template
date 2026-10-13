#pragma once
#include <Arduino.h>
#include <M5Unified.h>

/**
 * @brief src/app/ 配下の各機能（アプリ）が継承する基底インターフェース
 */
class AppBase {
public:
    virtual ~AppBase() = default;

    // アプリ基本情報
    virtual const char* getName() const = 0;
    virtual const char* getDescription() const { return ""; }
    virtual uint16_t getThemeColor() const { return 0x247F; } // デフォルトアクセントカラー

    // ライフサイクルコールバック
    virtual void onEnter() {}
    virtual void onExit() {}

    // フレーム更新・描画 (offsetY: StatusBarの高さ 24px)
    virtual void update() = 0;
    virtual void render(M5Canvas& canvas, int offsetY = 24) = 0;
};
