/*!
 * KNMK-0005A Pachoulino向けソースコード
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
#define SKIRT A0
#define MUNE A1
#define KAMI A2
#define STONE A3  // 右端のストーン

// IRプロトコル解析-NEC
#define DECODE_NEC

// タッチセンシングの初期値
int kami_offset;  
int mune_offset;
int skirt_offset;
int stone_offset;

// タッチセンシングフラグ
bool kami_flag;  
bool mune_flag;
bool skirt_flag;
bool stone_flag;

// 他の誰かがIRを発していることを検出する 保持タイマーも兼ねてる
int ir_detectflag = 0;
// 誰かが怒りモード担っているのを検出する　保持タイマーも兼ねてる
int ir_angry_detectflag = 0;
// 魔理沙を検出する　保持タイマーも兼ねてる
int marisa_detectflag = 0;
// マスパを検出する　保持タイマーも兼ねてる
int maspa_detectflag = 0;
// マスパ状態のゲージ
int maspa_gauge = 0;

uint8_t PATTERN_MODE = 0;  // 点灯パターン 0-2

#define NO_LED_FEEDBACK_CODE  // IR saves 418 bytes program space
#include <IRremote.hpp>

  int t1=0;
  int t2=170;
  int t3=340;

MoePCB Pachu(16);  // インスタンス生成（RGBLED数）

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
        Pachu.masterspark_charge();
      } else {
        // マスパ
        maspa();
      }
      break;

      // 点灯パターン２
    case 1:
      pattern2();
      break;

      // 点灯パターン３：ゲーミングモード
    case 2:
      Pachu.gaming(LED1, 0);   // LED0:裏面RGBLED
      Pachu.gaming(LED10, 0);  // LED4:ゲーミングモード（位相差）
      Pachu.gaming(LED11, 20);
      Pachu.gaming(LED12, 40);
      Pachu.gaming(LED13, 60);
      Pachu.gaming(LED14, 80);
      Pachu.gaming(LED2, 100);
      Pachu.gaming(LED4, 120);
      Pachu.gaming(LED3, 120);
      Pachu.gaming(LED5, 140);
      Pachu.gaming(LED6, 160);
      Pachu.gaming(LED7, 180);
      Pachu.gaming(LED8, 200);
      Pachu.gaming(LED9, 220);
      break;
  }

  while (!IrReceiver.isIdle())
    ;  // IR受信状態がアイドル状態になるまで待つ　NeoPixel処理は割り込みハンドラを阻害するため
  Pachu.update();  // 計算＆LEDに送信

  int pin1=9;//5,[9],[10],[11] 5は駄目だった 11だけ周波数が高い
  int pin2=10;
  int pin3=11;
  
  t1++;
  t2+=8;
  t3+=20;
  if (511<t1)t1=0;
  if (511<t2)t2=0;
  if (511<t3)t3=0;
/*
  if (255<t1)analogWrite(pin1,(511-t1));
  else analogWrite(pin1,t1);
  if (255<t2)analogWrite(pin2,(511-t2));
  else analogWrite(pin2,t2);
  if (255<t3)analogWrite(pin3,(511-t3));
  else analogWrite(pin3,t3);
*/
  analogWrite(pin1,255);
  analogWrite(pin2,255);
  analogWrite(pin3,255);
}

void setup() {
  //  Serial.begin(9600);  // シリアル通信を使いたいとき
  //  while(!Serial);

  Pachu.begin();  // 萌基板初期化 タイマー無効で開始

  IrSender.begin(3);    // IRremoteはD3から出力する
  IrReceiver.begin(2);  // D2で受信

  // タッチセンシングの初期値を登録
  kami_offset = ADCTouch.read(KAMI, 500);  // 読むポート, サンプリング数
  mune_offset = ADCTouch.read(MUNE, 500);
  skirt_offset = ADCTouch.read(SKIRT, 500);
  stone_offset = ADCTouch.read(STONE, 500);
}

void loop() {
  /* CPU内蔵温度感知機能を使う場合は校正した方がいいです（個体差大）
      static uint8_t cnt;
      if(30<cnt){
        cnt=0;
        Serial.print(Pachu.cputemp_raw()); Serial.print(" /
     ");//CPU温度ADC値を見たい時 Serial.print(Pachu.cputemp(26.0,
     305.0));//305.0のところは各自の校正値を入れる：校正時の周辺の実測温度、その時のCPU温度ADC値（上段）
        Serial.println(" C");
      }
      cnt++;
      //50度超えると暑いよ〜
      if(50 < Pachu.cputemp(26.0, 305.0)
     )Pachu.heat(true);//cputempに校正値を与えると気温が返ってくる else
     Pachu.heat(false);

      //10度下回ると寒いよ〜
      if(Pachu.cputemp(26.0, 305.0) < 10
     )Pachu.cold(true);//cputempに校正値を与えると気温が返ってくる else
     Pachu.cold(false);
  */

  int kami_sense = ADCTouch.read(KAMI, 10) - kami_offset;  // 髪の毛タッチ
  int mune_sense = ADCTouch.read(MUNE, 10) - mune_offset;  // お胸タッチ
  int skirt_sense = ADCTouch.read(SKIRT, 10) - skirt_offset;  // スカートタッチ
  int stone_sense = ADCTouch.read(STONE, 10) - stone_offset;  // 右の石タッチ

  //  タッチセンシングの値を見たいとき
  //  Serial.println(kami_sense);
  //  Serial.println(mune_sense);
  //  Serial.println(skirt_sense);
  //  Serial.println(stone_sense);

  // 髪タッチ検知
  if (50 < kami_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (kami_flag == 0) Pachu.brightness_add();
    kami_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else
    kami_flag = 0;  // 離したのでタッチフラグクリア

  // 胸タッチ検出で怒る
  if (50 < mune_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (mune_flag == 0) {
      // 胸タッチ検出IRをワンショット送る
      IR_send(ANGRY, Pachu.Get_FuryGauge());
    }
    Pachu.angry(true);
    mune_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else {
    // あるいは誰かがIR越しに怒っていることを検出したら怒る
    if (ir_angry_detectflag != 0)
      Pachu.angry(true);
    else
      // 胸タッチしていない、かつIRでも誰でも怒っていなければ怒りをおさめる
      Pachu.angry(false);
    mune_flag = 0;  // 離したのでタッチフラグクリア
  }

  // スカートタッチ検出
  if (50 < skirt_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (skirt_flag == 0) {
      Pachu.acknowledge();  // 了解コール
      if (PATTERN_MODE == 0)
        PATTERN_MODE = 1;
      else if (PATTERN_MODE == 1)
        PATTERN_MODE = 2;
      else
        PATTERN_MODE = 0;
    }
    skirt_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else {
    skirt_flag = 0;  // 離したのでタッチフラグクリア
  }

  // IR関係
  // 状態送信
  static int IR_cnt = 0;
  if (100 + random(0, 5) < IR_cnt) {
    IR_cnt = 0;
    if (50 < mune_sense) {
      IR_send(ANGRY, Pachu.Get_FuryGauge());  // 胸タッチ検出
    } else {
      IR_send(PATTERN_MODE, Pachu.Get_general_cnt());
    }
  }
  IR_cnt++;

  // 誰かがいればIRフラグが経つ　しばらく見つからなければフラグは減っていきゼロへ
  if (0 < ir_detectflag) ir_detectflag--;

  // 誰かが怒っていればいればIR怒りフラグが経つ　しばらく見つからなければフラグは減っていきゼロへ
  if (0 < ir_angry_detectflag) ir_angry_detectflag--;

  // 魔理沙がいればフラグが経つ　しばらく見つからなければフラグは減っていきゼロへ
  if (0 < marisa_detectflag) marisa_detectflag--;

  // マスパ検出フラグ
  if (0 < maspa_detectflag) maspa_detectflag--;

  // マスパ検出しなくなったらゲージを自動でへらす
  if (maspa_detectflag == 0) {
    if (0 < maspa_gauge) maspa_gauge -= 1;
  }

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
  IrSender.sendNEC((PATCHU << 8) + data1, data2, 0);
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
    /*
        Serial.print("kiban ID= ");
        Serial.print(ID);
        Serial.print(",   mode/command= ");
        Serial.print(command);
        Serial.print(",   data= ");
        Serial.println(data);
    */
    // 自分以外の誰かが居ることを検出
    if (ID != PATCHU) {
      // 未開発だけどID64までとりあえず認識する
      if ((0 < ID) && (ID < 64)) {
        ir_detectflag = 50 * 4;  // 4秒ほど見つからなければ自動でゼロへ

        // 魔理沙を見つけたら内部カウンタをシンクロする（スレーブ）
        if (ID == MARISA) {
          Pachu.Set_general_cnt(data);
          marisa_detectflag = 50 * 4;  // 4秒ほど見つからなければ自動でゼロへ
        }

        // 自分以外の誰かが怒りモードになってる
        if (command == ANGRY) {
          ir_angry_detectflag = 50 * 3;  // 3秒ほど維持する
        }

        // マスパ検出　魔理沙に限定していない
        if (command == MASTER_SPARK) {
          maspa_detectflag = 50 * 3;  // 3秒ほど維持する
          maspa_gauge = data;
        }
      }
    }
  }
  IrReceiver.resume();  // Enable receiving of the next value
}

void pattern1() {  // 魔理沙を認識したら同じ光パターンへ
  // LED1:裏面RGBLED
  Pachu.breath(LED1);

  //  誰かを見つけると帽子の星を黄色くする
  if (ir_detectflag != 0) {
    Pachu.moonbreath(LED9);
  } else {
    Pachu.twinklestar(LED9, A);
  }
  if (marisa_detectflag != 0) {
    Pachu.marisa_twinkle(LED12, 2);
    Pachu.marisa_twinkle(LED13, 4);
    Pachu.marisa_twinkle(LED14, 6);
    Pachu.marisa_twinkle(LED2, 9);
    Pachu.marisa_twinkle(LED3, 13);
    Pachu.marisa_twinkle(LED4, 16);
    Pachu.marisa_twinkle(LED5, 19);
    Pachu.marisa_twinkle(LED6, 24);
    Pachu.marisa_twinkle(LED7, 29);
    Pachu.rainbow(LED8, 0, A);
    Pachu.rainbow(LED11, 50 * 5, A);
    Pachu.rainbow(LED10, 50 * 6, A);
  } else {
    // デフォルト
    Pachu.twinklestar(LED2, A);
    Pachu.twinklestar(LED3, A);
    Pachu.twinklestar(LED6, A);
    Pachu.twinklestar(LED12, A);
    Pachu.twinklestar(LED14, A);
    Pachu.twinklestar(LED15, A);
    Pachu.rainbow(LED13, 50 * 4, C);
    Pachu.rainbow(LED4, 50 * 3, C);
    Pachu.rainbow(LED5, 50 * 2, C);
    Pachu.rainbow(LED7, 50, C);
    Pachu.rainbow(LED8, 0, C);
    Pachu.rainbow(LED11, 50 * 5, C);
    Pachu.rainbow(LED10, 50 * 6, C);
  }
}
void pattern2() {      // 魔理沙を認識したら同じ光パターンへ
  Pachu.breath(LED1);  // LED0:裏面RGBLED

  //  誰かを見つけると帽子の星を黄色くする
  if (ir_detectflag != 0) {
    Pachu.moonbreath(LED9);
  } else {
    Pachu.twinklestar(LED9, A);
  }
  //  if (marisa_detectflag != 0) {
  Pachu.marisa_twinkle(LED12, 2);
  Pachu.marisa_twinkle(LED13, 4);
  Pachu.marisa_twinkle(LED14, 6);
  Pachu.marisa_twinkle(LED2, 9);
  Pachu.marisa_twinkle(LED3, 13);
  Pachu.marisa_twinkle(LED4, 16);
  Pachu.marisa_twinkle(LED5, 19);
  Pachu.marisa_twinkle(LED6, 24);
  Pachu.marisa_twinkle(LED7, 29);
  Pachu.rainbow(LED8, 0, A);
  Pachu.rainbow(LED11, 50 * 5, B);
  Pachu.rainbow(LED10, 50 * 6, B);
  //  } else {
  // デフォルト パターン２では魔理沙検出と同じ光方にする
  /*
      Pachu.twinklestar(LED2, B);
      Pachu.twinklestar(LED3, B);
      Pachu.twinklestar(LED6, B);
      Pachu.twinklestar(LED12, B);
      Pachu.twinklestar(LED14, B);
      Pachu.twinklestar(LED15, B);
      Pachu.rainbow(LED13, 50 * 4, C);
      Pachu.rainbow(LED4, 50 * 3, C);
      Pachu.rainbow(LED5, 50 * 2, C);
      Pachu.rainbow(LED7, 50, C);
      Pachu.rainbow(LED8, 0, C);
      Pachu.rainbow(LED11, 50 * 5, C);
      Pachu.rainbow(LED10, 50 * 6, C);
    }
  */
}
void maspa() {
  Pachu.masterspark(LED1, 0);   // LED0:裏面RGBLED
  Pachu.masterspark(LED11, 1);  // このLEDだけ超輝度にする
  Pachu.masterspark(LED12, 2);
  Pachu.masterspark(LED10, 3);
  Pachu.masterspark(LED13, 4);
  Pachu.masterspark(LED14, 4);
  Pachu.masterspark(LED2, 5);
  Pachu.masterspark(LED4, 6);
  Pachu.masterspark(LED9, 6);
  Pachu.masterspark(LED3, 7);
  Pachu.masterspark(LED5, 8);
  Pachu.masterspark(LED8, 8);
  Pachu.masterspark(LED7, 9);
  Pachu.masterspark(LED6, 10);
}
