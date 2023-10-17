/*!
 * MoePCB.cpp - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2023 Mizuhasi Yukkie
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */
#include "MoePCB.h"

#include "Arduino.h"

// タイマー４ 20-25ms割り込み
extern void MoePCB_Task(void);

// NO_USE_TIMER4が宣言されていなければタイマーで自動実行
ISR(TIMER4_COMPA_vect) { MoePCB_Task(); }

// タイマー１版 20-25ms割り込み　※未使用
//    ISR (TIMER1_COMPA_vect) {
//      MoePCB_Task();
//    }
// コンストラクタ
MoePCB::MoePCB(uint8_t led_num) {
  _led_num = led_num;  // LED数をプライベートに保存
  Adafruit_NeoPixel pixels(led_num, RGBLED_PIN,
                           NEO_GRB + NEO_KHZ800);  // NeoPixelライブラリ初期化
  _pixels =
      pixels;  // インスタンスハンドルをプライベートに保存　今後はプライベートな方にアクセス
}

// 指定されなかった場合はタイマー無効で開始する
void MoePCB::begin() { begin(false); }

void MoePCB::begin(bool timer_enable) {
  pinMode(LED0, OUTPUT);         // 背面ノーマルLED
  digitalWrite(LED0, LOW);       // ON
  _timer_enable = timer_enable;  // タイマー状況を保存
  _pixels.begin();               // RGBLEDライブラリ初期化
  _pixels.clear();
  _pixels.show();
  for (int i = 0; i < _led_num; i++) {  // 初期化
    V_raw[i] = 0;
    V[i] = 0;
  }

  // タイマー起動-IRsendやBMEと同時にタイマー使うと動かなくなることがある
  if (_timer_enable) {
    // タイマーA：20-25msごとにコンペアAクリア＆割り込みを設定
    TCCR4B = 0;
    TCCR4A = 0;
    TCCR4C = 0;
    TCCR4D = 0;
    TCCR4E = 0;
    TCCR4B = 0b1100;  // 1/2048
    OCR4C = 156;      // 0-255  <-- 16,000,000 MHz / 2048 / 50Hz
    // OCR4C =195;//0-255  <-- 16,000,000 MHz / 2048 / 40Hz
    // タイマー４開始処理
    TIFR4 = (1 << OCF4A);
    TCNT4 = 0;
    TIMSK4 = (1 << OCIE4A);
    digitalWrite(LED0, HIGH);  // OFF
  }
  /*
  //タイマー1：20-25msごとにコンペアAクリア＆割り込みを設定
  TCCR1B = 0;  TCCR1A = 0;  TCCR1C = 0;
  TCCR1B = (1<<WGM12)|(1<<CS02)|(1<<CS00);//0b00000010;// 1/1024
  OCR1A =312;//16bit  <-- 16,000,000 MHz / 1024 / 50Hz
  //OCR1A =390;//16bit  <-- 16,000,000 MHz / 1024 / 40Hz
  //タイマー1開始処理
  TIFR1 = (1<<OCF1A);
  TCNT1 = 0;
  TIMSK1 = (1<<OCIE1A);
  */
}
// 現在の温度のCPU読みADC値を返す
float MoePCB::cputemp_raw(void) {
  unsigned int wADC;
  ADCSRB = _BV(MUX5);  // MUX100111が温度感知器
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0));
  ADCSRA |= _BV(ADEN);
  delay(20);
  ADCSRA |= _BV(ADSC);  // 変換開始
  while (bit_is_set(ADCSRA, ADSC))
    ;  // 変換終了
  wADC = ADCW;
  return (wADC);
  // 室温 294.0-306.0くらい 電源入れた直後は289.0とか
  // 冷蔵庫に少し入れたあと 274.0-277.0
}
// 校正値から温度を返す
float MoePCB::cputemp(float celsius_const, float cpu_temp_raw_const) {
  float tmp_sabun = celsius_const - (cpu_temp_raw_const - 324.31 / 1.22);
  return (cputemp_raw() - 324.31 / 1.22) + tmp_sabun;
}
// 怒りモード発動
void MoePCB::angry(bool state) {
  if (state)
    angly_flag = 1;
  else
    angly_flag = 0;  // 収まり
}
// 寒いよ
void MoePCB::cold(bool state) {
  if (state)
    cold_flag = 1;
  else
    cold_flag = 0;  // 収まり
}
// 暑い
void MoePCB::heat(bool state) {
  if (state)
    heat_flag = 1;
  else
    heat_flag = 0;  // 収まり
}
// 酔ってるよ
void MoePCB::drunk(bool state) {
  if (state)
    drunk_flag = 1;
  else
    drunk_flag = 0;  // 収まり
}
//------------------------------------------------------------------------------------
// 消灯
void MoePCB::mute(int led_id) {
  H[led_id] = 0;  // 色環は関係なし
  S[led_id] = 0;  // 彩度ゼロ
  V[led_id] = 0;
}
//------------------------------------------------------------------------------------
// ゆっくり呼吸するような白色点滅
void MoePCB::breath(int led_id) {
  H[led_id] = 0;  // 色環は関係なし
  S[led_id] = 0;  // 彩度ゼロ
  // イージングは必要ないので輝度は生データを直接触る/
  if (general_cnt < 128)
    V_raw[led_id] = (general_cnt) / 3;
  else
    V_raw[led_id] = ((255 - general_cnt)) / 3;
}
//------------------------------------------------------------------------------------
// ゆっくり月色点滅
void MoePCB::moonbreath(int led_id) {
  H[led_id] = 60;   // 少し黄色
  S[led_id] = 200;  // 彩度
  // イージングは必要ないので輝度は生データを直接触る/
  if (general_cnt < 64)
    V_raw[led_id] = (general_cnt);
  else if (general_cnt < 128)
    V_raw[led_id] = ((64 - (general_cnt - 64)));
  else if (general_cnt < 192)
    V_raw[led_id] = ((general_cnt - 128));
  else
    V_raw[led_id] = ((64 - (general_cnt - 192)));
}  //------------------------------------------------------------------------------------
// ゆっくりシアン色点滅（裏LEDに使うと色透けが強い）
void MoePCB::cyanbreath(int led_id) {
  H[led_id] = 160;  // シアン
  S[led_id] = 255;  // 彩度
  // イージングは必要ないので輝度は生データを直接触る/
  if (general_cnt < 64)
    V_raw[led_id] = (general_cnt);
  else if (general_cnt < 128)
    V_raw[led_id] = ((64 - (general_cnt - 64)));
  else if (general_cnt < 192)
    V_raw[led_id] = ((general_cnt - 128));
  else
    V_raw[led_id] = ((64 - (general_cnt - 192)));
}
//------------------------------------------------------------------------------------
// ゲーミングモード
void MoePCB::gaming(int led_id, int phase_shift) {
  const uint8_t gaming_brightnessTable[4] = {
      25, 65, 175, 250};  // 明るさレベルに応じた明るさ値
  uint8_t cnt_tmp = gaming_cnt - phase_shift;
  V[led_id] = gaming_brightnessTable[_brightness];  // 基本光量より１段明るい
  H[led_id] = map(cnt_tmp, 0, 255, 0, 360);
  S[led_id] = 255;  // 彩度MAX
}
//------------------------------------------------------------------------------------
// 基本のゆっくり虹色変化　位相差をつけるとお好みの色差で光らせられる
// サブモードAはナチュラルキラキラ、Bはデジタル的、CはAのキラキラ無し、Dは強制キラッ（MIDIとかで使う）
const uint8_t midi_twinkleTable[4] = {
    65, 155, 255, 255};  // 明るさレベルに応じたモードDで使う光量
void MoePCB::rainbow(int led_id, int phase_shift, uint8_t sub_mode) {
  static int led_id_last;  // 1回前にピカッとしたLEDのIDを保存
  V[led_id] = _brightnessTable[_brightness];  // 基本光量
  if ((sub_mode != C) and (sub_mode != D)) {  // C,Dではここはスキップ
    if (random(0, 250) == 0) {                // ランダムで明るくする
      if (led_id != led_id_last) {  // 連続して同じLEDをピカピカしないように考慮
        V[led_id] = _twinkleTable[_brightness];  // キラッと光量
        V_raw[led_id] = V[led_id];  // 現在値を指示値で上書き
      }
      led_id_last = led_id;
    }
  }
  if (sub_mode == D) {                           // 強制キラッ
    V[led_id] = midi_twinkleTable[_brightness];  // キラッと光量
    V_raw[led_id] = V[led_id];  // 現在値を指示値で上書き
  }
  if (sub_mode == A) H[led_id] = rainbow_cnt - phase_shift;
  if (sub_mode == B) H[led_id] = (((int)rainbow_cnt - phase_shift) >> 5) << 5;
  if (sub_mode == C) H[led_id] = rainbow_cnt - phase_shift;
  S[led_id] = 255;  // 彩度MAX
}
//------------------------------------------------------------------------------------
// ひんやり光パターン
void MoePCB::icy(int led_id, uint8_t sub_mode) {
  static int led_id_last;  // 1回前にピカッとしたLEDのIDを保存
  const uint8_t icy_brightnessTable[4] = {3, 10, 20,
                                          30};  // 明るさレベルに応じた明るさ値
  const uint8_t icy_twinkleTable[4] = {
      40, 65, 165, 255};  // 明るさレベルに応じたランダムで変更するきらきら値
  V[led_id] = icy_brightnessTable[_brightness];  // 基本光量
  static float V_last[30];

  if (S[led_id] > 0) S[led_id] -= 3;  // 設定値より現実の値が高ければ徐々に減算

  if (sub_mode == A) {
    if (random(0, 200) == 0) {
      if (led_id != led_id_last) {  // 連続して同じLEDをピカピカしないように考慮
        V[led_id] =
            icy_twinkleTable[_brightness];  // 明るさを範囲でランダムで変更
        V_raw[led_id] = V[led_id];
        S[led_id] = random(210, 330);  // 彩度をランダム変更
        S_raw[led_id] = S[led_id];
        if (random(0, 100) < 95) {  // 95/100の確率で水色系のランダム色
          H[led_id] = random(140, 250);  // 水色〜青のランダム色
        } else {                         // 5/100の確率で黄色
          H[led_id] = 42;                // 稀に黄色
          S[led_id] = 180;               // 黄色の時は彩度少し抑える
          S_raw[led_id] = S[led_id];
        }
      }
      led_id_last = led_id;
    }
    V_last[led_id] = V_raw[led_id];  // 初期化代わりに値を入れておく
  }
  // 最後のピカッとしたときの明るさを保持して強制的につっこむ
  if (sub_mode == B) {
    if (random(0, 50) == 0) {
      if (led_id != led_id_last) {  // 連続して同じLEDをピカピカしないように考慮
        V[led_id] = random(
            icy_brightnessTable[_brightness],
            icy_twinkleTable[_brightness]);  // 明るさを範囲でランダムで変更
        V_raw[led_id] = V[led_id];
        V_last[led_id] = V[led_id];
        S[led_id] = random(210, 330);  // 彩度をランダム変更
        S_raw[led_id] = S[led_id];
        if (random(0, 100) < 95) {  // 95/100の確率で水色系のランダム色
          H[led_id] = random(140, 250);  // 水色〜青のランダム色
        } else {                         // 5/100の確率で黄色
          H[led_id] = 42;                // 稀に黄色
          S[led_id] = 180;               // 黄色の時は彩度少し抑える
          S_raw[led_id] = S[led_id];
        }
      }
      led_id_last = led_id;
    } else {
      V_raw[led_id] = V_last[led_id];
    }
  }
}
//------------------------------------------------------------------------------------
// ゆっくり秋色変化　位相差をつけるとお好みの色差で光らせられる
void MoePCB::autumn(int led_id, int phase_shift) {
  static int led_id_last;  // 1回前にピカッとしたLEDのIDを保存
  V[led_id] = _brightnessTable[_brightness];  // 基本光量

  if (random(0, 250) == 0) {  // ランダムで明るくする
    if (led_id != led_id_last) {  // 連続して同じLEDをピカピカしないように考慮
      V[led_id] = _twinkleTable[_brightness];  // キラッと光量
      V_raw[led_id] = V[led_id];  // 現在値を指示値で上書き
    }
    led_id_last = led_id;
  }
  uint8_t tmp = general_cnt - phase_shift;
  if (127 < tmp)
    H[led_id] = map(tmp, 0, 127, 240, 120);
  else
    H[led_id] = map(tmp, 127, 255, 120, 240);
  S[led_id] = 255;  // 彩度MAX
}
//------------------------------------------------------------------------------------
// 緋想の剣　位相差をつけるとお好みの色差で光らせられる modeによって色が変わる
void MoePCB::sword(int led_id, int phase_shift, uint8_t sub_mode,
                   uint8_t color_mode) {
  V[led_id] = _brightnessTable[_brightness];  // 基本光量
  static uint8_t last_color_mode[30];  // 暫定で30個にしている　最適化すべき
  // 変更があったら色を変える
  if (last_color_mode[led_id] != color_mode) {
    V[led_id] = 0;
    V_raw[led_id] = V[led_id];
  }
  last_color_mode[led_id] = color_mode;
  uint8_t tmp = general_cnt - phase_shift;

  if (color_mode == A) {  // 燃えるような色
    if (tmp < 127) {
      H[led_id] = map(tmp, 0, 127, 8, 56);
    } else {
      H[led_id] = map(tmp, 127, 255, 56, 8);
    }
  } else if (color_mode == B) {  // 空色
    if (tmp < 127) {
      H[led_id] = map(tmp, 0, 127, 140, 255);
    } else {
      H[led_id] = map(tmp, 127, 255, 255, 140);
    }
  }
  // 彩度
  if (tmp < 150) {
    S[led_id] = map(tmp, 0, 200, 255, 200);
  } else {
    S[led_id] = map(tmp, 200, 255, 200, 255);
  }

  if (sub_mode == B) {
    // 輝度少し弄る
    if (_brightness == 0)
      V_raw[led_id] = V[led_id] - map(tmp, 0, 255, 0, 9);
    else if (_brightness == 1)
      V_raw[led_id] = V[led_id] - map(tmp, 0, 255, 0, 18);
    else if (_brightness == 2)
      V_raw[led_id] = V[led_id] - map(tmp, 0, 255, 0, 60);
    else if (_brightness == 3)
      V_raw[led_id] = V[led_id] - map(tmp, 0, 255, 0, 130);
  }
}
//------------------------------------------------------------------------------------
// レベルメーター
void MoePCB::lvmeter(int led_id, int atach_position,
                     uint8_t sub_mode) {  // Aはシンプル、Bはピーク付き
  V[led_id] = _brightnessTable[_brightness];  // 基本光量
  int V_tmp;
  V_tmp = _brightnessTable[_brightness];
  H[led_id] =
      map(atach_position, 0, 255, 180,
          -10);  // メーターの色を青から赤へ　ここ弄るとメーターの色変わる
  S[led_id] = 255;  // 彩度MAX

  // メーターポジション以上では輝度ゼロ（消灯）
  if (_LevelMeter < atach_position)
    V_raw[led_id] = 0;
  else
    V_raw[led_id] = V_tmp;

  // ピークLED
  if (sub_mode == B) {
    if ((_LevelPeak / 25) == (atach_position / 25)) {
      V_raw[led_id] = V_tmp;  // ピーク値は光らせる
    }
  }
}
//------------------------------------------------------------------------------------
// お星さまキラキラパターン
void MoePCB::twinklestar(int led_id, uint8_t sub_mode) {
  static int led_id_last;  // 1回前にピカッとしたLEDのIDを保存
  const uint8_t star_brightnessTable[4] = {0, 0, 0,
                                           0};  // 明るさレベルに応じた明るさ値
  const uint8_t star_twinkleTable[4] = {
      10, 25, 75, 165};  // 明るさレベルに応じたランダムで変更するきらきら値
  V[led_id] = star_brightnessTable[_brightness];  // 基本光量
  static float V_last[30];

  int randomness;
  if (_brightness == 0) randomness = 200;
  if (_brightness == 1) randomness = 150;
  if (_brightness == 2) randomness = 120;
  if (_brightness == 3) randomness = 100;

  if (S[led_id] > 0) S[led_id] -= 3;  // 設定値より現実の値が高ければ徐々に減算

  if (sub_mode == A) {
    if (random(0, randomness) == 0) {
      if (led_id != led_id_last) {  // 連続して同じLEDをピカピカしないように考慮
        V[led_id] =
            star_twinkleTable[_brightness];  // 明るさを範囲でランダムで変更

        V_raw[led_id] = V[led_id];
        S[led_id] = random(210, 330);  // 彩度をランダム変更
        S_raw[led_id] = S[led_id];
        if (random(0, 100) < 95) {  // 95/100の確率で黄色系のランダム色
          H[led_id] = random(30, 60);    // 黄色のランダム色
          S[led_id] = 180;               // 彩度少し抑える
        } else {                         // 5/100の確率で黄色
          H[led_id] = random(140, 250);  // 稀に青色
          S_raw[led_id] = S[led_id];
          S[led_id] = 180;  // 彩度少し抑える
        }
      }
      led_id_last = led_id;
    }
    V_last[led_id] = V_raw[led_id];  // 初期化代わりに値を入れておく
  }

  // 最後のピカッとしたときの明るさを保持して強制的につっこむ
  if (sub_mode == B) {
    if (random(0, 50) == 0) {
      if (led_id != led_id_last) {  // 連続して同じLEDをピカピカしないように考慮
        V[led_id] = random(
            star_brightnessTable[_brightness],
            star_twinkleTable[_brightness]);  // 明るさを範囲でランダムで変更
        V_raw[led_id] = V[led_id];
        V_last[led_id] = V[led_id];
        S[led_id] = random(210, 330);  // 彩度をランダム変更
        S_raw[led_id] = S[led_id];
        if (random(0, 100) < 95) {  // 95/100の確率で黄色系のランダム色
          H[led_id] = random(30, 60);    // 黄色のランダム色
          S[led_id] = 180;               // 彩度少し抑える
        } else {                         // 5/100の確率で黄色
          H[led_id] = random(140, 250);  // 稀に青色
          S_raw[led_id] = S[led_id];
          S[led_id] = 180;  // 彩度少し抑える
        }
      }
      led_id_last = led_id;
    } else {
      V_raw[led_id] = V_last[led_id];
    }
  }
}
//------------------------------------------------------------------------------------
// 魔理沙用通常キラキラパターン
void MoePCB::marisa_twinkle(int led_id, uint8_t position) {
  // 明るさレベルに応じた明るさ値
  const uint8_t star_brightnessTable[4] = {2, 3, 3, 4};
  // 明るさレベルに応じたランダムで変更するきらきら値
  const uint8_t star_twinkleTable[4] = {25, 50, 100, 200};
  //  const uint8_t star_twinkleTable[4] = {10, 25, 75, 165};
  V[led_id] = star_brightnessTable[_brightness];  // 基本光量
  static float V_last[30];

  if (S[led_id] > 0) S[led_id] -= 3;  // 設定値より現実の値が高ければ徐々に減算

  int randomness;
  if (_brightness == 0) randomness = 500;
  if (_brightness == 1) randomness = 300;
  if (_brightness == 2) randomness = 200;
  if (_brightness == 3) randomness = 100;
  if (random(0, randomness) == 0) {
    H[led_id] = random(30, 60);
    S[led_id] = 255;
    V[led_id] = star_twinkleTable[_brightness];  // 明るさを範囲でランダムで変更
    V_raw[led_id] = V[led_id];
  }

  if (general_cnt == position) {
    V[led_id] = star_twinkleTable[_brightness];  // 明るさを範囲でランダムで変更
    V_raw[led_id] = V[led_id];
    S[led_id] = random(210, 330);  // 彩度をランダム変更
    S_raw[led_id] = S[led_id];
    if (random(0, 100) < 95) {  // 95/100の確率で黄色系のランダム色
      H[led_id] = map(position, 0, 29, 0, 360);
      //  H[led_id] = random(30, 60);    // 黄色のランダム色
      S[led_id] = 255;               // 彩度
    } else {                         // 5/100の確率で黄色
      H[led_id] = random(140, 250);  // 稀に青色
      S_raw[led_id] = S[led_id];
      S[led_id] = 180;  // 彩度少し抑える
    }
  }
  V_last[led_id] = V_raw[led_id];  // 初期化代わりに値を入れておく
}
//------------------------------------------------------------------------------------
// マスタースパーク
void MoePCB::masterspark(int led_id, int phase_shift) {
  masterspark(led_id, phase_shift, false);
}
void MoePCB::masterspark(int led_id, int phase_shift, bool hakkero) {
  uint8_t cnt_tmp = (general_cnt - phase_shift);

  if (S[led_id] < (255 - 50))
    S[led_id] += 50;  // ピカッ時は彩度を下げてまた彩度MAXに戻す
  if (S[led_id] < 255) S[led_id] += 1;

  if (V_raw[led_id] > V[led_id])
    V_raw[led_id] -= 8.0;  // 設定値より現実の値が高ければ徐々に減算

  // 特定の時間で突っ込む値
  switch (cnt_tmp % 20) {
    case 0:

      S[led_id] = 255;
      V[led_id] = 255;  // 強い光
      V_raw[led_id] = V[led_id];
      H[led_id] = 10;
      break;

    case 2:

      S[led_id] = 0;
      V[led_id] = 255;  // 強い光
      V_raw[led_id] = V[led_id];
      if (random(0, 7) == 0) {
        H[led_id] = random(0, 360);  // 稀にカラフル
      } else {
        H[led_id] = random(30, 60);  // 基本黄色
      }
      break;

    default:
      V[led_id] = 80;  // ベースの明るさ
      break;
  }
  if (hakkero) {
    V[led_id] = 255;  // 最大輝度
    H[led_id] = random(00, 360);
    S[led_id] = 255;
  }

  //  V[led_id] = spark_brightnessTable[_brightness];  // 基本光量より１段明るい
  //  H[led_id] = 0;
  //  S[led_id] = 255;  // 彩度MAX
}  //------------------------------------------------------------------------------------
// マスパチャージ
void MoePCB::masterspark_charge() {
  for (int led_id = 0; led_id < _led_num; led_id++) {
    if (V_raw[led_id] < 80) V_raw[led_id] += 4.0;
    H[led_id] = random(30, 90);
    S[led_id] = 220;
  }
}
//------------------------------------------------------------------------------------

// 明るさを１段階追加する　最大→最小へ循環する
void MoePCB::brightness_add(void) {
  if (brightness == 0)
    brightness = 1;
  else if (brightness == 1)
    brightness = 2;
  else if (brightness == 2)
    brightness = 3;
  else
    brightness = 0;

  // イージングを無視してすぐに明るさを変更する
  // デフォルトのテーブルを使用しているため点灯モードによっては少しズレる(例:icyなど)
  for (int i = 0; i < _led_num; i++) {
    V[i] = _brightnessTable[brightness];
    V_raw[i] = V[i];
  }
  // 了解コール
  acknowledge();
}

// 了解コール
void MoePCB::acknowledge() {
  digitalWrite(LED0, LOW);  // ON

  // タイマーが許可されていれば割り込み一時停止
  if (_timer_enable) TIMSK4 = (0 << OCIE4A);  // 割り込み停止

  // タイマーが起動していなければfor文で無理やり時間を稼ぐ
  if (!_timer_enable) {
    for (int i = 0; i < 100; i++) {
      _pixels.clear();
      _pixels.show();
    }
  }

  _pixels.clear();
  _pixels.show();
  delay(60);

  // タイマーが許可されていれば割り込み一時停止
  if (_timer_enable) TIMSK4 = (1 << OCIE4A);  // 割り込み再開

  digitalWrite(LED0, HIGH);  // OFF
}

// モーフィング関数
float MoePCB::morph(float target_color, float H, uint8_t Gauge) {
  float tmp;
  if (Gauge != 0) {
    if ((180.0 + target_color) > H) {
      tmp = H * (255.0 - (float)Gauge) / 255.0 +
            (target_color * ((float)Gauge / 255.0));
    } else {
      tmp = H * (255.0 - (float)Gauge) / 255.0 +
            ((360 + target_color) * ((float)Gauge / 255.0));
    }
  } else {
    tmp = H;  // ゲージがゼロなら入力そのまま返す
  }
  return tmp;
}

// レベルメーターゲージに値を投入
void MoePCB::lvmeter_input(int input_level) {
  _LevelMeter = constrain(input_level, 0, 300);
  if (_LevelPeak <= _LevelMeter) _LevelPeak = _LevelMeter;  // ピーク値を更新
}

void MoePCB::update() {
  // Hは色環度数の指示値（0-359）
  // Sは彩度指示値（0-255）
  // Vは照度指示値（0-255）

  for (int i = 0; i < _led_num;
       i++) {  // LEDの数だけ計算を繰り返す。V[n]で指示した値にV_raw[n]が追従

    // 色環計算関係

    // 寒さゲージによって彩度を上書きモーフィングする。最終地点の色環に近い方に回す仕組みを採用
    // ターゲット色、現在色環、モーフィングゲージ（0-255）
    H[i] = morph(190, H[i], ColdGauge);

    // 暑さゲージによって彩度を上書きモーフィングする。最終地点の色環に近い方に回す仕組みを採用
    // ターゲット色、現在色環、モーフィングゲージ（0-255）
    H[i] = morph(0, H[i], HeatGauge);

    // 酔いゲージによって彩度を上書きモーフィングする。最終地点の色環に近い方に回す仕組みを採用
    // ターゲット色、現在色環、モーフィングゲージ（0-255）
    H[i] = morph(-7, H[i], DrunkGauge);

    // 怒りゲージによって彩度を上書きモーフィングする。最終地点の色環に近い方に回す仕組みを採用
    // ターゲット色、現在色環、モーフィングゲージ（0-255）
    H[i] = morph(-7, H[i], FuryGauge);

    // 指示値を実値に反映
    H_raw[i] = H[i];

    if (360 < H_raw[i]) H_raw[i] -= 360;  // 色環が回ってしまったら一周分引く
    if (H_raw[i] < 0) H_raw[i] += 360;  // 色環が回ってしまったら一周分足す

    // 彩度計算関係
    // 設定値と現実の値の差分で加減速
    S_raw[i] += (S[i] - S_raw[i]) / 10;
    // 怒りゲージにより彩度設定を無視して最大彩度になる
    S_raw[i] = max(S_raw[i], FuryGauge);
    // 寒さゲージにより彩度設定を無視して最大彩度になる
    S_raw[i] = max(S_raw[i], ColdGauge);
    // 暑さゲージにより彩度設定を無視して最大彩度になる
    S_raw[i] = max(S_raw[i], HeatGauge);
    // 酔いゲージにより彩度設定を無視して最大彩度になる
    S_raw[i] = max(S_raw[i], DrunkGauge);

    S_raw[i] = constrain(S_raw[i], 0, 255);  // 値を制限
    S[i] = constrain(S[i], 0, 255);          // 値を制限

    // 照度計算関係
    if (V_raw[i] <
        16) {  // 比例計算していると０近くの動きが鈍くなるので、０に近づいたら純粋に一定値で増減させる
      if (V_raw[i] > V[i])
        V_raw[i] -= 0.5;  // 設定値より現実の値が高ければ徐々に減算
      if (V_raw[i] < V[i])
        V_raw[i] += 0.5;  // 設定値より現実の値が低ければ徐々に加算
    } else {
      V_raw[i] += (V[i] - V_raw[i]) / 30;  // 設定値と現実の値の差分で加減速
    }
    // 怒りゲージにより明るさ設定を無視して最大輝度になる
    V_raw[i] = max(V_raw[i], FuryGauge) - pulsation;
    V_raw[i] = constrain(V_raw[i], 0, 255);  // 値を制限
    V[i] = constrain(V[i], 0, 255);          // 値を制限

    // LEDごとに値を送信
    _pixels.setPixelColor(i, _pixels.ColorHSV(map(H_raw[i], 0, 360, 0, 65535),
                                              S_raw[i], V_raw[i]));
  }

  // 虹色用カウンタ 0-360で一周（色環と一致）
  rainbow_cnt += 1.2;  // ゆっくり自動で色環指示値を回す
  if (360 < rainbow_cnt)
    rainbow_cnt -= 360;  // 色環が回ってしまったら一周分引く 0-360
  if (rainbow_cnt < 0)
    rainbow_cnt += 360;  // 色環が回ってしまったら一周分足す 0-360

  // 汎用カウンタ
  general_cnt += 1;

  // ゲーミングモード用カウンタ
  gaming_cnt += 6;

  // レベルメーター
  // if(0<(_LevelMeter-5)) _LevelMeter = _LevelMeter - 5;
  if (0 < _LevelMeter) _LevelMeter -= (_LevelMeter) / 10;
  if (0 < _LevelMeter) _LevelMeter -= 1;

  // レベルメーターのピーク値
  if (0 < _LevelPeak) _LevelPeak -= (_LevelPeak) / 30;
  if (0 < _LevelPeak) _LevelPeak -= 1;

  // 明るさ
  _brightness = constrain(brightness, 0, 3);

  _pixels.show();  // 一斉に更新

  digitalWrite(LED0, HIGH);  // OFF

  // 怒りフラグが立ったらゲージを自動で増減する
  if (angly_flag) {
    digitalWrite(LED0, LOW);  // ON
    if (250 < FuryGauge) {    // ゲージが増えていったら
      pulsation += 8;  // 脈動のためのゲージをチャージする
      if (160 < pulsation) pulsation = 0;
    }
    if ((FuryGauge + 6) < 255) FuryGauge = FuryGauge + 5;
    if ((FuryGauge) < 255) FuryGauge = FuryGauge + 1;
  } else {
    pulsation = 0;  // 脈動ゲージクリア
    if (0 < (FuryGauge - 12)) FuryGauge = FuryGauge - 12;
    if (0 < FuryGauge) FuryGauge = FuryGauge - 1;
  }

  // 寒さフラグが立ったらゲージを自動で増減する
  if (cold_flag) {
    digitalWrite(LED0, LOW);  // ON
    if ((ColdGauge) < 255) ColdGauge = ColdGauge + 1;
  } else {
    if (0 < ColdGauge) ColdGauge = ColdGauge - 1;
  }

  // 暑さフラグが立ったらゲージを自動で増減する
  if (heat_flag) {
    digitalWrite(LED0, LOW);  // ON
    if ((HeatGauge) < 255) HeatGauge = HeatGauge + 1;
  } else {
    if (0 < HeatGauge) HeatGauge = HeatGauge - 1;
  }

  // 酔いフラグが立ったらゲージを自動で増減する
  if (drunk_flag) {
    digitalWrite(LED0, LOW);  // ON
    if ((DrunkGauge) < 255) DrunkGauge = DrunkGauge + 1;
  } else {
    if (0 < DrunkGauge) DrunkGauge = DrunkGauge - 1;
  }
}
