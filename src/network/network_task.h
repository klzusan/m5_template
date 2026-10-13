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

private:
    NetworkTaskManager() = default;

    TaskHandle_t taskHandle_ = nullptr;
    portMUX_TYPE stateMutex_ = portMUX_INITIALIZER_UNLOCKED;
    NetworkState state_;

    static void taskEntry(void* param);
    void runTask();
    void syncNtpTime();
};
