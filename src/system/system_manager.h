#pragma once
#include <Arduino.h>
#include <M5Unified.h>
#include <Preferences.h>
#include "config.h"

struct BatteryStatus {
    int level = 100;         // 0 - 100 %
    int voltage = 4000;      // mV
    bool isCharging = false;
};

class SystemManager {
public:
    static SystemManager& instance() {
        static SystemManager inst;
        return inst;
    }

    void init();
    void update(); // loop内で定期呼び出し

    // バッテリー情報取得
    BatteryStatus getBatteryStatus();

    // ユーザー操作検知 (スリープタイマーリセット)
    void reportUserActivity();

    // 輝度設定
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const { return currentBrightness_; }

    // スリープタイムアウト設定 (秒。0で無効)
    void setSleepTimeoutSec(uint32_t sec);
    uint32_t getSleepTimeoutSec() const { return sleepTimeoutSec_; }

    // 明示的なスリープ移行
    void enterSleep();
    void powerOff();

private:
    SystemManager() = default;

    Preferences prefs_;
    uint8_t currentBrightness_ = DEFAULT_BRIGHTNESS;
    uint32_t sleepTimeoutSec_ = DEFAULT_SLEEP_TIMEOUT_SEC;
    uint32_t lastActivityTime_ = 0;
    uint32_t lastBatteryCheckTime_ = 0;
    BatteryStatus cachedBatteryStatus_;

    void loadPreferences();
    void checkInactivity();
};
