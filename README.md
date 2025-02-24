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

## 想定機種

詳細は `platformio.ini` を参照してください。

- m5stack-core-esp32
- m5stack-grey
- m5stack-fire
- m5stack-cores2
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
