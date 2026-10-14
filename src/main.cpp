#include "config.h"
#include "network/network_task.h"
#include "system/system_manager.h"
#include "ui/ui_manager.h"
#include "utils/logger.h"
#include <Arduino.h>
#include <M5Unified.h>

static const char *TAG = "Main";
static uint32_t lastFrameTime = 0;

void setup() {
  // 1. M5Unified 初期化 (Core2の画面, タッチ, 電源ICを自動検出)
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);

  // 2. ロガー初期化 & タイムゾーン設定
  Logger::init(115200);
  setenv("TZ", "JST-9", 1);
  tzset();
  LOG_I(TAG, "========================================");
  LOG_I(TAG, "  M5Stack Core2 Template Initializing   ");
  LOG_I(TAG, "========================================");

  // 3. システムマネージャー初期化 (輝度, 電源, スリープ)
  SystemManager::instance().init();

  // 4. バックグラウンド通信タスク起動 (Core 0)
  NetworkTaskManager::instance().start();

  // 5. UIマネージャー初期化 (Canvas生成 & 画面配置)
  UIManager::instance().init();

  LOG_I(TAG, "Initialization completed. Entering main loop.");
}

void loop() {
  // 1. M5Unified のハードウェア状態更新 (タッチ・ボタン判定に必須)
  M5.update();

  // 2. システム・UI状態更新
  SystemManager::instance().update();
  UIManager::instance().update();
  NetworkTaskManager::instance().checkAndApplyRtcSync();

  // 3. 一定フレームレート (約30 FPS) で描画
  uint32_t now = millis();
  if (now - lastFrameTime >= FRAME_DELAY_MS) {
    lastFrameTime = now;
    UIManager::instance().render();
  }

  // CPU負荷軽減のための短いスリープ
  delay(2);
}