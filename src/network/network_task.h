#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <M5Unified.h>

enum class NetworkStatus {
    NOT_CONFIGURED,
    CONNECTING,
    CONNECTED,
    DISCONNECTED
};

struct NetworkState {
    NetworkStatus status = NetworkStatus::NOT_CONFIGURED;
    String ipAddress = "0.0.0.0";
    int8_t rssi = 0;
    bool ntpSynced = false;
};

class NetworkTaskManager {
public:
    static NetworkTaskManager& instance() {
        static NetworkTaskManager inst;
        return inst;
    }

    void start();
    NetworkState getState();

    // メインスレッド(Core 1)から定期的に呼び出して、I2Cバスの競合なしにRTCを更新
    void checkAndApplyRtcSync();

private:
    NetworkTaskManager();

    TaskHandle_t taskHandle_ = nullptr;
    SemaphoreHandle_t mutex_ = nullptr;
    NetworkState state_;

    bool pendingRtcSync_ = false;
    m5::rtc_datetime_t syncedDateTime_{};

    static void taskEntry(void* param);
    void runTask();
    void syncNtpTime();
};
