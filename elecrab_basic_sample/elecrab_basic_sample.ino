/*

    @file elecrab_basic_sample.ino
    @brief えれくらぶ動作サンプル
        赤外線リモコン受信モジュール(Vout)に接続ピン番号・・・2番
        赤外線リモコン受信モジュール(GND )に接続ピン番号・・・3番
        赤外線リモコン受信モジュール(VCC )に接続ピン番号・・・4番
        左右に動くサーボモータのPIN番号(上側のサーボモータ)・・・5番
        上下に動くサーボモータのPIN番号(下側のサーボモータ)・・・6番
        シリアル（シリアルモニタなど）からコマンドを送信する際はLE、LFを付けて下さい
    @author Kei Takagi
    @date 2016.7.13

    Copyright (c) 2016 Kei Takagi
    Released under the MIT license
    http://opensource.org/licenses/mit-license.php

*/

#include <IRControlReceiver.h>
#include <Servo.h>

// ↓コメントを外すとシリアルモニタに赤外線のデータが表示されます
//#define DEBUG

// 左右に動くサーボモータのPIN番号(上側のサーボモータ)
#define SERVO1 5
// 上下に動くサーボモータのPIN番号(下側のサーボモータ)
#define SERVO2 6
// 歩く速度のウェイト(μs)(数値が小さいほど早く歩きます)
#define SPEED  200

IRControlReceiver ir(2);                                          // 赤外線リモコン受信モジュール(Vout)に接続するArduino側のピン番号は2番
Servo Servo1;                                                       // 左右に動く足の角度(上側のサーボモータ)
Servo Servo2;                                                       // 上下に動く足の角度(下側のサーボモータ)

int Servo1_val[3] = {40,  90, 140};                                 // 左右に動く足の角度(上側のサーボモータ)
int Servo2_val[3] = {60,  90, 120};                                 // 上下に動く足の角度(下側のサーボモータ)

unsigned char ir_dat[IR_DATA_MAX_BYTE_SIZE];                        // 赤外線リモコンデータの受信領域

// 動作用赤外線リモコンデータ領域
unsigned char motiondata[7][IR_DATA_MAX_BYTE_SIZE] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // データなし
  0x00, 0xFF, 0x18, 0xE7, 0x00, 0x00, 0x00, 0x00, // 2:前
  0x00, 0xFF, 0x7A, 0x85, 0x00, 0x00, 0x00, 0x00, // 3:右斜め
  0x00, 0xFF, 0x52, 0xAD, 0x00, 0x00, 0x00, 0x00, // 9:右斜め後ろ
  0x00, 0xFF, 0x4A, 0xB5, 0x00, 0x00, 0x00, 0x00, // 8:後ろ
  0x00, 0xFF, 0x42, 0xBD, 0x00, 0x00, 0x00, 0x00, // 7:左斜め後ろ
  0x00, 0xFF, 0x30, 0xCF, 0x00, 0x00, 0x00, 0x00  // 1:左斜め
};

// ======================
// setup()関数はスケッチがスタートしたときに1度だけ呼び出されます
// ======================
void setup() {

#ifdef DEBUG
  // ------------------------------------
  // パソコンとシリアル通信するための設定
  // ------------------------------------
  Serial.begin(9600) ;     // シリアル通信設定：9600bps
#endif

  // ------------------------------------
  // 赤外線リモコン受信モジュール設定
  // ------------------------------------
  pinMode(3, OUTPUT);       // 赤外線リモコン受信モジュール PIN3 GND
  digitalWrite(3, LOW);
  pinMode(4, OUTPUT);       // 赤外線リモコン受信モジュール PIN4 VCC
  digitalWrite(4, HIGH);
  pinMode(13, OUTPUT);      // PIN13 LED

  // ------------------------------------
  // えれくらぶの動作に関係する設定
  // ------------------------------------
  InitialPosition();        // 初期位置

  for (int i = 0 ; i < 5 ; i++) {
    // 足の角度を補正するため5秒とめる
    // 13番ピンのLEDをチカチカさせる
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
  }

}

// ======================
// loop()関数は繰り返し実行されます。
// ======================
void loop() {
  int i = 0;
  int ret = 0;  // 復帰値

  // ------------------------------------
  // 赤外線リモコンデータの受信
  // ------------------------------------
  ret = ir.receive(ir_dat);
  if (ret > 0) {
    if (memcmp(ir_dat, motiondata[0], IR_DATA_MAX_BYTE_SIZE) == 0)return;                 // 何もしない

    digitalWrite(13, HIGH);

#ifdef DEBUG
    //受信した赤外線リモコンデータを16進数で表示
    Serial.print("IR:HEX[");
    for ( i = 0; i < IRControlReceiver::IR_DATA_MAX_BYTE; i++) {
      if ( *(ir_dat + i) <= 0x0F )Serial.print('0');
      Serial.print(*(ir_dat + i), HEX);
    }
    Serial.println("]");
#endif

    // 赤外線センサーがサーボモータ制御信号のノイズをひろい誤動作します
    // サーボモータを動かす時だけサーボモータの設定を毎回行います

    Servo1.attach(SERVO1);       // サーボモータのピン設定
    Servo2.attach(SERVO2);       // サーボモータのピン設定

    if (memcmp(ir_dat, motiondata[1], IR_DATA_MAX_BYTE_SIZE) == 0)front();                // 前にすすむ
    else if (memcmp(ir_dat, motiondata[2], IR_DATA_MAX_BYTE_SIZE) == 0)rightOblique();    // 右斜めにすすむ
    else if (memcmp(ir_dat, motiondata[3], IR_DATA_MAX_BYTE_SIZE) == 0)rightObliqueBack();// 右斜め後ろにすすむ
    else if (memcmp(ir_dat, motiondata[4], IR_DATA_MAX_BYTE_SIZE) == 0)back();            // 後ろにすすむ
    else if (memcmp(ir_dat, motiondata[5], IR_DATA_MAX_BYTE_SIZE) == 0)leftObliqueBack(); // 左斜め後ろにすすむ
    else if (memcmp(ir_dat, motiondata[6], IR_DATA_MAX_BYTE_SIZE) == 0)leftOblique();     // 左斜めにすすむ

    Servo1.detach();             // サーボモータのピン解放
    Servo2.detach();             // サーボモータのピン解放

    digitalWrite(13, LOW);
  }
}

// ====================================
// 領域比較関数
// ====================================
int memcmp(const void *s1, const void *s2, size_t n)
{
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  while (n-- > 0) {
    if (*p1 != *p2)return (*p1 - *p2);
    p1++;
    p2++;
  }
  return (0);
}

// ======================
// 初期位置
// ======================
void InitialPosition() {
  Servo1.attach(SERVO1);       // サーボモータのピン設定
  Servo2.attach(SERVO2);       // サーボモータのピン設定

  Servo1.write(Servo1_val[1]); // 左右に動くサーボモータの制御
  Servo2.write(Servo2_val[1]); // 上下に動くサーボモータの制御

  Servo1.detach();             // サーボモータのピン解放
  Servo2.detach();             // サーボモータのピン解放
}

// ======================
// 真っ直ぐ進む
// ======================
void front() {
#ifdef DEBUG
  Serial.println("front");
#endif
  rightOblique();
  leftOblique();
}

// ======================
// 後ろに進む
// ======================
void back() {
#ifdef DEBUG
  Serial.println("back");
#endif
  rightObliqueBack();
  leftObliqueBack();
}

// ======================
// 右斜めに進む
// ======================
void rightOblique() {
#ifdef DEBUG
  Serial.println("rightOblique");
#endif
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
#ifdef DEBUG
  Serial.println("rightObliqueBack");
#endif
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
#ifdef DEBUG
  Serial.println("leftOblique");
#endif
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
#ifdef DEBUG
  Serial.println("leftObliqueBack");
#endif
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

