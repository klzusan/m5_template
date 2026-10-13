#include "network_task.h"
#include "config.h"
#include "secrets.h"
#include "utils/logger.h"
#include <time.h>

static const char* TAG = "NetTask";

void NetworkTaskManager::start() {
    if (taskHandle_ != nullptr) return;

    xTaskCreatePinnedToCore(
        taskEntry,
        "NetworkTask",
        NETWORK_TASK_STACK_SIZE,
        this,
        NETWORK_TASK_PRIORITY,
        &taskHandle_,
        NETWORK_TASK_CORE
    );
    LOG_I(TAG, "Network task started on Core %d", NETWORK_TASK_CORE);
}

void NetworkTaskManager::taskEntry(void* param) {
    auto* self = static_cast<NetworkTaskManager*>(param);
    self->runTask();
    vTaskDelete(nullptr);
}

NetworkState NetworkTaskManager::getState() {
    portENTER_CRITICAL(&stateMutex_);
    NetworkState copy = state_;
    portEXIT_CRITICAL(&stateMutex_);
    return copy;
}

void NetworkTaskManager::runTask() {
    String ssid = WIFI_SSID;
    String pass = WIFI_PASSWORD;

    if (ssid.isEmpty()) {
        LOG_W(TAG, "Wi-Fi SSID is empty. Network task idle. (Check secrets.h)");
        portENTER_CRITICAL(&stateMutex_);
        state_.status = NetworkStatus::NOT_CONFIGURED;
        portEXIT_CRITICAL(&stateMutex_);
        
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    LOG_I(TAG, "Connecting to Wi-Fi SSID: %s", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    while (true) {
        if (WiFi.status() == WL_CONNECTED) {
            bool wasDisconnected = false;
            portENTER_CRITICAL(&stateMutex_);
            if (state_.status != NetworkStatus::CONNECTED) {
                wasDisconnected = true;
            }
            state_.status = NetworkStatus::CONNECTED;
            state_.ipAddress = WiFi.localIP().toString();
            state_.rssi = WiFi.RSSI();
            portEXIT_CRITICAL(&stateMutex_);

            if (wasDisconnected) {
                LOG_I(TAG, "Wi-Fi Connected! IP: %s, RSSI: %d dBm", 
                      state_.ipAddress.c_str(), state_.rssi);
                syncNtpTime();
            }
        } else {
            portENTER_CRITICAL(&stateMutex_);
            state_.status = NetworkStatus::CONNECTING;
            state_.ipAddress = "---";
            state_.rssi = 0;
            portEXIT_CRITICAL(&stateMutex_);

            LOG_D(TAG, "Connecting to Wi-Fi...");
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void NetworkTaskManager::syncNtpTime() {
    LOG_I(TAG, "Starting NTP sync...");
    configTime(TIMEZONE_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 5000)) {
        // RTC にも反映
        m5::rtc_datetime_t dt;
        dt.date.year    = timeinfo.tm_year + 1900;
        dt.date.month   = timeinfo.tm_mon + 1;
        dt.date.date    = timeinfo.tm_mday;
        dt.date.weekDay = timeinfo.tm_wday;
        dt.time.hours   = timeinfo.tm_hour;
        dt.time.minutes = timeinfo.tm_min;
        dt.time.seconds = timeinfo.tm_sec;
        M5.Rtc.setDateTime(dt);

        portENTER_CRITICAL(&stateMutex_);
        state_.ntpSynced = true;
        portEXIT_CRITICAL(&stateMutex_);

        LOG_I(TAG, "NTP Sync success: %04d/%02d/%02d %02d:%02d:%02d",
              dt.date.year, dt.date.month, dt.date.date,
              dt.time.hours, dt.time.minutes, dt.time.seconds);
    } else {
        LOG_W(TAG, "NTP Sync failed/timed out.");
    }
}
