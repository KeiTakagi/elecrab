/*

    @file elecrab_demo.ino
    @brief えれくらぶ動作サンプル
        赤外線リモコン受信モジュール(Vout)に接続ピン番号・・・2番
        赤外線リモコン受信モジュール(GND )に接続ピン番号・・・3番
        赤外線リモコン受信モジュール(VCC )に接続ピン番号・・・4番
        左右に動くサーボモータのPIN番号(上側のサーボモータ)・・・5番
        上下に動くサーボモータのPIN番号(下側のサーボモータ)・・・6番
        シリアル（シリアルモニタなど）からコマンドを送信する際はLE、LFを付けて下さい
    @author Kei Takagi
    @date 2016.2.19

    Copyright (c) 2016 Kei Takagi
    Released under the MIT license
    http://opensource.org/licenses/mit-license.php

*/

#include <EEPROM.h>
#include <IRControlReceiver.h>
#include <Servo.h>

// 左右に動くサーボモータのPIN番号(上側のサーボモータ)
#define SERVO1 5
// 上下に動くサーボモータのPIN番号(下側のサーボモータ)
#define SERVO2 6

int SPEED = 200;                                                     // 歩く速度のウェイト(μs)(数値が小さいほど早く歩きます)

IRControlReceiver ir(2);                                          // 赤外線リモコン受信モジュール(Vout)に接続するArduino側のピン番号は2番
Servo Servo1;                                                       // 左右に動く足の角度(上側のサーボモータ)
Servo Servo2;                                                       // 上下に動く足の角度(下側のサーボモータ)

int Servo1_val[3] = {40,  90, 140};                                  // 左右に動く足の角度(上側のサーボモータ)
int Servo2_val[3] = {60,  90, 120};                                  // 上下に動く足の角度(下側のサーボモータ)

unsigned char ir_dat[IR_DATA_MAX_BYTE_SIZE];                         // 赤外線リモコンデータの受信領域
unsigned char ir_zero[IR_DATA_MAX_BYTE_SIZE];                        // 赤外線リモコンデータの受信領域

// えれくらぶ動作設定
enum Motion {
  FRONT,              // 前
  RIGHTOBLIQUE,       // 右斜め
  RIGHTOBLIQUEBACK,   // 右斜め後ろ
  BACK,               // 後ろ
  LEFTOBLIQUEBACK,    // 左斜め後ろ
  LEFTOBLIQUE,        // 左斜め
  STOP,               // 停止
  MotionCOUNT         // enumオブジェクトの個数
};

// えれくらぶ動作設定用メッセージ
char msg[][ 24 ] =
{
  "[2]:FRONT           ", // 前
  "[3]:RIGHTOBLIQUE    ", // 右斜め
  "[9]:RIGHTOBLIQUEBACK", // 右斜め後ろ
  "[8]:BACK            ", // 後ろ
  "[7]:LEFTOBLIQUEBACK ", // 左斜め後ろ
  "[1]:LEFTOBLIQUE     ", // 左斜め
  "STOP                "  // STOP
};

// 動作用 EPROM データ領域
struct MotionData {
  int speed;                                                         // 歩く速度
  unsigned char ir_dat[MotionCOUNT][IR_DATA_MAX_BYTE_SIZE];          // 動作用赤外線リモコンデータ領域
  int srv1[3];                                                       // 左右に動く足の角度(上側のサーボモータ)
  int srv2[3];                                                       // 上下に動く足の角度(下側のサーボモータ)
} motiondata;

// ======================
// setup()関数はスケッチがスタートしたときに1度だけ呼び出されます
// ======================
void setup() {
  int  command;           // 設定内容
  int  i = 0;             // loop用

  // ------------------------------------
  // パソコンとシリアル通信するための設定
  // ------------------------------------
  Serial.begin(9600) ;

  // ------------------------------------
  // 赤外線リモコン受信モジュール設定
  // ------------------------------------
  pinMode(3, OUTPUT);        // 赤外線リモコン受信モジュール PIN3 GND
  digitalWrite(3, LOW);
  pinMode(4, OUTPUT);        // 赤外線リモコン受信モジュール PIN4 VCC
  digitalWrite(4, HIGH);
  pinMode(13, OUTPUT);       // PIN13 LED
  for (i = 0; i < IR_DATA_MAX_BYTE_SIZE; i++)ir_zero[i] = 0x00; //領域クリア

  // ------------------------------------
  // えれくらぶの動作に関係する設定
  // ------------------------------------
  Servo1.attach(SERVO1);    // サーボモータの初期設定
  Servo2.attach(SERVO2);    // サーボモータの初期設定

  // ------------------------------------
  // EPROMからの設定読込
  // ------------------------------------
  EEPROM.get( 0, motiondata );            // EPROMから設定値を読込
  if (motiondata.speed < 100 || 900 < motiondata.speed || motiondata.srv1[0] < 30 || 150 < motiondata.srv1[3] ||  motiondata.srv2[0] < 30 || 150 < motiondata.srv2[3] ) {
    // EPROMの値がおかしい場合領域EPROMの値は使用しない
    Serial.print(F("The value of EPROM does not use.\r\n"));
    for ( command = 0; command < MotionCOUNT - 1 ; command++ ) {
      for ( i = 0; i < IR_DATA_MAX_BYTE_SIZE; i++) {
        *(motiondata.ir_dat[command] + i) = 0x00;
      }
    }
    motiondata.speed = SPEED;
    motiondata.srv1[0] = Servo1_val[0];
    motiondata.srv1[1] = Servo1_val[1];
    motiondata.srv1[2] = Servo1_val[2];

    motiondata.srv2[0] = Servo2_val[0];
    motiondata.srv2[1] = Servo2_val[1];
    motiondata.srv2[2] = Servo2_val[2];
  }

  // ------------------------------------
  // 歩く速度・左右に動く足の角度の設定
  // ------------------------------------
  SPEED = motiondata.speed;
  Servo1_val[0] = motiondata.srv1[0];
  Servo1_val[1] = motiondata.srv1[1];
  Servo1_val[2] = motiondata.srv1[2];

  Servo2_val[0] = motiondata.srv2[0];
  Servo2_val[1] = motiondata.srv2[1];
  Servo2_val[2] = motiondata.srv2[2];

  // ------------------------------------
  // 初期位置
  // ------------------------------------
  InitialPosition();

  // ------------------------------------
  // リモコン設定
  // ------------------------------------
  iesetting();
}

// ======================
// loop()関数は繰り返し実行されます。
// ======================
void loop() {
  int ret = 0;  // 復帰値
  int i;
  // 赤外線リモコンデータの受信
  ret = ir.receive(ir_dat);
  if (ret > 0) {
    if (memcmp( ir_zero, ir_dat, IR_DATA_MAX_BYTE_SIZE ) == 0 ) {
      // NO DATA
    }
    else if (memcmp(ir_dat, motiondata.ir_dat[0], IR_DATA_MAX_BYTE_SIZE) == 0) {
      front();              // 前にすすむ
    }
    else if (memcmp(ir_dat, motiondata.ir_dat[1], IR_DATA_MAX_BYTE_SIZE) == 0) {
      rightOblique();       // 右斜めにすすむ
    }
    else if (memcmp(ir_dat, motiondata.ir_dat[2], IR_DATA_MAX_BYTE_SIZE) == 0) {
      rightObliqueBack();   // 右斜め後ろにすすむ
    }
    else if (memcmp(ir_dat, motiondata.ir_dat[3], IR_DATA_MAX_BYTE_SIZE) == 0) {
      back();               // 後ろにすすむ
    }
    else if (memcmp(ir_dat, motiondata.ir_dat[4], IR_DATA_MAX_BYTE_SIZE) == 0) {
      leftObliqueBack();    // 左斜め後ろにすすむ
    }
    else if (memcmp(ir_dat, motiondata.ir_dat[5], IR_DATA_MAX_BYTE_SIZE) == 0) {
      leftOblique();        // 左斜めにすすむ
    }
    else InitialPosition(); // 初期位置
  }
}

// ====================================
// リモコン設定
// ====================================
void iesetting() {
  int  slen = 0;                               // パソコンからシリアル通信で受信した文字数
  int  command = -1;                           // 設定内容
  int  ret = 0;                                // 復帰値
  int  i = 0;                                  // loop用
  unsigned long time;                          // 起動時間取得用
  unsigned char ir_dat[IR_DATA_MAX_BYTE_SIZE]; // IRデータ受信用
  char sbuf[34];                               // パソコンからシリアル通信で受信する文字列格納用
  boolean  saveflg;                            // 保存フラグ
  //えれくらぶをパソコンから設定するためのコマンド説明
  Serial.print(F("Set Up The Erecrub TV remote control.\r\n"));
  Serial.print(F("If you want to edit mode, please Enter key within 5 seconds.\r\n"));
  SETTING_DUMP();

  time = millis() + 5000;
  do {
    // ------------------------------------
    // パソコンからシリアルデータを受信
    // ------------------------------------
    if (Serial.available() > 0 ) {      // 受信したデータが存在するならば
      *(sbuf + slen) = Serial.read();   // シリアルポートより1文字読み込む
      if (command == -1 && *sbuf == 0x0A || *sbuf == 0x0D) {
        // ------------------------------------
        // 編集モードに入る
        // ------------------------------------
        LED(500, 1);
        Serial.print(F("<<Edit mode>>\r\n"));
        Serial.print(F("Please press the same key twice to register.\r\n"));
        time = millis() + 600000;       // 終了までの時間を10分延長
        command = FRONT;
        slen = 0;
        Serial.print(msg[command]);
        Serial.print(F("\r\n"));
      }
      else if ( slen < 30 && !( *(sbuf + slen) == 0x0A || *(sbuf + slen) == 0x0D )) {
        slen++;
      } else {
        // ------------------------------------
        // 文字数が30以上 or 末尾文字0x0A 0x0D
        // ------------------------------------
        *(sbuf + slen) = 0x00;
        slen = 0;
        if ( *sbuf == 'Q' || *sbuf == 'q' ) {
          time = 0;
          LED(300, 3);
          Serial.print(F("Not Save.\r\n"));
        }
        else if ( *sbuf == 'S' || *sbuf == 's' ) {
          // 受信文字列をatoi
          motiondata.speed = atoi(sbuf + 1);
          if ( 50 <= motiondata.speed  && motiondata.speed <= 1000 ) {
            LED(300, 1);
            Serial.print(F("Speed(msec):"));
            Serial.print(motiondata.speed);
            Serial.print(F("\r\n"));
          } else {
            LED(300, 3);
            motiondata.speed = 200;
            Serial.print(F("Input err.\r\n"));
          }
        }
        else if ( *sbuf == 'U' || *sbuf == 'u' ) {
          // ------------------------------------
          // 左右に動く足の角度(上側のサーボモータ)
          // ------------------------------------

          // 受信文字列をカンマ区切りにしてatoi
          motiondata.srv1[0] = atoi(strtok(sbuf + 1, ","));
          motiondata.srv1[1] = atoi(strtok(NULL, ","));
          motiondata.srv1[2] = atoi(strtok(NULL, ","));

          if ( 30 < motiondata.srv1[0] && motiondata.srv1[0] < motiondata.srv1[1] && motiondata.srv1[1] < motiondata.srv1[2] &&  motiondata.srv1[2] <= 150) {
            LED(300, 1);
            Serial.print(F("Servo1:"));
            Serial.print(motiondata.srv1[0]);
            Serial.print(F(","));
            Serial.print(motiondata.srv1[1]);
            Serial.print(F(","));
            Serial.print(motiondata.srv1[2]);
            Serial.print(F("\r\n"));
            Servo1.write(motiondata.srv1[1]); // 左右に動くサーボモータの制御
          } else {
            LED(300, 3);
            motiondata.srv1[0] = Servo1_val[0];
            motiondata.srv1[1] = Servo1_val[1];
            motiondata.srv1[2] = Servo1_val[2];
            Serial.print(F("Input err.\r\n"));
          }
        }
        else if ( *sbuf == 'D' || *sbuf == 'd' ) {
          // ------------------------------------
          // 上下に動く足の角度(下側のサーボモータ)
          // ------------------------------------

          // 受信文字列をカンマ区切りにしてatoi
          motiondata.srv2[0] = atoi(strtok(sbuf + 1, ","));
          motiondata.srv2[1] = atoi(strtok(NULL, ","));
          motiondata.srv2[2] = atoi(strtok(NULL, ","));

          if ( 40 < motiondata.srv2[0] && motiondata.srv2[0] < motiondata.srv2[1] && motiondata.srv2[1] < motiondata.srv2[2] &&  motiondata.srv2[2] <= 140) {
            LED(300, 1);
            Serial.print(F("Servo2:"));
            Serial.print(motiondata.srv2[0]);
            Serial.print(F(","));
            Serial.print(motiondata.srv2[1]);
            Serial.print(F(","));
            Serial.print(motiondata.srv2[2]);
            Serial.print(F("\r\n"));
            Servo2.write(motiondata.srv2[1]); // 上下に動くサーボモータの制御
          } else {
            LED(300, 3);
            motiondata.srv2[0] = Servo2_val[0];
            motiondata.srv2[1] = Servo2_val[1];
            motiondata.srv2[2] = Servo2_val[2];
            Serial.print(F("Input err.\r\n"));
          }
        }
      }
    }

    // ------------------------------------
    // 赤外線リモコンデータの受信
    // ------------------------------------
    ret = ir.receive( ir_dat );
    if (ret > 0) {
      for ( i = 0; i < IR_DATA_MAX_BYTE_SIZE; i++) {
        if ( *(ir_dat + i) <= 0x0F )Serial.print('0');
        Serial.print( *(ir_dat + i) , HEX);
      }
      Serial.print(F("\r\n"));
      if (memcmp( ir_zero, ir_dat, IR_DATA_MAX_BYTE_SIZE ) == 0 ) {
        // NO DATA
      } else if (memcmp( motiondata.ir_dat[command], ir_dat, IR_DATA_MAX_BYTE_SIZE ) == 0 ) {
        saveflg = true;
        for ( i = 0 ; i < command ; i++) {
          if (memcmp( motiondata.ir_dat[i], ir_dat, IR_DATA_MAX_BYTE_SIZE ) == 0 ) {
            saveflg = false; // 同じデータが既に存在するので保存しない
          }
        }
        if (saveflg == true) {
          // 保存
          command ++;
          LED(500, 1);
          Serial.print(F("OK\r\n"));

          if (STOP <= command ) {
            SETTING_DUMP();
            Serial.print(F("EPROM SAVE.\r\n"));
            LED(500, 1);
            EEPROM.put( 0, motiondata );    // EPROMに設定値を書込
            time = 0;
          } else {
            Serial.print(msg[command]);
            Serial.print(F("\r\n"));
          }
        }
      } else {
        //受信した赤外線リモコンデータを16進数で表示
        for (i = 0; i < IR_DATA_MAX_BYTE_SIZE; i++) *(motiondata.ir_dat[command] + i) = *(ir_dat + i);
      }
    }
  } while ( millis() < time);

  Serial.print(F("Setting end.\r\n"));
}

// ====================================
// LED
// ====================================
void LED(int on, int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(13, HIGH);   // LED on
    delay(on);
    digitalWrite(13, LOW);    // LED off
    delay(100);
  }
}

// ====================================
// 設定表示
// ====================================
void SETTING_DUMP()
{
  int  command;      // 設定内容
  int  i = 0;        // loop用

  Serial.print(F("===============\r\n"));
  for ( command = 0; command < MotionCOUNT - 1 ; command ++) {
    Serial.print(msg[command]);
    Serial.print(F(":"));
    for ( i = 0; i < IRControlReceiver::IR_DATA_MAX_BYTE; i++) {
      if ( *(motiondata.ir_dat[command] + i) <= 0x0F )Serial.print('0');
      Serial.print( *(motiondata.ir_dat[command] + i) , HEX);
    }
    Serial.print(F("\r\n"));
  }
  Serial.print(F("Speed:"));
  Serial.print(motiondata.speed);
  Serial.print(F("\r\n"));
  Serial.print(F("Servo1:"));
  Serial.print(motiondata.srv1[0]);
  Serial.print(F(","));
  Serial.print(motiondata.srv1[1]);
  Serial.print(F(","));
  Serial.print(motiondata.srv1[2]);
  Serial.print(F("\r\n"));
  Serial.print(F("Servo2:"));
  Serial.print(motiondata.srv2[0]);
  Serial.print(F(","));
  Serial.print(motiondata.srv2[1]);
  Serial.print(F(","));
  Serial.print(motiondata.srv2[2]);
  Serial.print(F("\r\n"));
  Serial.print(F("===============\r\n"));
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
  Servo1.write(Servo1_val[1]); // 左右に動くサーボモータの制御
  Servo2.write(Servo2_val[1]); // 上下に動くサーボモータの制御
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
  Servo1.write(motiondata.srv1[servo1Val]); // 左右に動くサーボモータの制御
  Servo2.write(motiondata.srv2[servo2Val]); // 上下に動くサーボモータの制御
  delay(SPEED);
}
