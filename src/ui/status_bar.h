#pragma once
#include "config.h"
#include "network/network_task.h"
#include "system/system_manager.h"
#include <Arduino.h>
#include <M5Unified.h>

class StatusBar {
public:
  StatusBar() = default;

  void init(M5Canvas &canvas);
  void update(const BatteryStatus &battery, const NetworkState &network);
  void draw(M5Canvas &canvas);

private:
  BatteryStatus battery_;
  NetworkState network_;
  char timeStr_[64] = "----/--/--(---), --:--:--";
  uint32_t lastSec_ = 99;

  void drawBatteryIcon(M5Canvas &canvas, int x, int y, int level,
                       bool isCharging);
  void drawWifiIcon(M5Canvas &canvas, int x, int y, NetworkStatus status,
                    int8_t rssi);
};
