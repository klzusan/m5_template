#pragma once
#include "app/app_base.h"
#include "app/calculator/calculator_app.h"
#include "app/system_info/system_info_app.h"
#include "config.h"
#include "status_bar.h"
#include "touch_button.h"
#include <Arduino.h>
#include <M5Unified.h>
#include <vector>

class UIManager {
public:
  static UIManager &instance() {
    static UIManager inst;
    return inst;
  }

  void init();
  void update();
  void render();

  // アプリ切り替え
  void launchApp(AppBase *app);
  void returnToHome();
  bool isHome() const { return currentApp_ == nullptr; }

private:
  UIManager() = default;

  M5Canvas statusCanvas_{&M5.Display};
  M5Canvas mainCanvas_{&M5.Display};

  StatusBar statusBar_;

  // アプリインスタンス
  CalculatorApp calcApp_;
  SystemInfoApp sysInfoApp_;
  std::vector<AppBase *> apps_;
  int selectedAppIndex_ = 0;
  AppBase *currentApp_ = nullptr;

  // BtnB 3秒長押し監視用
  uint32_t btnBPressStartTime_ = 0;
  static constexpr uint32_t HOLD_HOME_THRESHOLD_MS = 3000; // 3秒

  void drawHomeContent();
  void checkBtnBHomeTrigger();
  void drawHoldProgressBar(uint32_t elapsedMs);
};
