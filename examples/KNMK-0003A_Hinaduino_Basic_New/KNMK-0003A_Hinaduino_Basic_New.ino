/*!
 * KNMK-0003A Hinaduino向けソースコード
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
#include <Params/Hina.h>

volatile uint8_t PATTERN_MODE = 0;   // 点灯パターン 0-2
volatile uint8_t PATTERN_SUBMODE = 0;   // 点灯サブパターン 0-1
volatile uint8_t BRIGHTNESS_IDX = 1; // 輝度レベル 0-3

// 萌基板インスタンス (チルノ)
// タッチセンサ、サブLED、メインLEDモジュールを利用
// メインLEDモジュールには調光フィルタに FullSet (全部乗せ) 、色生成モジュールに Breath, Autumn, Rainbow, Gaming の4種を利用
MoePCB_Core<
  MoePCB_Params_Hina,
  MoePCB_TouchSensorModule,
  MoePCB_SubLEDModule,
  MoePCB_LED_Module<
    MoePCB_LEDColorFilter_FullSet,
    MoePCB_LEDColorCalculator_Breath,
    MoePCB_LEDColorCalculator_Autumn,
    MoePCB_LEDColorCalculator_Rainbow,
    MoePCB_LEDColorCalculator_Gaming
  >
> Hina;

// 表面のLEDの並び
constexpr MoePCB_Params_Hina::LED::led_index_t led_idx_list[] = {
  MoePCB_Params_Hina::LED::Placement::Head,
  MoePCB_Params_Hina::LED::Placement::UpperRight,
  MoePCB_Params_Hina::LED::Placement::Right,
  MoePCB_Params_Hina::LED::Placement::LowerRight,
  MoePCB_Params_Hina::LED::Placement::InnerRight,
  MoePCB_Params_Hina::LED::Placement::InnerLeft,
  MoePCB_Params_Hina::LED::Placement::LowerLeft,
  MoePCB_Params_Hina::LED::Placement::Left,
  MoePCB_Params_Hina::LED::Placement::UpperLeft
};

// LEDモジュールの色生成処理を Autumn に設定
void set_led_autumn(int base, bool onlyChangeBrightness) {
  auto& led = Hina.getMainLED().getColorCalculator();

  // UpperLeft が autumn なら Heart は breath, 他はすべて autumn
  if (onlyChangeBrightness && led.get_interface_autumn(MoePCB_Params_Hina::LED::Placement::UpperLeft) != nullptr) {
    // 輝度レベルとモードを変更
    for (MoePCB_Params_Hina::LED::led_index_t led_idx = 0; led_idx < MoePCB_Params_Hina::LED::Length; ++led_idx) {
      auto const pI = led.get_interface_autumn(led_idx);
      if (pI != nullptr) {
        pI->set_brightness_level(BRIGHTNESS_IDX);
      }
    }
  } else {
    // 色生成処理を breath, autumn に変更
    led.set_mode_breath(MoePCB_Params_Hina::LED::Placement::Heart);

    for (uint8_t i = 0; i < sizeof(led_idx_list)/sizeof(led_idx_list[0]); ++i) {
      const auto led_idx = led_idx_list[i];
      led.set_mode_autumn(led_idx, -base * i, 0.9375f, BRIGHTNESS_IDX);
    }
  }
}

// LEDモジュールの色生成処理を rainbow に設定
void set_led_rainbow(float base) {
  auto& led = Hina.getMainLED().getColorCalculator();

  // UpperLeft が rainbow なら Heart は breath, 他はすべて rainbow
  if (led.get_interface_rainbow(MoePCB_Params_Hina::LED::Placement::UpperLeft) != nullptr) {
    // 輝度レベルのみ変更
    for (MoePCB_Params_Hina::LED::led_index_t led_idx = 0; led_idx < MoePCB_Params_Hina::LED::Length; ++led_idx) {
      auto const pI = led.get_interface_rainbow(led_idx);
      if (pI != nullptr) {
        pI->set_brightness_level(BRIGHTNESS_IDX);
      }
    }
  } else {
    // 色生成処理を breath, rainbow に変更
    led.set_mode_breath(MoePCB_Params_Hina::LED::Placement::Heart);

    constexpr auto mode = MoePCB_LEDColorCalculator_Rainbow::Mode::NaturalAndTwinkle;
    for (uint8_t i = 0; i < sizeof(led_idx_list)/sizeof(led_idx_list[0]); ++i) {
      const auto led_idx = led_idx_list[i];
      led.set_mode_rainbow(led_idx, -base * i, 1.2f, mode, BRIGHTNESS_IDX);
    }
  }
}

// LEDモジュールの色生成処理を gaming に設定
void set_led_gaming() {
  auto& led = Hina.getMainLED().getColorCalculator();

  // Heart が gaming なら 他もすべて gaming
  if (led.get_interface_gaming(MoePCB_Params_Hina::LED::Placement::Heart) != nullptr) {
    // 輝度レベルのみ変更
    for (MoePCB_Params_Hina::LED::led_index_t led_idx = 0; led_idx < MoePCB_Params_Hina::LED::Length; ++led_idx) {
      auto const pI = led.get_interface_gaming(led_idx);
      if (pI != nullptr) {
        pI->set_brightness_level(BRIGHTNESS_IDX);
      }
    }
  } else {
    // 色生成処理を gaming に変更（位相差約20度）
    // 元実装では step = 6 だが update 間隔が早まったため 5 に落とす (それでもやや早い)
    led.set_mode_gaming(MoePCB_Params_Hina::LED::Placement::Heart, 0, 5, BRIGHTNESS_IDX);

    for (uint8_t i = 0; i < sizeof(led_idx_list)/sizeof(led_idx_list[0]); ++i) {
      const auto led_idx = led_idx_list[i];
      const uint8_t shift = ((uint16_t)256*i)/(360/20);
      led.set_mode_gaming(led_idx, -shift, 5, BRIGHTNESS_IDX);
    }
  }
}

// LED調光モード反映
void apply(bool onlyChangeBrightness) {
  // LEDモジュールの色生成処理を点灯モードに応じ切り替え
  switch(PATTERN_MODE) {
  case 0: set_led_autumn((PATTERN_SUBMODE == 0) ? 20 : 200, onlyChangeBrightness); break;
  case 1: set_led_rainbow((PATTERN_SUBMODE == 0) ? 30.0f : 300.0f); break;
  case 2: set_led_gaming(); break;
  }

  // LEDモジュールの色生成処理のベース輝度は最大値で固定
  Hina.getMainLED().getColorCalculator().setBrightnessAll(255);

  // フィルタの直前の色情報を初期化
  Hina.getMainLED().getColorFilter().resetLastColor();

  // LEDモジュールの色生成処理、フィルタ処理の更新を有効化
  Hina.getMainLED().getColorCalculator().setEnabledAll(true);
  Hina.getMainLED().getColorFilter().setEnabled(true);
}

// 了解コール
void acknowledge(int ms) {
  // サブLED点灯
  Hina.getSubLED().set(true);
  Hina.getSubLED().show();

  // LEDモジュールの色生成処理、フィルタ処理の更新を止める
  Hina.getMainLED().getColorCalculator().setEnabledAll(false);
  Hina.getMainLED().getColorFilter().setEnabled(false);

  // すべてのLEDを消灯
  Hina.getMainLED().getRawLED().clear();
  Hina.getMainLED().getRawLED().show();

  // 少し待機 (割り込みがかかってもLEDモジュールを無効化しているため消灯したままになる)
  delay(ms);

  // LEDモジュールの色生成処理、フィルタ処理の更新を再開
  Hina.getMainLED().getColorCalculator().setEnabledAll(true);
  Hina.getMainLED().getColorFilter().setEnabled(true);

  // サブLED消灯
  Hina.getSubLED().set(false);
  Hina.getSubLED().show();
}



// タイマー割り込みにより一定間隔で実行される処理
// 割り込み間隔は要確認
void MoePCB_Task() {
  Hina.interrupt(); // 萌基板割り込み実行
}

// 起動後に1回呼ばれるArduinoの関数
void setup() {
//  Serial.begin(9600);//シリアル通信を使いたいとき

  // 萌基板初期化
  Hina.begin();

  // LED ベース光量 最大値に設定
  Hina.getMainLED().getRawLED().setBrightness(255);

  apply(false);
}

// setup後繰り返し呼ばれるArduinoの関数
void loop() {
  // 萌基板フレーム更新処理
  Hina.update();

// TODO: ThermometerModule を利用したサンプルを書く
/* CPU内蔵温度感知機能を使う場合は校正した方がいいです（個体差大）
    static uint8_t cnt;
    if(30<cnt){
      cnt=0;
      Serial.print(Hina.cputemp_raw()); Serial.print(" / ");//CPU温度ADC値を見たい時
      Serial.print(Hina.cputemp(26.0, 305.0));//305.0のところは各自の校正値を入れる：校正時の周辺の実測温度、その時のCPU温度ADC値（上段）    
      Serial.println(" C");
    }
    cnt++;
    //50度超えると暑いよ〜
    if(50 < Hina.cputemp(26.0, 305.0) )Hina.heat(true);//cputempに校正値を与えると気温が返ってくる
    else Hina.heat(false);

    //10度下回ると寒いよ〜
    if(Hina.cputemp(26.0, 305.0) < 10 )Hina.cold(true);//cputempに校正値を与えると気温が返ってくる
    else Hina.cold(false);
*/    

  // タッチセンサインスタンスへの参照
  auto& touch_module = Hina.getTouchSensor();
  auto& ribbon_sensor = touch_module.getRawTouchSensor(MoePCB_Params_Hina::TouchSensor::Placement::Ribbon);
  auto& hair_sensor = touch_module.getRawTouchSensor(MoePCB_Params_Hina::TouchSensor::Placement::Hair);
  auto& chest_sensor = touch_module.getRawTouchSensor(MoePCB_Params_Hina::TouchSensor::Placement::Chest);
  auto& skirt_sensor = touch_module.getRawTouchSensor(MoePCB_Params_Hina::TouchSensor::Placement::Skirt);

  // 髪タッチ検知
  if (hair_sensor.isOnTouch()) {
    // 輝度レベルインクリメント
    BRIGHTNESS_IDX = (BRIGHTNESS_IDX + 1) % 4;

    acknowledge(80); 
    apply(true);
  }

  // 胸タッチ検出
  // 長押しと判定した場合怒りモードを有効にする
  Hina.getMainLED().getColorFilter().set_state_angry(chest_sensor.isPressing());

  //スカートタッチ検出
  if (skirt_sensor.isOnTouch()) {
    PATTERN_SUBMODE=0;   //サブモードを戻しておく

    PATTERN_MODE = (PATTERN_MODE + 1) % 3;

    acknowledge(80); 
    apply(false);
  }
  
  // リボンタッチ検出
  if (PATTERN_MODE != 2 && ribbon_sensor.isOnTouch()) {
    PATTERN_SUBMODE = (PATTERN_SUBMODE + 1) % 2;

    acknowledge(80); 
    apply(false);
  }

  // 60FPSを目指したdelay呼び出し。loop関数内の処理でdelay呼び出しが含まれているものもあり、
  // またタイマ割り込みの処理を行っている間loop関数の処理は進行しないため、実際にはloop関数は
  // 目標より低いFPSで進行する。また処理内容ごとに処理時間が変わるため、FPSは一定にならない。
  delay(15);
}
