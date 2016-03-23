/*

   @file elecrab_beginner.ino
   @brief えれくらぶ動作サンプル
          左右に動くサーボモータのPIN番号(上側のサーボモータ)・・・5番
          上下に動くサーボモータのPIN番号(下側のサーボモータ)・・・6番
   @author Kei Takagi
   @date 2016.2.19

   Copyright (c) 2016 Kei Takagi
   Released under the MIT license
   http://opensource.org/licenses/mit-license.php

*/

#include <Servo.h>

//! 左右に動くサーボモータのPIN番号(上側のサーボモータ)
#define SERVO1 5
//! 上下に動くサーボモータのPIN番号(下側のサーボモータ)
#define SERVO2 6
//! 歩く速度のウェイト(μs)(数値が小さいほど早く歩きます)
#define SPEED  200

Servo Servo1;                      // 左右に動く足の角度(上側のサーボモータ)
Servo Servo2;                      // 上下に動く足の角度(下側のサーボモータ)

int Servo1_val[3] = {40,  90, 140}; // 左右に動く足の角度(上側のサーボモータ)
int Servo2_val[3] = {60,  90, 120}; // 上下に動く足の角度(下側のサーボモータ)

// ======================
// setup()関数はスケッチがスタートしたときに1度だけ呼び出されます
// ======================
void setup() {
  // ------------------------------------
  // えれくらぶの動作に関係する設定
  // ------------------------------------
  Servo1.attach(SERVO1);             // サーボモータの初期設定
  Servo2.attach(SERVO2);             // サーボモータの初期設定
  InitialPosition();                 // 初期位置
  delay(5000);                       // 5秒とめる
}

// ======================
// loop()関数は繰り返し実行されます。
// ======================
void loop() {
  front();                           // 前にすすむ
  front();                           // 前にすすむ
  front();                           // 前にすすむ
  delay(1000);                       // 1秒とまる

  back();                            // 後ろにすすむ
  back();                            // 後ろにすすむ
  back();                            // 後ろにすすむ
  delay(1000);                       // 1秒とまる

  rightOblique();                    // 右前にすすむ
  rightOblique();                    // 右前にすすむ
  rightOblique();                    // 右前にすすむ
  delay(1000);                       // 1秒とまる

  rightObliqueBack();                // 右後ろにすすむ
  rightObliqueBack();                // 右後ろにすすむ
  rightObliqueBack();                // 右後ろにすすむ
  delay(1000);                       // 1秒とまる

  leftOblique();                     // 左前にすすむ
  leftOblique();                     // 左前にすすむ
  leftOblique();                     // 左前にすすむ
  delay(1000);                       // 1秒とまる

  leftObliqueBack();                 // 左後ろにすすむ
  leftObliqueBack();                 // 左後ろにすすむ
  leftObliqueBack();                 // 左後ろにすすむ
  delay(1000);                       // 1秒とまる
}

// ======================
// 初期位置
// ======================
void InitialPosition() {
  Servo1.write(Servo1_val[1]);       // 左右に動くサーボモータの制御
  Servo2.write(Servo2_val[1]);       // 上下に動くサーボモータの制御
}

// ======================
// 真っ直ぐ進む
// ======================
void front() {
  rightOblique();
  leftOblique();
}

// ======================
// 後ろに進む
// ======================
void back() {
  rightObliqueBack();
  leftObliqueBack();
}

// ======================
// 右斜めに進む
// ======================
void rightOblique() {
  walk(1, 0);
  walk(0, 0);
  walk(0, 2);
  walk(1, 2);
  walk(1, 1);
}

// ======================
// 右斜め後ろに進む
// ======================
void rightObliqueBack() {
  walk(1, 2);
  walk(0, 2);
  walk(0, 0);
  walk(1, 0);
  walk(1, 1);
}

// ======================
// 左斜めに進む
// ======================
void leftOblique() {
  walk(1, 2);
  walk(2, 2);
  walk(2, 0);
  walk(1, 0);
  walk(1, 1);
}

// ======================
// 左斜め後ろに進む
// ======================
void leftObliqueBack() {
  walk(1, 0);
  walk(2, 0);
  walk(2, 2);
  walk(1, 2);
  walk(1, 1);
}


// ======================
// サーボモータの角度を変える
// ======================
void walk(int servo1Val, int servo2Val) {
  Servo1.write(Servo1_val[servo1Val]); // 左右に動くサーボモータの制御
  Servo2.write(Servo2_val[servo2Val]); // 上下に動くサーボモータの制御
  delay(SPEED);
}

