#pragma once

// Wi-Fi 認証情報テンプレート
// secrets.example.h を secrets.h にコピーし、実際のSSIDとパスワードを設定してください。
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// NTP サーバー設定 (デフォルト: 日本標準時 JST-9)
#define NTP_SERVER_1  "ntp.nict.jp"
#define NTP_SERVER_2  "pool.ntp.org"
#define TIMEZONE_OFFSET_SEC (9 * 3600) // JST (+9時間)
#define DAYLIGHT_OFFSET_SEC 0
