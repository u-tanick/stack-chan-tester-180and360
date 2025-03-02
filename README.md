# stack-chan-tester-180and360

## 概要

M5Stackに顔を表示させるライブラリ [M5Stack-Avatar](https://github.com/stack-chan/m5stack-avatar) を中心に開発されている手乗りロボット `ｽﾀｯｸﾁｬﾝ` での利用を想定し、
サーボモーターを従来の `180度回転を2個` ではなく、`180度回転と360度回転の2種` を想定したテストプログラムです。

従来のタイプのテストプログラムは、[stack-chan-tester](https://github.com/mongonta0716/stack-chan-tester) として @mongonta0716 さんにより公開されています。

本プログラムは、`stack-chan-tester` を素に、改変を行ったものですが、コード理解の参考になるようライブラリのincludeや変数の宣言を整理しなおしたものとなっています。

## 使用ライブラリ

詳細は `platformio.ini` を参照してください。

- meganetaaan/M5Stack-Avatar@0.10.0
- mongonta0716/stackchan-arduino
- gob/gob_unifiedButton @ ^0.1.5
- tobozo/M5Stack-SD-Updater

- その他
  - サーボモーターの制御は `ServoEasing` を使用しています。

## 想定サーボモーター

デフォルトでは180, 360ともに `TowerPro` のサーボモーターを想定した設定となっています。

- 180度回転サーボモーター
  - [TowerPro SG90](https://akizukidenshi.com/catalog/g/g108761/)
  - [FEETECH FS90](https://www.switch-science.com/products/7111)
  - [M5Stack Servo Kit 180](https://www.switch-science.com/products/6478)
- 360度回転サーボモーター
  - [TowerPro SG90-HV](https://akizukidenshi.com/catalog/g/g114382/)
  - [FEETECH FS90R](https://www.switch-science.com/products/7113)
  - [M5Stack Servo Kit 360](https://www.switch-science.com/products/6479)

使用するサーボモーターのメーカーを変更する場合は、180度サーボ、360度サーボそれぞれ以下の箇所で切り替え可能です。
これら以外のサーボモーターを使用する場合は、適宜関連するコードを改修してください。

``` cpp
// L:94 付近

// 使用する180サーボのメーカー
#define USE_Servo_180_TowerPro
// #define USE_Servo_180_Feetech360
// #define USE_Servo_180_M5Stack
```

``` cpp
// L:115 付近

// 使用する360サーボのメーカー
#define USE_Servo_360_TowerPro
// #define USE_Servo_360_Feetech360
// #define USE_Servo_360_M5Stack
```

## 想定M5Stack製品

詳細は `platformio.ini` を参照してください。デフォルトでは `m5stack-core2` となっています。

- m5stack-core-esp32
- m5stack-grey
- m5stack-fire
- m5stack-core2
- m5stack-cores3

## 提供機能

- Aボタン
  - テスト駆動モード ON/OFF
  - サーボの回転角度や回転速度を順次変化させていくモードです。
- Bボタン
  - 強制停止
  - テスト駆動、ランダム駆動のどちらの状態でも強制的に停止します。
- Cボタン
  - ランダム駆動モード ON/OFF
  - サーボの回転角度や回転速度をランダムに変化させるモードです。
