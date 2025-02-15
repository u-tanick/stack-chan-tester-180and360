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
#include <ESP32Servo.h>

Servo servo180;  // 180度サーボ
Servo servo360;  // 360度サーボ

// const int servo180Pin = 26;  // 180度サーボのピン
// const int servo360Pin = 33;  // 360度サーボのピン

bool isRunning = false;  // サーボ動作のフラグ
unsigned long prevTime180 = 0, prevTime360 = 0;
unsigned long interval180 = 0, interval360 = 0;
int angle180 = 0;
bool increasing = true;

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
// OLD version
// #include <ESP32Servo.h>
// /**
//  * 360度サーボのPWM値
//  * 時計回り   : 500 - 1500US : 0度
//  * 停止       : 1500US : 90度
//  * 反時計周り : 1500 - 2500US : 180度
// */
// Servo servo_X;                               // 360度サーボ
// ServoEasing servo_Y;                         // 180度サーボ
// #define START_DEGREE_VALUE_Y 90
// int servo_offset_y = 0;                      // Y軸サーボのオフセット（サーボの初期位置からの+-で設定）

// // --------------------------------
// // 360度サーボ動作用
// // 停止
// void stop360servo() {
//   servo_X.write(90);
// }

// // 停止よりマイナス
// void move360servoPlus() {
//   stop360servo();
//   delay(1);
//   servo_X.write(65);
//   delay(5000);
//   stop360servo();
// }

// // 停止よりプラス
// void move360servoMinus() {
//   stop360servo();
//   delay(1);
//   servo_X.write(105);
//   delay(5000);
//   stop360servo();
// }

// // --------------------------------
// // 180度サーボ動作用
// // サーボモーター：Y位置指定
// void moveY(int y, uint32_t millis_for_move = 0) {
//   if (millis_for_move == 0) {
//     servo_Y.setEaseTo(y + servo_offset_y);
//   } else {
//     servo_Y.setEaseToD(y + servo_offset_y, millis_for_move);
//   }
//   // サーボが停止するまでウェイトします。
//   synchronizeAllServosStartAndWaitForAllServosToStop();
// }

// // --------------------------------
// // 複合挙動用
// // ランダムモード
// void moveServoRandom() {
//   for (;;) {
//     delay(500);
//   }
//   M5.Speaker.tone(2500, 500);
// }

// // テストモード
// void moveServoTest() {
//   // サーボのテストの動き
//   for (int i=0; i<2; i++) { // 同じ動きを2回繰り返す。
//     avatar.setSpeechText("Y center -> lower  ");
//     moveY(system_config.getServoInfo(AXIS_Y)->lower_limit, 1000); // 上を向く
//     avatar.setSpeechText("Y lower -> upper  ");
//     moveY(system_config.getServoInfo(AXIS_Y)->upper_limit, 1000); // 下を向く
//     avatar.setSpeechText("Hello!!");
//     moveY(system_config.getServoInfo(AXIS_Y)->start_degree, 500); //正面を向く
//     avatar.setSpeechText("Move Leg.");                            // 足をバタバタ
//     move360servoPlus();
//     move360servoMinus();
//     avatar.setSpeechText("More Move.");                           // もう一回バタバタ
//     move360servoPlus();
//     move360servoMinus();
//     avatar.setSpeechText("Finish!!");
//   }
// }

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
  
  servo180.attach(SERVO_180_PIN);
  servo360.attach(SERVO_360_PIN);

  // OLD version
  // サーボの初期化を行います。（このとき、初期位置（正面）を向きます。）
  // X軸方向の初期化（360度サーボ）
  // servo_X.setPeriodHertz(50);
  // if (servo_X.attach(SERVO_360_PIN, 500, 2500)) {
  //   Serial.print("Error attaching servo X");
  // }
  // // Y軸方向の初期化（180度サーボ）
  // Serial.print("Success attached servo x\n");
  // if (servo_Y.attach(SERVO_180_PIN,
  //                    START_DEGREE_VALUE_Y + servo_offset_y,
  //                    DEFAULT_MICROSECONDS_FOR_0_DEGREE,
  //                    DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
  //   Serial.print("Error attaching servo y\n");
  // }
  // Serial.print("Success attached servo y\n");
  // servo_Y.setEasingType(EASE_QUADRATIC_IN_OUT);
  // // サーボモーター初期位置設定
  // moveY(system_config.getServoInfo(AXIS_Y)->start_degree, 500);

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
  // バッテリーアイコンの表示／非表示
  avatar.setBatteryIcon(true);
  // フォントの指定
  avatar.setSpeechFont(&fonts::lgfxJapanGothicP_16);

  // last_mouth_millis = millis();    // loop内で使用するのですが、処理を止めずにタイマーを実行するための変数です。一定時間で口を開くのとセリフを切り替えるのに利用します。
  //moveRandom();
  //testServo();

  randomSeed(analogRead(0));
  interval180 = random(5000, 10001); // 5秒〜10秒のランダム間隔
  interval360 = random(7000, 30001); // 7秒〜30秒のランダム間隔

}

// ----------------------------------------------
char speechText[100];  // フォーマットされた文字列を格納するためのバッファ
void loop() {

// CoreS3のボタン処理有効化（画面=タッチパネルを[右][中央][左]に3等分した領域がそれぞれBtnA, BtnB, BtnCに対応）
// 関数内でボタン処理を行う場合は、各関数の中でM5.update();とセットで必ず宣言
#ifdef ARDUINO_M5STACK_CORES3
  unifiedButton.update(); // M5.update() よりも前に呼ぶ事
#endif
  M5.update();

  // === ボタンAが押されたら動作開始/停止を切り替え ===
  if (M5.BtnA.wasPressed()) {
    isRunning = !isRunning;
    if (!isRunning) {
      servo180.write(90);  // 180°サーボを中間位置に戻す
      servo360.write(90);  // 360°サーボを停止
    }
  }

  if (!isRunning) return;  // 停止中なら何もしない

  unsigned long currentMillis = millis();

  // === 180°サーボの動作 (5秒〜10秒間隔) ===
  if (currentMillis - prevTime180 >= interval180) {
    prevTime180 = currentMillis;
    interval180 = random(5000, 10001); // 5秒〜10秒のランダム間隔

    if (increasing) {
      angle180 += 45;
      if (angle180 >= 180) increasing = false;
    } else {
      angle180 -= 45;
      if (angle180 <= 0) increasing = true;
    }
    servo180.write(angle180);
  }

  // === 360°サーボの動作 (7秒〜30秒間隔) ===
  if (currentMillis - prevTime360 >= interval360) {
    prevTime360 = currentMillis;
    interval360 = random(7000, 30001); // 7秒〜30秒のランダム間隔

    int speed = random(0, 181);
    servo360.write(speed);
  }

  // // ボタンA
  // // スピーカーを鳴らす、M5Stack-Avatarの表情変更、M5Stack-Avatarの台詞表示
  // if (M5.BtnA.wasPressed()) {
  //     M5.Speaker.tone(1000, 200);
  //     avatar.setExpression(Expression::Happy);
  //     avatar.setSpeechText("御用でしょうか？");
  // }

  // // ボタンB
  // // M5Stack-Avatarの台詞をテキスト変数で渡して表示、変数をログに出力
  // if (M5.BtnB.wasPressed()) {
  //     M5.Speaker.tone(1500, 200);
  //     avatar.setExpression(Expression::Neutral);
  //     char buff[100];
  //     sprintf(buff,"こんにちわ！");
  //     avatar.setSpeechText(buff);
  //     M5_LOGI("SpeechText: %c\n", buff);
  // }

  // // ボタンC
  // // M5Stack-Avatarの顔変更
  // if (M5.BtnC.wasPressed()) {
  //     M5.Speaker.tone(2000, 200);
  //     // cp = new ColorPalette();
  //     // cp->set(COLOR_PRIMARY, TFT_BLACK);
  //     // cp->set(COLOR_BACKGROUND, TFT_WHITE);
  //     // avatar.setColorPalette(*cp);
  //     // avatar.setFace(new SacabambaspisFace());
  //     // avatar.setSpeechText("うにょんうにょん");

  //     // moveServoTest();
  // }

  // delay(1);
}
