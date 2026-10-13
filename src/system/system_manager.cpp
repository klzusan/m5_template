#include "system_manager.h"
#include "utils/logger.h"

static const char* TAG = "SysMgr";

void SystemManager::init() {
    loadPreferences();
    M5.Display.setBrightness(currentBrightness_);
    lastActivityTime_ = millis();

    // 初回バッテリー取得
    cachedBatteryStatus_.level = M5.Power.getBatteryLevel();
    cachedBatteryStatus_.isCharging = M5.Power.isCharging();
    LOG_I(TAG, "SystemManager initialized. Brightness: %d, SleepTimeout: %lus", 
          currentBrightness_, sleepTimeoutSec_);
}

void SystemManager::loadPreferences() {
    if (prefs_.begin(NvsKeys::NAMESPACE, true)) {
        currentBrightness_ = prefs_.getUChar(NvsKeys::KEY_BRIGHTNESS, DEFAULT_BRIGHTNESS);
        sleepTimeoutSec_   = prefs_.getUInt(NvsKeys::KEY_SLEEP_SEC, DEFAULT_SLEEP_TIMEOUT_SEC);
        prefs_.end();
    }
}

void SystemManager::setBrightness(uint8_t brightness) {
    currentBrightness_ = brightness;
    M5.Display.setBrightness(currentBrightness_);

    if (prefs_.begin(NvsKeys::NAMESPACE, false)) {
        prefs_.putUChar(NvsKeys::KEY_BRIGHTNESS, currentBrightness_);
        prefs_.end();
    }
}

void SystemManager::setSleepTimeoutSec(uint32_t sec) {
    sleepTimeoutSec_ = sec;
    reportUserActivity();

    if (prefs_.begin(NvsKeys::NAMESPACE, false)) {
        prefs_.putUInt(NvsKeys::KEY_SLEEP_SEC, sleepTimeoutSec_);
        prefs_.end();
    }
    LOG_I(TAG, "Sleep timeout set to %lu sec", sleepTimeoutSec_);
}

void SystemManager::reportUserActivity() {
    lastActivityTime_ = millis();
}

void SystemManager::update() {
    // 5秒おきにバッテリーステータス更新
    uint32_t now = millis();
    if (now - lastBatteryCheckTime_ >= 5000) {
        lastBatteryCheckTime_ = now;
        cachedBatteryStatus_.level = M5.Power.getBatteryLevel();
        cachedBatteryStatus_.isCharging = M5.Power.isCharging();
    }

    checkInactivity();
}

BatteryStatus SystemManager::getBatteryStatus() {
    return cachedBatteryStatus_;
}

void SystemManager::checkInactivity() {
    if (sleepTimeoutSec_ == 0) {
        return; // 自動スリープ無効
    }

    uint32_t elapsedSec = (millis() - lastActivityTime_) / 1000;
    if (elapsedSec >= sleepTimeoutSec_) {
        LOG_I(TAG, "Inactivity timeout reached (%lus). Entering sleep.", elapsedSec);
        enterSleep();
    }
}

void SystemManager::enterSleep() {
    LOG_I(TAG, "Preparing for sleep...");
    // 画面消灯
    M5.Display.setBrightness(0);
    M5.Display.sleep();

    // 画面タッチまたは電源ボタンで復帰するように設定
    M5.Power.setChargeCurrent(0); // 充電電流設定があれば適宜
    
    // Light Sleep または Deep Sleep 待機 (M5.Power helper)
    // 画面タッチ復帰が簡単な Light Sleep を利用
    M5.Power.lightSleep();

    // 復帰処理
    M5.Display.wakeup();
    M5.Display.setBrightness(currentBrightness_);
    reportUserActivity();
    LOG_I(TAG, "Woke up from sleep.");
}

void SystemManager::powerOff() {
    LOG_I(TAG, "Powering off...");
    M5.Display.setBrightness(0);
    M5.Display.sleep();
    M5.Power.powerOff();
}
