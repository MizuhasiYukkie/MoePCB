/*!
 * KNMK-0006A Mariduino向けソースコード
 *
 * Copyright (c) 2023 Mizuhasi Yukkie
 * This software is released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

/*
 * This program requires the folloing libraries.
 * このコードを使用するためには、下記のライブラリをインストールしてください。
 *
 * 【ADCTouch】 https://github.com/martin2250/ADCTouch
 * 【Adafruit_NeoPixel】 https://github.com/adafruit/Adafruit_NeoPixel
 * 【MoePCB】 https://github.com/MizuhasiYukkie/MoePCB
 *
 * ツール→ボード："Arduino Leonardo"を選択
 * ツール→シリアルポート：「USBで接続済みの萌基板のポート」（ArduinoLeonardoと表示されます）を選択
 * 「マイコンボードに書き込む」を実行するとここに書いてあるプログラムが書き込まれます。
 * ※USB端子経由で書き込めない場合はICSP端子からAVRISP
 * mkⅡなどを用いて書き込んでください。
 */

#include <MoePCB.h>
#include <ADCTouch.h>

// タッチセンシングのポート設定（基板により違うので注意）
#define MUNE A0
#define KAMI A1
#define APRON A3
#define STAR A2   // 右端の星

// IRプロトコル解析-NEC
#define DECODE_NEC

// タッチセンシングの初期値
int kami_offset;
int mune_offset;
int apron_offset;
int star_offset;

// タッチセンシングフラグ
bool kami_flag;
bool mune_flag;
bool apron_flag;
bool star_flag;

// 他の誰かがIRを発していることを検出する 保持タイマーも兼ねてる
int ir_detectflag = 0;
// 誰かが怒りモード担っているのを検出する　保持タイマーも兼ねてる
int ir_angry_detectflag = 0;

// マスパ状態のゲージ
int maspa_gauge = 0;

uint8_t PATTERN_MODE = 0;  // 点灯パターン 0-2

#define NO_LED_FEEDBACK_CODE  // IR saves 418 bytes program space
#include <IRremote.hpp>

MoePCB Marisa(16);  // インスタンス生成（RGBLED数）

// タイマーにより自動で実行
void MoePCB_Task() {
  switch (PATTERN_MODE) {
      // 点灯パターン１
    case 0:
      // マスパ状態ではない
      if (0 == maspa_gauge) {
        pattern1();
      } else if (maspa_gauge < 30) {
        // マスパチャージ
        Marisa.masterspark_charge();
      } else {
        // マスパ
        maspa();
      }
      break;

      // 点灯パターン２
    case 1:
      // マスパ状態ではない
      if (0 == maspa_gauge) {
        pattern2();
      } else if (maspa_gauge < 30) {
        // マスパチャージ
        Marisa.masterspark_charge();
      } else {
        // マスパ
        maspa();
      }
      break;

      // 点灯パターン３：ゲーミングモード
    case 2:
      Marisa.gaming(LED1, 0);  // LED0:裏面RGBLED
      Marisa.gaming(LED8, 0);  // LED4:ゲーミングモード（位相差）
      Marisa.gaming(LED7, 20);
      Marisa.gaming(LED6, 40);
      Marisa.gaming(LED5, 60);
      Marisa.gaming(LED4, 80);
      Marisa.gaming(LED3, 100);
      Marisa.gaming(LED2, 120);
      Marisa.gaming(LED14, 140);
      Marisa.gaming(LED12, 160);
      Marisa.gaming(LED13, 180);
      Marisa.gaming(LED11, 200);
      Marisa.gaming(LED9, 220);
      Marisa.gaming(LED10, 240);
      break;
  }

  while (!IrReceiver.isIdle())
    ;  // IR受信状態がアイドル状態になるまで待つ　NeoPixel処理は割り込みハンドラを阻害するため
  Marisa.update();  // 計算＆LEDに送信
}

void setup() {
  Serial.begin(9600);  // シリアル通信を使いたいとき
  //  while(!Serial);

  Marisa.begin();  // 萌基板初期化 タイマー無効で開始

  IrSender.begin(3);    // IRremoteはD3から出力する
  IrReceiver.begin(2);  // D2で受信

  // タッチセンシングの初期値を登録
  kami_offset = ADCTouch.read(KAMI, 500);  // 読むポート, サンプリング数
  mune_offset = ADCTouch.read(MUNE, 500);
  apron_offset = ADCTouch.read(APRON, 500);
  star_offset = ADCTouch.read(STAR, 500);
}

void loop() {
  int kami_sense = ADCTouch.read(KAMI, 10) - kami_offset;  // 髪の毛タッチ
  int mune_sense = ADCTouch.read(MUNE, 10) - mune_offset;  // お胸タッチ
  int apron_sense = ADCTouch.read(APRON, 10) - apron_offset;  // エプロンタッチ
  // 試作では右から二番目の星↑
  int star_sense = ADCTouch.read(STAR, 10) - star_offset;  // 右端の星タッチ

  // 髪タッチ検知
  if (50 < kami_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (kami_flag == 0) Marisa.brightness_add();
    kami_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else
    kami_flag = 0;  // 離したのでタッチフラグクリア

  // 胸タッチ検出で怒る
  if (50 < mune_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (mune_flag == 0) {
      // 胸タッチ検出IRをワンショット送る
      IR_send(ANGRY, Marisa.Get_FuryGauge());
    }
    Marisa.angry(true);
    mune_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else {
    // あるいは誰かがIR越しに怒っていることを検出したら怒る
    if (ir_angry_detectflag != 0)
      Marisa.angry(true);
    else
      // 胸タッチしていない、かつIRでも誰でも怒っていなければ怒りをおさめる
      Marisa.angry(false);
    mune_flag = 0;  // 離したのでタッチフラグクリア
  }

  // スカートタッチ検出
  if (50 < apron_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (apron_flag == 0) {
      Marisa.acknowledge();  // 了解コール
      if (PATTERN_MODE == 0)
        PATTERN_MODE = 1;
      else if (PATTERN_MODE == 1)
        PATTERN_MODE = 2;
      else
        PATTERN_MODE = 0;
    }
    apron_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else {
    apron_flag = 0;  // 離したのでタッチフラグクリア
  }

  // 星タッチでマスタースパークチャージ
  if (50 < star_sense) {
    if (maspa_gauge < 35) maspa_gauge += 1;
    // フラグがまだ立っていなければ以下を実行
    if (star_flag == 0) {
      // タッチ検出でマスパゲージ（チャージ開始のお知らせ）を[ワンショット送る
      IR_send(MASTER_SPARK, maspa_gauge);
    }
    star_flag = 1;  // フラグを立てることで立ち上がり時のみ実行

    // マスパがチャージから発射になった辺りでももう一度送信
    if (maspa_gauge == 30) IR_send(MASTER_SPARK, maspa_gauge);

    // マスパの周期に合わせてIR発信→重ければ削除
    if ((Marisa.Get_general_cnt() % 20) == 0)
      IR_send(PATTERN_MODE, Marisa.Get_general_cnt());

  } else {
    if (0 < maspa_gauge) maspa_gauge -= 1;
    star_flag = 0;  // 離したのでタッチフラグクリア
  }

  // IR関係
  // 状態送信
  static int IR_cnt = 0;
  if (100 + random(0, 5) < IR_cnt) {
    IR_cnt = 0;

    if (50 < star_sense) {
      IR_send(MASTER_SPARK, maspa_gauge);  // 胸タッチ検出
    } else if (50 < mune_sense) {
      IR_send(ANGRY, Marisa.Get_FuryGauge());  // 胸タッチ検出
    } else {
      IR_send(PATTERN_MODE, Marisa.Get_general_cnt());
    }
  }
  IR_cnt++;

  // 誰かがいればIRフラグが経つ　しばらく見つからなければフラグは減っていきゼロへ
  if (0 < ir_detectflag) ir_detectflag--;

  // 誰かが怒っていればいればIR怒りフラグが経つ　しばらく見つからなければフラグは減っていきゼロへ
  if (0 < ir_angry_detectflag) ir_angry_detectflag--;

  MoePCB_Task();  // 萌基板タスク

  // IR受信データがあればデコード関数へ
  if (IrReceiver.decode()) IR_decode();

  // そこそこ正確に20ms待機する
  // 前回から20ms経つまで待機。超えていたらすぐ次へ
  static unsigned long past_t;
  while (millis() < (past_t + 20))
    ;
  past_t = millis();
}

// 発信　基板IDを間違えないように。
void IR_send(uint8_t data1, uint8_t data2) {
  IrSender.sendNEC((MARISA << 8) + data1, data2, 0);
  // 基板ID(8bit) +コマンド(8bit) +データ(8bit) +リピート回数
}

// 受信
void IR_decode() {
  // 受信データ 基板ID、コマンド（orモード）、データ　8bitｘ３
  uint8_t ID;
  uint8_t command;
  uint8_t data;

  if (IrReceiver.decodedIRData.protocol == NEC) {
    ID = highByte(IrReceiver.decodedIRData.address);
    command = lowByte(IrReceiver.decodedIRData.address);
    data = IrReceiver.decodedIRData.command;

    // 自分以外の誰かが居ることを検出
    if (ID != MARISA) {
      // 未開発だけどID64までとりあえず認識する
      if ((0 < ID) && (ID < 64)) {
        ir_detectflag = 50 * 4;  // 4秒ほど見つからなければ自動でゼロへ

        // 自分以外の誰かが怒りモードになってる
        if (command == ANGRY) {
          ir_angry_detectflag = 50 * 3;  // 3秒ほど維持する
        }
      }
    }
  }
  IrReceiver.resume();  // Enable receiving of the next value
}

void pattern1(void) {
  Marisa.breath(LED1);  // LED0:裏面RGBLED
  // 八卦炉
  if (ir_detectflag) {  // 誰かを見つけたとき
    Marisa.moonbreath(LED7);
  } else {
    Marisa.marisa_twinkle(LED7, 3);
  }
  Marisa.marisa_twinkle(LED8, 0);
  Marisa.marisa_twinkle(LED6, 6);
  Marisa.marisa_twinkle(LED5, 7);
  Marisa.marisa_twinkle(LED4, 9);
  Marisa.marisa_twinkle(LED3, 12);
  Marisa.marisa_twinkle(LED2, 16);
  Marisa.marisa_twinkle(LED14, 18);
  Marisa.marisa_twinkle(LED12, 21);
  Marisa.marisa_twinkle(LED13, 23);
  Marisa.marisa_twinkle(LED11, 25);
  Marisa.marisa_twinkle(LED9, 27);
  Marisa.marisa_twinkle(LED10, 29);

  if (random(0, 10) == 0) {
    Marisa.marisa_twinkle(LED4, 9 + 60);
    Marisa.marisa_twinkle(LED3, 12 + 60);
    Marisa.marisa_twinkle(LED2, 16 + 60);
    Marisa.marisa_twinkle(LED14, 18 + 60);
    Marisa.marisa_twinkle(LED12, 21 + 60);
    Marisa.marisa_twinkle(LED13, 23 + 60);
    Marisa.marisa_twinkle(LED11, 25 + 60);
    Marisa.marisa_twinkle(LED9, 27 + 60);
    Marisa.marisa_twinkle(LED10, 29 + 60);

    Marisa.marisa_twinkle(LED7, 0 + 110);
    Marisa.marisa_twinkle(LED6, 1 + 110);
    Marisa.marisa_twinkle(LED8, 6 + 110);
    Marisa.marisa_twinkle(LED5, 4 + 110);
    Marisa.marisa_twinkle(LED4, 5 + 110);
    Marisa.marisa_twinkle(LED3, 7 + 110);
  }
}

void pattern2(void) {
  Marisa.breath(LED1);  // LED0:裏面RGBLED
  // 八卦炉
  if (ir_detectflag) {  // 誰かを見つけたとき
    Marisa.moonbreath(LED7);
  } else {
    Marisa.marisa_twinkle(LED7, 3);
  }
  Marisa.marisa_twinkle(LED8, 0);
  Marisa.marisa_twinkle(LED6, 6);
  Marisa.marisa_twinkle(LED5, 7);
  Marisa.marisa_twinkle(LED13, 23);
  Marisa.marisa_twinkle(LED9, 27);

  Marisa.rainbow(LED4, 0, A);
  Marisa.rainbow(LED3, 40, A);
  Marisa.rainbow(LED2, 80, A);
  Marisa.rainbow(LED14, 120, A);
  Marisa.rainbow(LED12, 160, A);
  Marisa.rainbow(LED11, 200, A);
  Marisa.rainbow(LED10, 240, A);

  if (random(0, 10) == 0) {
    Marisa.marisa_twinkle(LED4, 9 + 60);
    Marisa.marisa_twinkle(LED3, 12 + 60);
    Marisa.marisa_twinkle(LED2, 16 + 60);
    Marisa.marisa_twinkle(LED14, 18 + 60);
    Marisa.marisa_twinkle(LED12, 21 + 60);
    Marisa.marisa_twinkle(LED13, 23 + 60);
    Marisa.marisa_twinkle(LED11, 25 + 60);
    Marisa.marisa_twinkle(LED9, 27 + 60);
    Marisa.marisa_twinkle(LED10, 29 + 60);

    Marisa.marisa_twinkle(LED7, 0 + 110);
    Marisa.marisa_twinkle(LED6, 1 + 110);
    Marisa.marisa_twinkle(LED8, 6 + 110);
    Marisa.marisa_twinkle(LED5, 4 + 110);
    Marisa.marisa_twinkle(LED4, 5 + 110);
    Marisa.marisa_twinkle(LED3, 7 + 110);
  }
}

void maspa() {
  Marisa.masterspark(LED1, 0);        // LED0:裏面RGBLED
  Marisa.masterspark(LED7, 1, true);  // このLEDだけ超輝度にする
  Marisa.masterspark(LED6, 2);
  Marisa.masterspark(LED8, 3);
  Marisa.masterspark(LED5, 3);
  Marisa.masterspark(LED4, 3);
  Marisa.masterspark(LED3, 4);
  Marisa.masterspark(LED2, 5);
  Marisa.masterspark(LED14, 6);
  Marisa.masterspark(LED12, 8);
  Marisa.masterspark(LED13, 7);
  Marisa.masterspark(LED11, 9);
  Marisa.masterspark(LED9, 7);
  Marisa.masterspark(LED10, 9);
}
