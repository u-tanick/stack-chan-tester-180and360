// ==================================
// 全体共通のヘッダファイルのinclude
#include <Arduino.h>                         // Arduinoフレームワークを使用する場合は必ず必要
#include <SD.h>                              // SDカードを使うためのライブラリです。
#include <Update.h>                          // 定義しないとエラーが出るため追加。
#include <Ticker.h>                          // 定義しないとエラーが出るため追加。
#include <M5StackUpdater.h>                  // M5Stack SDUpdaterライブラリ
#include <M5Unified.h>                       // M5Unifiedライブラリ
// ================================== End

// ==================================
// for Avatar
#include <Avatar.h>                          // 顔を表示するためのライブラリ https://github.com/meganetaaan/m5stack-avatar
using namespace m5avatar;                    // (Avatar.h)avatarのnamespaceを使う宣言（こうするとm5avatar::???と書かなくて済む。)
Avatar avatar;                               // (Avatar.h)avatarのクラスを定義
ColorPalette *cp;                            // アバターのカラーパレット用

// 表情を変更
void changeExpression(int num) {
  switch(num) {
    case 0:
      avatar.setExpression(Expression::Angry);
      break;
    case 1:
      avatar.setExpression(Expression::Doubt);
      break;
    case 2:
      avatar.setExpression(Expression::Happy);
      break;
    case 3:
      avatar.setExpression(Expression::Neutral);
      break;
    case 4:
      avatar.setExpression(Expression::Sad);
      break;
    case 5:
      avatar.setExpression(Expression::Sleepy);
      break;
    default:
      avatar.setExpression(Expression::Neutral);
  }
}
// ================================== End

// ==================================
// for 吹き出しテキスト関連
#include "formatString.hpp"                  // 文字列に変数の値を組み込むために使うライブラリ https://gist.github.com/GOB52/e158b689273569357b04736b78f050d6
uint32_t mouth_wait = 2000;                  // 通常時のセリフ入れ替え時間（msec）
uint32_t last_mouth_millis = 0;              // セリフを入れ替えた時間

const char* lyrics[] = { "BtnA:MoveTo90  ", "BtnB:ServoTest  ", "BtnC:RandomMode  ", "BtnALong:AdjustMode"};  // 通常モード時に表示するセリフ
const int lyrics_size = sizeof(lyrics) / sizeof(char*);  // セリフの数
int lyrics_idx = 0;                                      // 表示するセリフ数用の変数
// ================================== End

// ==================================
// for SystemConfig
#include <Stackchan_system_config.h>          // stack-chanの初期設定ファイルを扱うライブラリ
StackchanSystemConfig system_config;          // (Stackchan_system_config.h) プログラム内で使用するパラメータをYAMLから読み込むクラスを定義
// ================================== End

// ==================================
// for SD
#define SDU_APP_PATH "/stackchan_tester.bin"  // (SDUpdater.h)SDUpdaterで使用する変数
#define TFCARD_CS_PIN 4                       // SDカードスロットのCSPIN番号
// ================================== End

// ==================================
// for Servo
#include <ServoEasing.hpp>

/**
 * 360度サーボのPWM値
 * 時計回り   : 500 - 1500US : 0度
 * 停止       : 1500US : 90度
 * 反時計周り : 1500 - 2500US : 180度
*/

ServoEasing servo180;  // 180度サーボ
ServoEasing servo360;  // 360度サーボ

#define START_DEGREE_VALUE_SERVO_180  85  // 180度サーボ（Y軸方向）の初期角度
#define START_DEGREE_VALUE_SERVO_360  95  // 360度サーボ（X軸方向）の初期角度(=360度サーボの停止位置：試作に使用したsg90-hvの場合90だと停止しなかったのでこの値に設定)

const int MIN_PWM = 500;
const int MAX_PWM = 2400;

bool isRandomRunning = false;  // サーボ動作のフラグ
bool isTestRunning = false;    // サーボテスト動作のフラグ
unsigned long prevTime180 = 0, prevTime360 = 0;
unsigned long interval180 = 0, interval360 = 0;

int servo180_angle = 0; // 180サーボの角度用変数
int servo360_speed = 0; // 360サーボの速度用変数

// ボード種別毎のピン設定など
#ifdef ARDUINO_M5STACK_Core2
  // M5Stack Core2用のサーボの設定
  // Port.A X:G33, Y:G32
  // Port.C X:G13, Y:G14
  // スタックチャン基板 X:G19, Y:G27
  #define SERVO_360_PIN 33
  #define SERVO_180_PIN 32
#endif
#ifdef ARDUINO_M5STACK_FIRE 
  // M5Stack Fireの場合はPort.A(X:G22, Y:G21)のみです。
  // I2Cと同時利用は不可
  #define SERVO_360_PIN 22
  #define SERVO_180_PIN 21
#endif
#ifdef ARDUINO_M5Stack_Core_ESP32 
  // M5Stack Basic/Gray/Go用の設定
  // Port.A X:G22, Y:G21
  // Port.C X:G16, Y:G17
  // スタックチャン基板 X:G5, Y:G2
  #define SERVO_360_PIN 22
  #define SERVO_180_PIN 21
#endif
#ifdef ARDUINO_M5STACK_CORES3
  // M5Stack CoreS3用の設定 ※暫定的にplatformio.iniにARDUINO_M5STACK_CORES3を定義しています。
  // Port.A X:G1 Y:G2
  // Port.B X:G8 Y:G9
  // Port.C X:18 Y:17
  #define SERVO_360_PIN 18 
  #define SERVO_180_PIN 17
  #include <gob_unifiedButton.hpp> // 2023/5/12現在 M5UnifiedにBtnA等がないのでGobさんのライブラリを使用
  goblib::UnifiedButton unifiedButton;
#endif

bool core_port_a = false;         // Core1のPortAを使っているかどうか
// ================================== End

// ==================================
// for Servo Running Mode

// サーボのタイマーを初期化する関数
void initializeServoTimers() {
  unsigned long now = millis();
  prevTime180 = now;
  prevTime360 = now;
  interval180 = random(0, 101);  // ボタンを押した直後はすぐに動作するように初期値として短い時間を設定
  interval360 = random(0, 101);
}

// ランダムモード開始時にタイマーをリセット
void startRandomMode() {
  isRandomRunning = true;
  initializeServoTimers();  // タイマー初期化
}

// テストモード開始時にタイマーをリセット
void startTestMode() {
  isTestRunning = true;
  initializeServoTimers();  // タイマー初期化
}

// ランダムモード
void servoRandomRunningMode(unsigned long currentMillis) {

  // === 180°サーボの動作 (5秒〜10秒間隔) ===
  if (currentMillis - prevTime180 >= interval180) {
    prevTime180 = currentMillis;
    interval180 = random(5000, 10001); // 5秒〜10秒のランダム間隔

    // 90 ～ 50 度の間の角度（初期位置は85）
    int rand_speed_offset_180 = random(-5, 36);
    servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180 - rand_speed_offset_180);

    // 顔の向きが変わるタイミングで表情も変化させる
    changeExpression((rand_speed_offset_180 + 5) % 7);
  }

  // === 360°サーボの動作 (7秒〜30秒間隔) ===
  if (currentMillis - prevTime360 >= interval360) {
    prevTime360 = currentMillis;
    interval360 = random(7000, 30001); // 7秒〜30秒のランダム間隔

    // 66 ～ 114 度が示す速度（初期位置は90、2度刻み）
    int rand_speed_offset_360 = random(-12, 12)* 2;
    servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360 + rand_speed_offset_360);
  }

}

// テストモード
int count_180 = 0;
int count_360 = 0;
void servoTestRunningMode(unsigned long currentMillis) {

  // === 180°サーボの動作 (3秒間隔) ===
  if (currentMillis - prevTime180 >= interval180) {
    prevTime180 = currentMillis;
    interval180 = 3000; // 3秒間隔固定
    servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180 - (count_180 % 8) * 5);
    count_180 = (count_180 + 1) % 99;
  }

  // === 360°サーボの動作 (5秒間隔) ===
  if (currentMillis - prevTime360 >= interval360) {
    prevTime360 = currentMillis;
    interval360 = 5000; // 5秒間隔固定

    // 60 ～ 120 度が示す速度（初期位置は90）
    servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360 + (count_360 % 7) * 10 - 30);
    count_360 = (count_360 + 1) % 99;
  }
}
// ================================== End

// ----------------------------------------------
void setup() {
  // 設定用の情報を抽出
  auto cfg = M5.config();
  // Groveポートの出力をしない（m5atomS3用）
  // cfg.output_power = true;
  // M5Stackをcfgの設定で初期化
  M5.begin(cfg);

  M5.Speaker.setVolume(100);

#ifdef ARDUINO_M5STACK_FIRE 
  // M5Stack Fireの場合、Port.Aを使う場合は内部I2CをOffにする必要がある。
  M5.In_I2C.release();
#endif
#ifdef ARDUINO_M5STACK_CORES3 // CORES3のときに有効になります。
  // 画面上のタッチパネルを3分割してBtnA, B, Cとして使う設定。
  unifiedButton.begin(&M5.Display, goblib::UnifiedButton::appearance_t::transparent_all);
#endif

  // ログ設定
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);    // M5Unifiedのログ初期化（画面には表示しない。)
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);     // M5Unifiedのログ初期化（シリアルモニターにESP_LOG_INFOのレベルのみ表示する)
  M5.Log.setEnableColor(m5::log_target_serial, false);         // M5Unifiedのログ初期化（ログをカラー化しない。）
  M5_LOGI("Hello World");                                      // logにHello Worldと表示
  SD.begin(GPIO_NUM_4, SPI, 25000000);                         // SDカードの初期化
  delay(2000);                                                 // SDカードの初期化を少し待ちます。

  system_config.loadConfig(SD, "");                            // SDカードから初期設定ファイルを読み込む
  if (M5.getBoard() == m5::board_t::board_M5Stack) {           // Core1かどうかの判断
    if (system_config.getServoInfo(AXIS_X)->pin == 22) {       // サーボのGPIOが22であるか確認（本当は21も確認してもいいかもしれないが省略）
      // M5Stack Coreの場合、Port.Aを使う場合は内部I2CをOffにする必要がある。バッテリー表示は不可。
      M5_LOGI("I2CRelease");              // ログに出力
      avatar.setBatteryIcon(false);       // avatarのバッテリーアイコンを表示しないモードに設定
      M5.In_I2C.release();                // I2Cを使わないように設定(IMUやIP5306との通信をしないようにします。)※設定しないとサーボの動きがおかしくなります。
      core_port_a = true;                 // Core1でポートAを使用しているフラグをtrueに設定
    }
  } else {
    avatar.setBatteryIcon(true);          // Core2以降の場合は、バッテリーアイコンを表示する。
  }

  // servoの初期化
  M5_LOGI("attach servo");

  ESP32PWM::allocateTimer(0); // ESP32Servoはタイマーを割り当てる必要がある
  ESP32PWM::allocateTimer(1);

  servo180.setPeriodHertz(50);  // サーボ用のPWMを50Hzに設定
  servo360.setPeriodHertz(50);
  
  servo180.attach(SERVO_180_PIN, MIN_PWM, MAX_PWM);
  servo360.attach(SERVO_360_PIN, MIN_PWM, MAX_PWM);
  
  servo180.setEasingType(EASE_QUADRATIC_IN_OUT);       // 動きの最初と終わりがスムーズになる
  servo360.setEasingType(EASE_LINEAR);       // 一定の速度で動かす場合は EASE_LINEAR に変更

  setSpeedForAllServos(60);

  servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180);  // 180°サーボを初期位置にセット
  servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止位置にセット

  M5.Power.setExtOutput(!system_config.getUseTakaoBase());       // 設定ファイルのTakaoBaseがtrueの場合は、Groveポートの5V出力をONにする。
  M5_LOGI("ServoType: %d\n", system_config.getServoType());      // サーボのタイプをログに出力

  // 顔の色変更
  // TFT_WHITEなど既定の色が多数用意されている
  // オリジナルの色は以下のコードで定義可能
  // uint16_t customColor = 0;
  // customColor = M5.Lcd.color565(255,140,50);
  cp = new ColorPalette();
  cp->set(COLOR_PRIMARY, TFT_WHITE);
  cp->set(COLOR_BACKGROUND, TFT_BLACK);
  avatar.setColorPalette(*cp);
  avatar.init(8);
  // avatar.setBatteryIcon(true);                        // バッテリーアイコンの表示／非表示
  avatar.setSpeechFont(&fonts::lgfxJapanGothicP_12);  // フォントの指定

  last_mouth_millis = millis();    // loop内で使用するのですが、処理を止めずにタイマーを実行するための変数です。一定時間で口を開くのとセリフを切り替えるのに利用します。

  // ランダム動作用の変数初期化、個体差を出すためMACアドレスを使用する
  uint8_t mac[6];
  esp_efuse_mac_get_default(mac);
  uint32_t seed = mac[0] | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
  randomSeed(seed);
  interval180 = random(5000, 10001); // 5秒〜10秒のランダム間隔
  interval360 = random(7000, 30001); // 7秒〜30秒のランダム間隔

}

// ----------------------------------------------
char speechText[100];  // フォーマットされた文字列を格納するためのバッファ、Avatarの吹き出しに変数を使用する場合に使用
void loop() {

// CoreS3のボタン処理有効化（画面=タッチパネルを[右][中央][左]に3等分した領域がそれぞれBtnA, BtnB, BtnCに対応）
// 関数内でボタン処理を行う場合は、各関数の中でM5.update();とセットで必ず宣言
#ifdef ARDUINO_M5STACK_CORES3
  unifiedButton.update(); // M5.update() よりも前に呼ぶ事
#endif
  M5.update();

  // === ボタンAが押されたらテスト動作モードの開始/停止を切り替え ===
  if (M5.BtnA.wasPressed()) {
    M5.Speaker.tone(1500, 200);
    avatar.setExpression(Expression::Happy);
    avatar.setSpeechText("テストモード");
    if (!isTestRunning) {
      startTestMode();
    } else {
      isTestRunning = false;
      isRandomRunning = false;  // ランダムモードのフラグは強制終了
      servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180);  // 180°サーボを初期位置に戻す
      servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止
      avatar.setExpression(Expression::Neutral);
      avatar.setSpeechText("");
    }
  }

  // === 強制停止 ===
  if (M5.BtnB.wasPressed()) {
    M5.Speaker.tone(2500, 200);
    avatar.setSpeechText("停止");
    isTestRunning = false;
    isRandomRunning = false;  // ランダムモードのフラグは強制終了
    servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180);  // 180°サーボを初期位置に戻す
    servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止
    avatar.setExpression(Expression::Neutral);
    delay(1500);
    avatar.setSpeechText("");
}

    // === ボタンCが押されたらランダム動作モードの開始/停止を切り替え ===
  if (M5.BtnC.wasPressed()) {
    M5.Speaker.tone(1000, 200);
    avatar.setSpeechText("");
    if (!isRandomRunning) {
      startRandomMode();
    } else {
      isRandomRunning = false;
      isTestRunning = false;  // テストモードのフラグは強制終了
      servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180);  // 180°サーボを初期位置に戻す
      servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止
      avatar.setExpression(Expression::Neutral);
      avatar.setSpeechText("");
    }
  }

  if (!isRandomRunning && !isTestRunning) return;  // 停止中なら何もしない

  unsigned long currentMillis = millis();
  if (isRandomRunning) servoRandomRunningMode(currentMillis);
  if (isTestRunning) servoTestRunningMode(currentMillis);

}
