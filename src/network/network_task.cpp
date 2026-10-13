#include "network_task.h"
#include "config.h"
#include "secrets.h"
#include "utils/logger.h"
#include <time.h>
#include <esp_sntp.h>

static const char* TAG = "NetTask";

NetworkTaskManager::NetworkTaskManager() {
    mutex_ = xSemaphoreCreateMutex();
}

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
    NetworkState copy;
    if (mutex_ != nullptr && xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
        copy = state_;
        xSemaphoreGive(mutex_);
    }
    return copy;
}

void NetworkTaskManager::checkAndApplyRtcSync() {
    if (mutex_ != nullptr && xSemaphoreTake(mutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
        if (pendingRtcSync_) {
            pendingRtcSync_ = false;
            m5::rtc_datetime_t dt = syncedDateTime_;
            xSemaphoreGive(mutex_);

            // メインスレッド(Core 1)側から安全にI2C経由でRTC設定
            M5.Rtc.setDateTime(dt);
            LOG_I(TAG, "RTC synchronized safely from Main Thread: %04d/%02d/%02d %02d:%02d:%02d",
                  dt.date.year, dt.date.month, dt.date.date,
                  dt.time.hours, dt.time.minutes, dt.time.seconds);
            return;
        }
        xSemaphoreGive(mutex_);
    }
}

void NetworkTaskManager::runTask() {
    String ssid = WIFI_SSID;
    String pass = WIFI_PASSWORD;

    if (ssid.isEmpty()) {
        LOG_W(TAG, "Wi-Fi SSID is empty. Network task idle. (Check secrets.h)");
        if (mutex_ != nullptr && xSemaphoreTake(mutex_, portMAX_DELAY) == pdTRUE) {
            state_.status = NetworkStatus::NOT_CONFIGURED;
            xSemaphoreGive(mutex_);
        }
        
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    LOG_I(TAG, "Connecting to Wi-Fi SSID: %s", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    bool wasConnected = false;
    uint32_t lastNtpAttempt = 0;

    while (true) {
        wl_status_t wifiStatus = WiFi.status();

        if (wifiStatus == WL_CONNECTED) {
            String ip = WiFi.localIP().toString();
            int8_t rssi = WiFi.RSSI();

            bool justConnected = false;
            if (mutex_ != nullptr && xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
                if (state_.status != NetworkStatus::CONNECTED) {
                    justConnected = true;
                }
                state_.status = NetworkStatus::CONNECTED;
                state_.ipAddress = ip;
                state_.rssi = rssi;
                xSemaphoreGive(mutex_);
            }

            if (justConnected || !wasConnected) {
                wasConnected = true;
                LOG_I(TAG, "Wi-Fi Connected! IP: %s, RSSI: %d dBm", ip.c_str(), rssi);
                syncNtpTime();
                lastNtpAttempt = millis();
            } else {
                // 未同期の場合は30秒おき、同期済みの場合は1時間おきに再同期
                bool isSynced = false;
                if (mutex_ != nullptr && xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
                    isSynced = state_.ntpSynced;
                    xSemaphoreGive(mutex_);
                }
                uint32_t interval = isSynced ? (3600 * 1000) : (30 * 1000);
                if (millis() - lastNtpAttempt >= interval) {
                    lastNtpAttempt = millis();
                    syncNtpTime();
                }
            }
        } else {
            wasConnected = false;
            if (mutex_ != nullptr && xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
                state_.status = NetworkStatus::CONNECTING;
                state_.ipAddress = "---";
                state_.rssi = 0;
                xSemaphoreGive(mutex_);
            }
            LOG_D(TAG, "Connecting to Wi-Fi (status: %d)...", (int)wifiStatus);
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void NetworkTaskManager::syncNtpTime() {
    LOG_I(TAG, "Starting NTP sync (JST: UTC+9)...");

    // SNTP クライアント初期化
    if (sntp_enabled()) {
        sntp_stop();
    }
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, NTP_SERVER_1);
    sntp_setservername(1, NTP_SERVER_2);
    sntp_init();

    bool synced = false;

    // SNTP 同期完了を待機 (最大15秒: 500ms x 30回)
    // sntp_get_sync_status() が COMPLETED になることで、本物のNTPパケット到達を確実に検出
    for (int retry = 0; retry < 30; ++retry) {
        if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
            synced = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (synced) {
        // NTP で同期された UTC 時間を取得
        time_t utcNow = 0;
        time(&utcNow);

        // 日本標準時 (JST = UTC + 9時間 = 32400秒) に明示的に変換
        // OS/newlib の環境変数 TZ に依存せず、直接計算することでズレを100%防止
        time_t jstNow = utcNow + (9 * 3600);

        struct tm timeinfo;
        gmtime_r(&jstNow, &timeinfo);

        m5::rtc_datetime_t dt;
        dt.date.year    = timeinfo.tm_year + 1900;
        dt.date.month   = timeinfo.tm_mon + 1;
        dt.date.date    = timeinfo.tm_mday;
        dt.date.weekDay = timeinfo.tm_wday;
        dt.time.hours   = timeinfo.tm_hour;
        dt.time.minutes = timeinfo.tm_min;
        dt.time.seconds = timeinfo.tm_sec;

        if (mutex_ != nullptr && xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
            syncedDateTime_ = dt;
            pendingRtcSync_ = true;
            state_.ntpSynced = true;
            xSemaphoreGive(mutex_);
        }

        LOG_I(TAG, "NTP Sync SUCCESS (JST): %04d/%02d/%02d (%d) %02d:%02d:%02d",
              dt.date.year, dt.date.month, dt.date.date, dt.date.weekDay,
              dt.time.hours, dt.time.minutes, dt.time.seconds);
    } else {
        LOG_W(TAG, "NTP Sync timed out. Status: %d", (int)sntp_get_sync_status());
    }
}
