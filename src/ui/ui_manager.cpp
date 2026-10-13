#include "ui_manager.h"
#include "network/network_task.h"
#include "system/system_manager.h"
#include "utils/logger.h"

static const char *TAG = "UIManager";

void UIManager::init() {
  // 1. ステータスバー Canvas (320x24)
  statusBar_.init(statusCanvas_);

  // 2. メイン領域 Canvas (320x216)
  mainCanvas_.setColorDepth(16);
  mainCanvas_.setPsram(true);
  if (!mainCanvas_.createSprite(SCREEN_WIDTH, MAIN_CANVAS_HEIGHT)) {
    LOG_W(TAG,
          "Failed to create main canvas with PSRAM, falling back to SRAM...");
    mainCanvas_.setPsram(false);
    mainCanvas_.createSprite(SCREEN_WIDTH, MAIN_CANVAS_HEIGHT);
  }

  // 3. アプリケーションリストの初期化
  apps_.clear();
  apps_.push_back(&calcApp_);
  apps_.push_back(&sysInfoApp_);
  selectedAppIndex_ = 0;
  currentApp_ = nullptr;

  LOG_I(TAG, "UIManager initialized in Home Screen mode with %d apps.",
        (int)apps_.size());
}

void UIManager::launchApp(AppBase *app) {
  if (app == nullptr)
    return;

  if (currentApp_ != nullptr) {
    currentApp_->onExit();
  }

  currentApp_ = app;
  btnBPressStartTime_ = 0;
  M5.Speaker.tone(2000, 25);
  SystemManager::instance().reportUserActivity();

  currentApp_->onEnter();
  LOG_I(TAG, "Launched App: %s", app->getName());
}

void UIManager::returnToHome() {
  if (currentApp_ != nullptr) {
    LOG_I(TAG, "Exiting app: %s to Home", currentApp_->getName());
    currentApp_->onExit();
    currentApp_ = nullptr;
  }
  btnBPressStartTime_ = 0;
  SystemManager::instance().reportUserActivity();
}

void UIManager::checkBtnBHomeTrigger() {
  if (M5.BtnB.isPressed()) {
    if (btnBPressStartTime_ == 0) {
      btnBPressStartTime_ = millis();
    }
    uint32_t elapsed = millis() - btnBPressStartTime_;
    SystemManager::instance().reportUserActivity();

    if (elapsed >= HOLD_HOME_THRESHOLD_MS) {
      // 10秒長押し達成
      M5.Speaker.tone(1500, 80);
      delay(100);
      M5.Speaker.tone(2200, 150);
      returnToHome();
    }
  } else {
    btnBPressStartTime_ = 0;
  }
}

void UIManager::update() {
  if (currentApp_ != nullptr) {
    // --- アプリ実行中 ---
    // 1. BtnB 10秒長押しでホーム復帰を監視
    checkBtnBHomeTrigger();

    // ホームに戻っていなければアプリの更新を実行
    if (currentApp_ != nullptr) {
      currentApp_->update();
    }
  } else {
    // --- ホーム画面 (ランチャー) ---
    int totalApps = (int)apps_.size();

    bool btnTriggered = false;

    // BtnA: 前のアプリへ
    if (M5.BtnA.wasClicked()) {
      btnTriggered = true;
      if (totalApps > 0) {
        selectedAppIndex_ = (selectedAppIndex_ - 1 + totalApps) % totalApps;
        M5.Speaker.tone(1600, 15);
        SystemManager::instance().reportUserActivity();
        LOG_I(TAG, "Selected App (Prev): %s",
              apps_[selectedAppIndex_]->getName());
      }
    }

    // BtnC: 次のアプリへ
    if (M5.BtnC.wasClicked()) {
      btnTriggered = true;
      if (totalApps > 0) {
        selectedAppIndex_ = (selectedAppIndex_ + 1) % totalApps;
        M5.Speaker.tone(1800, 15);
        SystemManager::instance().reportUserActivity();
        LOG_I(TAG, "Selected App (Next): %s",
              apps_[selectedAppIndex_]->getName());
      }
    }

    // 画面タップで選択中アプリを起動
    // 注: Core2のBtnA/B/Cはタッチパネル最下部(Y>=240)のため、Y>=220のタッチやBtn押下時は除外
    auto touchDetail = M5.Touch.getDetail();
    if (!btnTriggered && !M5.BtnA.isPressed() && !M5.BtnC.isPressed() && touchDetail.wasClicked()) {
      int ty = touchDetail.y;
      int pty = touchDetail.prev_y;
      if (ty >= STATUS_BAR_HEIGHT && ty < 220 && pty < 220 && totalApps > 0) {
        launchApp(apps_[selectedAppIndex_]);
      }
    }
  }

  // ステータスバーの状態更新
  statusBar_.update(SystemManager::instance().getBatteryStatus(),
                    NetworkTaskManager::instance().getState());
}

void UIManager::render() {
  // 1. ステータスバー描画
  statusBar_.draw(statusCanvas_);
  statusCanvas_.pushSprite(0, 0);

  // 2. メイン領域描画
  if (currentApp_ != nullptr) {
    // アプリ描画
    currentApp_->render(mainCanvas_, STATUS_BAR_HEIGHT);

    // BtnB長押し中のプログレスバーをオーバーレイ描画
    if (btnBPressStartTime_ > 0) {
      uint32_t elapsed = millis() - btnBPressStartTime_;
      drawHoldProgressBar(elapsed);
    }
  } else {
    // ホーム画面描画
    drawHomeContent();
  }

  mainCanvas_.pushSprite(0, STATUS_BAR_HEIGHT);
}

void UIManager::drawHomeContent() {
  mainCanvas_.fillSprite(Theme::BG_DARK);

  if (apps_.empty())
    return;
  AppBase *selApp = apps_[selectedAppIndex_];
  uint16_t themeCol = selApp->getThemeColor();

  // 中央のアプリ選択カード
  const int cardW = 260;
  const int cardH = 120;
  const int cardX = (SCREEN_WIDTH - cardW) / 2;
  const int cardY = 16;

  // カード背景・境界線
  mainCanvas_.fillRoundRect(cardX, cardY, cardW, cardH, 10, Theme::BG_PANEL);
  mainCanvas_.drawRoundRect(cardX, cardY, cardW, cardH, 10, themeCol);
  mainCanvas_.drawRoundRect(cardX + 1, cardY + 1, cardW - 2, cardH - 2, 9,
                            themeCol);

  // アプリ番号表示 (例: [1 / 2])
  char pageStr[16];
  snprintf(pageStr, sizeof(pageStr), "[ %d / %d ]", selectedAppIndex_ + 1,
           (int)apps_.size());
  mainCanvas_.setFont(&fonts::Font0);
  mainCanvas_.setTextColor(Theme::TEXT_MUTED, Theme::BG_PANEL);
  mainCanvas_.setTextDatum(textdatum_t::top_right);
  mainCanvas_.drawString(pageStr, cardX + cardW - 12, cardY + 10);

  // アプリタイトル
  mainCanvas_.setFont(&fonts::Font4);
  mainCanvas_.setTextColor(themeCol, Theme::BG_PANEL);
  mainCanvas_.setTextDatum(textdatum_t::top_center);
  mainCanvas_.drawString(selApp->getName(), SCREEN_WIDTH / 2, cardY + 16);

  // アプリ説明
  mainCanvas_.setFont(&fonts::Font2);
  mainCanvas_.setTextColor(Theme::TEXT_MUTED, Theme::BG_PANEL);
  mainCanvas_.setTextDatum(textdatum_t::middle_center);
  mainCanvas_.drawString(selApp->getDescription(), SCREEN_WIDTH / 2,
                         cardY + 64);

  // 起動アクション案内
  mainCanvas_.setFont(&fonts::Font2);
  mainCanvas_.setTextColor(Theme::TEXT_MAIN, Theme::BG_PANEL);
  mainCanvas_.drawString("▶ TAP SCREEN TO LAUNCH ◀", SCREEN_WIDTH / 2,
                         cardY + 95);

  // ドットインジケーター (カード下)
  int dotCenterY = cardY + cardH + 12;
  int totalApps = (int)apps_.size();
  int dotSpacing = 16;
  int startDotX = SCREEN_WIDTH / 2 - ((totalApps - 1) * dotSpacing) / 2;

  for (int i = 0; i < totalApps; ++i) {
    int dotX = startDotX + i * dotSpacing;
    if (i == selectedAppIndex_) {
      mainCanvas_.fillCircle(dotX, dotCenterY, 4, themeCol);
    } else {
      mainCanvas_.fillCircle(dotX, dotCenterY, 3, Theme::BTN_DEFAULT);
    }
  }

  // 下部操作ガイド
  int footerY = MAIN_CANVAS_HEIGHT - 28;
  mainCanvas_.drawFastHLine(14, footerY - 4, SCREEN_WIDTH - 28,
                            Theme::BG_PANEL);

  mainCanvas_.setFont(&fonts::Font2);
  // BtnA
  mainCanvas_.setTextDatum(textdatum_t::middle_left);
  mainCanvas_.setTextColor(Theme::ACCENT_BLUE, Theme::BG_DARK);
  mainCanvas_.drawString("< [A] Prev", 20, footerY + 10);

  // BtnB
  mainCanvas_.setTextDatum(textdatum_t::middle_center);
  mainCanvas_.setTextColor(Theme::TEXT_MUTED, Theme::BG_DARK);
  mainCanvas_.setFont(&fonts::Font0);
  mainCanvas_.drawString("HOLD [B] 5s to return", SCREEN_WIDTH / 2,
                         footerY + 10);

  // BtnC
  mainCanvas_.setFont(&fonts::Font2);
  mainCanvas_.setTextDatum(textdatum_t::middle_right);
  mainCanvas_.setTextColor(Theme::ACCENT_BLUE, Theme::BG_DARK);
  mainCanvas_.drawString("Next [C] >", SCREEN_WIDTH - 20, footerY + 10);
}

void UIManager::drawHoldProgressBar(uint32_t elapsedMs) {
  if (elapsedMs < 200)
    return; // わずかなタップでのチラつきを防止

  int remainingSec = (int)((HOLD_HOME_THRESHOLD_MS - elapsedMs + 999) / 1000);
  if (remainingSec < 0)
    remainingSec = 0;

  // フローティングバナー
  int boxW = 240;
  int boxH = 50;
  int boxX = (SCREEN_WIDTH - boxW) / 2;
  int boxY = 70;

  mainCanvas_.fillRoundRect(boxX, boxY, boxW, boxH, 8, Theme::STATUS_BG);
  mainCanvas_.drawRoundRect(boxX, boxY, boxW, boxH, 8, Theme::ACCENT_WARN);

  // テキスト表示
  mainCanvas_.setTextDatum(textdatum_t::top_center);
  mainCanvas_.setFont(&fonts::Font2);
  mainCanvas_.setTextColor(Theme::TEXT_MAIN, Theme::STATUS_BG);
  char buf[48];
  snprintf(buf, sizeof(buf), "Hold [B] for Home: %ds", remainingSec);
  mainCanvas_.drawString(buf, SCREEN_WIDTH / 2, boxY + 6);

  // 進行状況バー
  int barW = 200;
  int barH = 6;
  int barX = (SCREEN_WIDTH - barW) / 2;
  int barY = boxY + 34;
  mainCanvas_.fillRoundRect(barX, barY, barW, barH, 3, Theme::BG_PANEL);

  int progressW = (barW * elapsedMs) / HOLD_HOME_THRESHOLD_MS;
  if (progressW > barW)
    progressW = barW;
  mainCanvas_.fillRoundRect(barX, barY, progressW, barH, 3, Theme::ACCENT_WARN);
}
