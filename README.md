# M5Stack Core2 開発テンプレート (PlatformIO + M5Unified)

M5Stack Core2 向けに設計された、堅牢で拡張性の高い C++ 開発テンプレートです。  
最新の **M5Unified** ライブラリを採用し、ESP32 のデュアルコア（FreeRTOS）を活かしたマルチタスク設計、M5Canvas によるフリッカーフリーな UI 描画、省電力スリープ制御、Wi-Fi/NTP 自動同期を備えています。

---

## 🌟 主な特徴

- **M5Unified 完全対応**: Core2 のディスプレイ、静電容量タッチパネル、電源管理 IC (AXP192/AXP2101)、RTC、振動モーターを統一インターフェースで制御。
- **デュアルコア並行処理 (FreeRTOS)**:
  - **Core 1 (メイン)**: ユーザー入力検知、システム状態更新、UI 描画（約 30 FPS）。
  - **Core 0 (バックグラウンド)**: Wi-Fi 接続維持、自動再接続、NTP 時刻同期。
- **フリッカーフリーな UI アーキテクチャ**:
  - `M5Canvas`（スプライトメモリ）によるオフスクリーン描画。
  - 常時表示ステータスバー（Wi-Fi、現在時刻、バッテリー残量・充電状態）。
  - **ホーム画面（ランチャー）＆ アプリ切り替え構造**: `AppBase` を継承して `src/app/` 配下に機能を追加可能。
  - **ホーム復帰ショートカット**: 各アプリ実行中に画面下部 **BtnB を 10秒長押し** すると、リアルタイムカウントダウン通知とともにホーム画面へ安全に復帰。
  - タッチイベント対応ボタンコンポーネント (`TouchButton`)。
- **電源・省電力管理 (`SystemManager`)**:
  - バッテリー残量・電圧・充電ステータスのモニタリング。
  - 無操作自動スリープ / 明示的なスリープ移行。
  - バックライト輝度やスリープ時間設定を NVS (`Preferences`) に自動保存・復元。
- **セキュアな設定管理**:
  - Wi-Fi 認証情報などの機密情報を `secrets.h` で分離管理（Git 対象外）。
  - テーマカラーや定数を `include/config.h` に一元化。
- **統一ロガー (`Logger`)**:
  - タイムスタンプ・ログレベル付き出力マクロ (`LOG_I`, `LOG_W`, `LOG_E`, `LOG_D`)。

---

## 📁 ディレクトリ構成

```text
m5_template/
├── include/
│   ├── config.h            # 画面サイズ、カラーテーマ、FPS、定数設定
│   ├── secrets.example.h   # Wi-Fi / NTP 設定テンプレート
│   └── secrets.h           # (Git管理外) 実際の Wi-Fi SSID / パスワード
├── src/
│   ├── main.cpp            # アプリケーションエントリポイント (setup / loop)
│   ├── app/                # アプリケーション機能・拡張ロジック
│   │   ├── app_base.h      # アプリ基底クラス (AppBase)
│   │   ├── calculator/     # 計算機アプリ (CalculatorApp)
│   │   └── system_info/    # システム情報アプリ (SystemInfoApp)
│   ├── network/            # バックグラウンド通信タスク (Core 0)
│   │   ├── network_task.h
│   │   └── network_task.cpp
│   ├── system/             # 電源、バッテリー、スリープ、NVS設定管理
│   │   ├── system_manager.h
│   │   └── system_manager.cpp
│   ├── ui/                 # UI制御、ステータスバー、ボタン描画
│   │   ├── status_bar.h / status_bar.cpp
│   │   ├── touch_button.h / touch_button.cpp
│   │   └── ui_manager.h / ui_manager.cpp
│   └── utils/              # ユーティリティ (ロガー等)
│       └── logger.h
├── platformio.ini          # PlatformIO プロジェクト設定
└── README.md
```

---

## 🚀 クイックスタート

### 1. 動作環境
- **ハードウェア**: M5Stack Core2 (または M5Stack Core2 for AWS)
- **開発環境**: Visual Studio Code + [PlatformIO IDE 拡張機能](https://platformio.org/)

### 2. Wi-Fi 設定ファイルの作成
`include/secrets.example.h` をコピーして `include/secrets.h` を作成し、接続先 Wi-Fi 情報を入力します。

```bash
# PowerShell
Copy-Item include/secrets.example.h include/secrets.h
```

`include/secrets.h` を開き、SSID とパスワードを設定します:
```cpp
#define WIFI_SSID     "あなたのWi-FiのSSID"
#define WIFI_PASSWORD "あなたのWi-Fiのパスワード"
```
*(※ `secrets.h` は `.gitignore` に登録されているため、Git にコミットされる心配はありません)*

### 3. ビルドと書き込み
M5Stack Core2 を PC に USB 接続し、VS Code 下部の PlatformIO ツールバー、またはターミナルから書き込みます。

- **ビルド**: `pio run -e m5stack-core2`
- **書き込み**: `pio run -e m5stack-core2 -t upload`
- **シリアルモニタ (115200 bps)**: `pio run -e m5stack-core2 -t monitor`

---

## 🛠️ アプリケーション（機能）の開発・管理

本テンプレートでは、すべての機能が `AppBase`（[`src/app/app_base.h`]）を継承した独立したモジュールとして設計されています。ホーム画面（ランチャー）から各アプリを切り替えて起動できます。

---

### 1. 新しいアプリを追加する手順

例として「タイマーアプリ（`TimerApp`）」を新しく追加する場合の手順です。

#### Step 1: ディレクトリとクラスの作成
`src/app/timer/` ディレクトリを作成し、`timer_app.h` と `timer_app.cpp` を作成します。

```cpp
// src/app/timer/timer_app.h
#pragma once
#include "app/app_base.h"
#include "config.h"

class TimerApp : public AppBase {
public:
    TimerApp() = default;

    // アプリ基本情報 (ホーム画面のカードに表示されます)
    const char* getName() const override { return "Timer"; }
    const char* getDescription() const override { return "Countdown Timer"; }
    uint16_t getThemeColor() const override { return Theme::ACCENT_WARN; } // テーマ色

    // ライフサイクルコールバック
    void onEnter() override { /* 画面開始時の初期化 */ }
    void onExit() override  { /* 画面離脱時のクリーンアップ */ }

    // 毎フレームの入力更新・描画 (offsetY = StatusBar高さ: 24)
    void update() override {
        // M5.Touch やボタン入力処理
    }
    void render(M5Canvas& canvas, int offsetY = 24) override {
        canvas.fillSprite(Theme::BG_DARK);
        // canvas.drawString(...) 等で描画
    }
};
```

#### Step 2: `UIManager` にアプリを登録
[`src/ui/ui_manager.h`] と [`src/ui/ui_manager.cpp`] にアプリを追加します。

1. **ヘッダー ([`src/ui/ui_manager.h`])**:
   ```cpp
   #include "app/timer/timer_app.h" // インクルード追加

   class UIManager {
       // ...
   private:
       CalculatorApp calcApp_;
       SystemInfoApp sysInfoApp_;
       TimerApp timerApp_; // ★ インスタンス変数を追加
   };
   ```

2. **実装 ([`src/ui/ui_manager.cpp`])**:
   `init()` メソッド内の `apps_.push_back(...)` に追加します。
   ```cpp
   void UIManager::init() {
       // ...
       apps_.clear();
       apps_.push_back(&calcApp_);
       apps_.push_back(&sysInfoApp_);
       apps_.push_back(&timerApp_); // ★ リストに登録
       // ...
   }
   ```
これだけで自動的にホーム画面のカルーセルに新しいカードが追加され、BtnA / BtnC で選択して起動できるようになります！

---

### 2. アプリを削除する手順

不要になったアプリを取り除く場合は、追加の逆の手順を行います。

1. **`UIManager::init()` からの登録解除 ([`src/ui/ui_manager.cpp`])**:
   `apps_.push_back(...)` の対象行を削除（またはコメントアウト）します。
   ```cpp
   // apps_.push_back(&sysInfoApp_); // 削除したいアプリを除外
   ```
2. **`UIManager` からのインスタンス・インクルード削除 ([`src/ui/ui_manager.h`])**:
   メンバ変数宣言と `#include` 行を削除します。
3. **不要なソースコードの削除**:
   `src/app/<削除したいアプリ名>/` ディレクトリを削除します。

---

### 3. アプリの並び順を変更する手順

ホーム画面（カルーセル）で表示されるアプリの並び順は、[`src/ui/ui_manager.cpp`] の `apps_.push_back(...)` の **記述順** で決定されます。

```cpp
// 例: System Info を最初(デフォルト)に表示したい場合
apps_.clear();
apps_.push_back(&sysInfoApp_); // 1番目 (初期選択)
apps_.push_back(&calcApp_);    // 2番目
```

---

## 外部センサー（I2C / ADC 等）の追加

センサー読み取りや外部デバイス連携を追加する場合の推奨構成です。

1. **`src/sensors/` などのディレクトリを作成し、センサー管理クラスを作成:**
   ```cpp
   // src/sensors/env_sensor.h
   #pragma once
   #include <Arduino.h>

   class EnvSensor {
   public:
       static EnvSensor& instance() {
           static EnvSensor inst;
           return inst;
       }
       bool init() {
           // I2C センサーなどの初期化処理
           return true;
       }
       float getTemperature() { return 24.5f; }
   };
   ```

2. **`src/main.cpp` の `setup()` で初期化:**
   ```cpp
   #include "sensors/env_sensor.h"

   void setup() {
       // ... 既存初期化処理 ...
       EnvSensor::instance().init();
   }
   ```

3. **`UIManager` やタスクからデータを取得して描画・送信:**
   UI で定期表示する場合は、`UIManager::drawMainContent()` 内でセンサー値を取得して `mainCanvas_.printf(...)` で描画します。

---

## バックグラウンド通信タスク（Web API / MQTT）の拡張

Core 0 で動作する `NetworkTaskManager` (`src/network/network_task.cpp`) は、メインループをブロックせずに通信を行うためのタスクです。

1. **状態構造体に共有データを追加 (`src/network/network_task.h`):**
   ```cpp
   struct NetworkState {
       NetworkStatus status = NetworkStatus::NOT_CONFIGURED;
       String ipAddress = "0.0.0.0";
       int8_t rssi = 0;
       bool ntpSynced = false;
       float fetchedServerValue = 0.0f; // 共有したいデータを追加
   };
   ```

2. **通信タスクループ (`network_task.cpp`) 内で API 取得処理を実行:**
   ```cpp
   void NetworkTaskManager::runTask() {
       // ...
       while (true) {
           if (WiFi.status() == WL_CONNECTED) {
               // 定期的な HTTP GET や MQTT 送受信
               // HTTPClient http;
               // ...
               
               // 排他制御 (portENTER_CRITICAL / portEXIT_CRITICAL) を使って状態を更新
               portENTER_CRITICAL(&stateMutex_);
               state_.fetchedServerValue = 123.45f;
               portEXIT_CRITICAL(&stateMutex_);
           }
           vTaskDelay(pdMS_TO_TICKS(10000)); // 10秒おき
       }
   }
   ```

3. **Core 1 (メイン側) から安全に値を取得:**
   `NetworkTaskManager::instance().getState()` を呼び出すと、排他制御された安全なコピーが返されます。

---

### パターン 4: UI テーマカラーや設定値の変更

`include/config.h` に主要な定数がまとまっています。

- **カラーパレット**: `Theme::` 名前空間に定義された RGB565 形式のカラーコードを変更することで、全体の配色を一括変更できます。
- **画面リフレッシュレート**: `TARGET_FPS` (デフォルト 30) で調整可能です。
- **自動スリープ秒数**: `DEFAULT_SLEEP_TIMEOUT_SEC`（デフォルト 0: 無効）を秒単位で設定すると、画面無操作時に自動で省電力スリープへ移行します。

---

## 💡 開発時の Tips & 注意点

- **画面更新のルール**:  
  直接 `M5.Display` に描画するとフリッカー（画面のちらつき）の原因になります。必ず `M5Canvas` (`statusCanvas_` または `mainCanvas_`) に対して描画し、フレーム更新の最後に `pushSprite()` してください。
- **タッチ操作の認識**:  
  `M5.update()` が毎フレーム `loop()` で呼ばれている必要があります。タッチ座標判定は `M5.Touch.getDetail()` を使用しています。
- **ハードウェアボタン (A/B/C)**:  
  Core2 の液晶下部にある 3 つの赤い丸は仮想ボタン（`BtnA`, `BtnB`, `BtnC`）です。`M5.BtnA.wasClicked()` 等で簡単にタップを検出できます。

---

## 📜 ライセンス

本テンプレートは自由に変更・商用利用が可能です。
