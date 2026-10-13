#pragma once
#include <Arduino.h>

// ==========================================
// ディスプレイ & UI 設定
// ==========================================
constexpr int SCREEN_WIDTH = 320;
constexpr int SCREEN_HEIGHT = 240;
constexpr int STATUS_BAR_HEIGHT = 24;
constexpr int MAIN_CANVAS_HEIGHT = SCREEN_HEIGHT - STATUS_BAR_HEIGHT; // 216

// フレームレート制御
constexpr uint32_t TARGET_FPS = 30;
constexpr uint32_t FRAME_DELAY_MS = 1000 / TARGET_FPS;

// カラーパレット (RGB565 / M5GFX)
namespace Theme {
    constexpr uint16_t BG_DARK      = 0x10A2; // 深いダークグレー (#101418)
    constexpr uint16_t BG_PANEL     = 0x18E5; // パネル背景 (#1B2026)
    constexpr uint16_t STATUS_BG    = 0x0841; // ステータスバー背景 (#0B0F14)
    constexpr uint16_t TEXT_MAIN    = 0xFFFF; // 白 (#FFFFFF)
    constexpr uint16_t TEXT_MUTED   = 0x9CD3; // 薄いグレー (#9AA0A6)
    constexpr uint16_t ACCENT_BLUE  = 0x247F; // シアン/ブルー (#2196F3)
    constexpr uint16_t ACCENT_GREEN = 0x2E69; // エメラルドグリーン (#2ECC71)
    constexpr uint16_t ACCENT_WARN  = 0xFA40; // オレンジ (#F39C12)
    constexpr uint16_t ACCENT_ALERT = 0xE986; // レッド (#E74C3C)
    constexpr uint16_t BTN_DEFAULT  = 0x2967; // 通常ボタン (#2B303A)
    constexpr uint16_t BTN_PRESSED  = 0x420C; // 押下時ボタン (#434A59)
    constexpr uint16_t KEY_NUM_BG   = 0x2124; // 数字キー背景
    constexpr uint16_t KEY_FUNC_BG  = 0x39E7; // 機能キー(C, +/-, %)背景
    constexpr uint16_t KEY_OP_BG    = 0xFA40; // 演算子キー背景 (オレンジ)
    constexpr uint16_t KEY_OP_PRESS = 0xC260; // 演算子押下時
}

// ==========================================
// システム & 省電力設定
// ==========================================
// スリープタイムアウト秒 (0: スリープ無効 / デフォルト)
constexpr uint32_t DEFAULT_SLEEP_TIMEOUT_SEC = 0; 

// 初期バックライト輝度 (0 - 255)
constexpr uint8_t DEFAULT_BRIGHTNESS = 128;

// NVS(Preferences) のキー名
namespace NvsKeys {
    constexpr const char* NAMESPACE        = "m5_template";
    constexpr const char* KEY_SLEEP_SEC    = "sleep_sec";
    constexpr const char* KEY_BRIGHTNESS   = "brightness";
}

// ==========================================
// タスク設定 (FreeRTOS)
// ==========================================
constexpr uint32_t NETWORK_TASK_STACK_SIZE = 8192; // WiFi & NTP 用に拡張
constexpr UBaseType_t NETWORK_TASK_PRIORITY = 1;
constexpr BaseType_t NETWORK_TASK_CORE = 0; // バックグラウンド通信は Core 0
