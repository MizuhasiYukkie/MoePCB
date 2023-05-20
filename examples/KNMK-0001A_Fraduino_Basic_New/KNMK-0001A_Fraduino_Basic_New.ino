/*!
 * KNMK-0001A Fraduino向けソースコード
 *
 * Copyright (c) 2023 Mizuhasi Yukkie, Onozawa Hiro
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
 * ※USB端子経由で書き込めない場合はICSP端子からAVRISP mkⅡなどを用いて書き込んでください。
 */

// 萌基板モジュール新規実装を利用する
#define USE_NEW_MOE_PCB
#include <MoePCB.h>
#include <Params/Fran.h>

volatile uint8_t PATTERN_MODE = 0;   // 点灯パターン 0-2
volatile uint8_t BRIGHTNESS_IDX = 1; // 輝度レベル 0-3

// 萌基板インスタンス (フラン)
// タッチセンサ、サブLED、メインLEDモジュールを利用
// メインLEDモジュールには調光フィルタに FullSet (全部乗せ) 、色生成モジュールに Breath, Rainbow, Gaming の3種を利用
MoePCB_Core<
  MoePCB_Params_Fran,
  MoePCB_TouchSensorModule,
  MoePCB_SubLEDModule,
  MoePCB_LED_Module<
    MoePCB_LEDColorFilter_FullSet,
    MoePCB_LEDColorCalculator_Breath,
    MoePCB_LEDColorCalculator_Rainbow,
    MoePCB_LEDColorCalculator_Gaming
  >
> Fran;

// 表面のLEDの並び
constexpr MoePCB_Params_Fran::LED::led_index_t led_idx_list[] = {
  MoePCB_Params_Fran::LED::Placement::OuterLeft,
  MoePCB_Params_Fran::LED::Placement::Left,
  MoePCB_Params_Fran::LED::Placement::InnerLeft,
  MoePCB_Params_Fran::LED::Placement::InnerRight,
  MoePCB_Params_Fran::LED::Placement::Right,
  MoePCB_Params_Fran::LED::Placement::OuterRight
};

// LEDモジュールの色生成処理を Rainbow に設定
void set_led_rainbow(MoePCB_LEDColorCalculator_Rainbow::Mode::type_t mode) {
  auto& led = Fran.getMainLED().getColorCalculator();

  // Heart が breath なら 他はすべて rainbow
  if (led.get_interface_breath(MoePCB_Params_Fran::LED::Placement::Heart) != nullptr) {
    // 輝度レベルとモードを変更
    for (MoePCB_Params_Fran::LED::led_index_t led_idx = 0; led_idx < MoePCB_Params_Fran::LED::Length; ++led_idx) {
      auto const pI = led.get_interface_rainbow(led_idx);
      if (pI != nullptr) {
        pI->set_mode(mode);
        pI->set_brightness_level(BRIGHTNESS_IDX);
      }
    }
  } else {
    // 色生成処理を breath, icy に変更
    led.set_mode_breath(MoePCB_Params_Fran::LED::Placement::Heart);

    // レインボーモード,点灯パターンA（自動でキラキラ）（位相差60度）
    for (uint8_t i = 0; i < sizeof(led_idx_list)/sizeof(led_idx_list[0]); ++i) {
      const auto led_idx = led_idx_list[i];
      led.set_mode_rainbow(led_idx, -60.0f * i, 1.2f, mode, BRIGHTNESS_IDX);
    }
  }
}

// LEDモジュールの色生成処理を gaming に設定
void set_led_gaming() {
  auto& led = Fran.getMainLED().getColorCalculator();

  // Heart が gaming なら 他もすべて gaming
  if (led.get_interface_gaming(MoePCB_Params_Fran::LED::Placement::Heart) != nullptr) {
    // 輝度レベルのみ変更
    for (MoePCB_Params_Fran::LED::led_index_t led_idx = 0; led_idx < MoePCB_Params_Fran::LED::Length; ++led_idx) {
      auto const pI = led.get_interface_gaming(led_idx);
      if (pI != nullptr) {
        pI->set_brightness_level(BRIGHTNESS_IDX);
      }
    }
  } else {
    // 色生成処理を gaming に変更（位相差約20度）
    // 元実装では step = 6 だが update 間隔が早まったため 5 に落とす (それでもやや早い)
    led.set_mode_gaming(MoePCB_Params_Fran::LED::Placement::Heart, 0, 5, BRIGHTNESS_IDX);

    for (uint8_t i = 0; i < sizeof(led_idx_list)/sizeof(led_idx_list[0]); ++i) {
      const auto led_idx = led_idx_list[i];
      const uint8_t shift = ((uint16_t)256*i)/(360/20);
      led.set_mode_gaming(led_idx, -shift, 5, BRIGHTNESS_IDX);
    }
  }
}

// LED調光モード反映
void apply() {
  // LEDモジュールの色生成処理を点灯モードに応じ切り替え
  switch(PATTERN_MODE) {
  case 0: set_led_rainbow(MoePCB_LEDColorCalculator_Rainbow::Mode::NaturalAndTwinkle); break;
  case 1: set_led_rainbow(MoePCB_LEDColorCalculator_Rainbow::Mode::DigitalAndTwinkle); break;
  case 2: set_led_gaming(); break;
  }

  // LEDモジュールの色生成処理のベース輝度は最大値で固定
  Fran.getMainLED().getColorCalculator().setBrightnessAll(255);

  // フィルタの直前の色情報を初期化
  Fran.getMainLED().getColorFilter().resetLastColor();

  // LEDモジュールの色生成処理、フィルタ処理の更新を有効化
  Fran.getMainLED().getColorCalculator().setEnabledAll(true);
  Fran.getMainLED().getColorFilter().setEnabled(true);

}

// 了解コール
void acknowledge(int ms) {
  // サブLED点灯
  Fran.getSubLED().set(true);
  Fran.getSubLED().show();

  // LEDモジュールの色生成処理、フィルタ処理の更新を止める
  Fran.getMainLED().getColorCalculator().setEnabledAll(false);
  Fran.getMainLED().getColorFilter().setEnabled(false);

  // すべてのLEDを消灯
  Fran.getMainLED().getRawLED().clear();
  Fran.getMainLED().getRawLED().show();

  // 少し待機 (割り込みがかかってもLEDモジュールを無効化しているため消灯したままになる)
  delay(ms);

  // LEDモジュールの色生成処理、フィルタ処理の更新を再開
  Fran.getMainLED().getColorCalculator().setEnabledAll(true);
  Fran.getMainLED().getColorFilter().setEnabled(true);

  // サブLED消灯
  Fran.getSubLED().set(false);
  Fran.getSubLED().show();
}



// タイマー割り込みにより一定間隔で実行される処理
// 割り込み間隔は要確認
void MoePCB_Task() {
  Fran.interrupt(); // 萌基板割り込み実行
}

// 起動後に1回呼ばれるArduinoの関数
void setup() {
//  Serial.begin(9600);//シリアル通信を使いたいとき

  // 萌基板初期化
  Fran.begin();

  // LED ベース光量 最大値に設定
  Fran.getMainLED().getRawLED().setBrightness(255);

  apply();
}

// setup後繰り返し呼ばれるArduinoの関数
void loop() {
  // 萌基板フレーム更新処理
  Fran.update();

// TODO: ThermometerModule を利用したサンプルを書く
/* CPU内蔵温度感知機能を使う場合は校正した方がいいです（個体差大）
    static uint8_t cnt;
    if(30<cnt){
      cnt=0;
      Serial.print(Fran.cputemp_raw()); Serial.print(" / ");//CPU温度ADC値を見たい時
      Serial.print(Fran.cputemp(26.0, 305.0));//305.0のところは各自の校正値を入れる：校正時の周辺の実測温度、その時のCPU温度ADC値（上段）    
      Serial.println(" C");
    }
    cnt++;
    //50度超えると暑いよ〜
    if(50 < Fran.cputemp(26.0, 305.0) )Fran.heat(true);//cputempに校正値を与えると気温が返ってくる
    else Fran.heat(false);

    //10度下回ると寒いよ〜
    if(Fran.cputemp(26.0, 305.0) < 10 )Fran.cold(true);//cputempに校正値を与えると気温が返ってくる
    else Fran.cold(false);
*/    

  // タッチセンサインスタンスへの参照
  auto& touch_module = Fran.getTouchSensor();
  auto& hair_sensor = touch_module.getRawTouchSensor(MoePCB_Params_Fran::TouchSensor::Placement::Hair);
  auto& chest_sensor = touch_module.getRawTouchSensor(MoePCB_Params_Fran::TouchSensor::Placement::Chest);
  auto& skirt_sensor = touch_module.getRawTouchSensor(MoePCB_Params_Fran::TouchSensor::Placement::Skirt);

  // 髪タッチ検知
  if (hair_sensor.isOnTouch()) {
    // 輝度レベルインクリメント
    BRIGHTNESS_IDX = (BRIGHTNESS_IDX + 1) % 4;

    acknowledge(80); 
    apply();
  }

  // 胸タッチ検出
  // 長押しと判定した場合怒りモードを有効にする
  Fran.getMainLED().getColorFilter().set_state_angry(chest_sensor.isPressing());

  //スカートタッチ検出
  if (skirt_sensor.isOnTouch()) {
    if (PATTERN_MODE == 0)     PATTERN_MODE = 1;
    else if(PATTERN_MODE == 1) PATTERN_MODE = 2;
    else                       PATTERN_MODE = 0;

    acknowledge(80); 
    apply();
  }
  
  // 60FPSを目指したdelay呼び出し。loop関数内の処理でdelay呼び出しが含まれているものもあり、
  // またタイマ割り込みの処理を行っている間loop関数の処理は進行しないため、実際にはloop関数は
  // 目標より低いFPSで進行する。また処理内容ごとに処理時間が変わるため、FPSは一定にならない。
  delay(15);
}
