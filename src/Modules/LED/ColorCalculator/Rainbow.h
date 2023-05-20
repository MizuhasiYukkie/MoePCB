/*!
 * Modules/LED/ColorCalculator/Rainbow.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorCalculator_Rainbow_h
#define Modules_LED_ColorCalculator_Rainbow_h

#include "Base.h"
#include <new>

// 基本のゆっくり虹色変化　位相差をつけるとお好みの色差で光らせられる
class MoePCB_LEDColorCalculator_Rainbow
{
private:
  // クラスを識別するための固有の定数
  // TODO: 適当な関数の配置先アドレス等コンパイル時に一意に定まる値を流用したい (コンパイラの警告に引っかかるため出来ていない)
  static constexpr uintptr_t IdValue = 0x50;

public:
  // 点灯モード
  // NaturalAndTwinkle : ナチュラル (キラキラあり)
  // DigitalAndTwinkle : デジタル的
  // Natural : ナチュラル (キラキラなし)
  // ForceTwinkle : 強制キラッ（MIDIとかで使う）
  class Mode
  {
    friend class MoePCB_LEDColorCalculator_Rainbow;

  public:
    typedef uint8_t type_t;

  public:
    static constexpr type_t NaturalAndTwinkle = 0; // ナチュラル (キラキラあり)
    static constexpr type_t DigitalAndTwinkle = 1; // デジタル的
    static constexpr type_t Natural = 2;           // ナチュラル (キラキラなし)
    static constexpr type_t ForceTwinkle = 3;      // 強制キラッ（MIDIとかで使う）

  private:
    static constexpr type_t RequireBits = 2; // 内部用 : 必要なビット幅
  };

  // 明るさレベル
  class BrightnessLevel {
    friend class MoePCB_LEDColorCalculator_Rainbow;

  public:
    typedef uint8_t type_t;

  private:
    static constexpr type_t RequireBits = 2; // 内部用 : 必要なビット幅
  };

public:
  // LED調光モジュール個別インターフェース
  class SpecificInterface : public MoePCB_LEDColorCalculator_SpecificInterfaceBase
  {
  private:
    // 連続して同じLEDをピカピカしないよう、インターバルを設定
    typedef uint8_t twinkle_interval_t;
    static constexpr twinkle_interval_t twinkle_cooltime = 32;

    //明るさレベルに応じた輝度
#if __cpp_constexpr >= 201304
    // C++14以降向け (こちらの実装でもビルド、実行できるが警告が出る)
    // cite: https://cpprefjp.github.io/lang/cpp14/relaxing_constraints_on_constexpr.html
    static inline constexpr uint8_t BrightnessTable[4]  = { 10,  25,  65, 175 }; // 通常輝度
    static inline constexpr uint8_t TwinkleTable[4]     = { 40,  65, 155, 255 }; // ランダムきらきら輝度
    static inline constexpr uint8_t MidiTwinkleTable[4] = { 65, 155, 255, 255 }; // ForceKiraKira で使う輝度
    constexpr uint8_t GetBrightness(BrightnessLevel::type_t brightness_level) { return BrightnessTable[brightness_level]; }
    constexpr uint8_t GetTwinkleBrightness(BrightnessLevel::type_t brightness_level) { return TwinkleTable[brightness_level]; }
    constexpr uint8_t GetMidiBrightness(BrightnessLevel::type_t brightness_level) { return MidiTwinkleTable[brightness_level]; }
#else
    constexpr uint8_t GetBrightness(BrightnessLevel::type_t brightness_level) {
      // 基本光量
      switch (brightness_level) {
        case 0: return 10;
        case 1: return 25;
        case 2: return 65;
        case 3: return 175;
        default: return 0;
      }
    }
    constexpr uint8_t GetTwinkleBrightness(BrightnessLevel::type_t brightness_level) {
      // ランダムきらきら輝度
      switch (brightness_level) {
        case 0: return 40;
        case 1: return 65;
        case 2: return 155;
        case 3: return 255;
        default: return 0;
      }
    }
    constexpr uint8_t GetMidiBrightness(BrightnessLevel::type_t brightness_level) {
      // ForceKiraKira で使う輝度
      switch (brightness_level) {
        case 0: return 65;
        case 1: return 155;
        case 2: return 255;
        case 3: return 255;
        default: return 0;
      }
    }
#endif

  public:
    SpecificInterface() = delete;
    SpecificInterface(int phase_shift, float step, Mode::type_t mode, BrightnessLevel::type_t brightness_level)
      : mode(mode)
      , brightness_level(brightness_level)
      , twinkle_interval(random(0, twinkle_cooltime))
      , rainbow_cnt(phase_shift)
      , step(step)
    { }
    ~SpecificInterface() = default;

  protected:
    void* identifier() const override { return (void *) IdValue; }
    void begin() override { }
    void end() override { }

    color_t update() override
    {
      // V 輝度
      uint8_t V;
      bool forceV = false;
      {
        V = GetBrightness(brightness_level); //基本光量

        // 点灯モードでキラキラを要求されている場合の処理
        if (mode == Mode::NaturalAndTwinkle || mode == Mode::DigitalAndTwinkle) {
          // 連続して同じLEDをピカピカしないように考慮
          if (twinkle_interval > 0) {
            --twinkle_interval;
          }
          else {
            if(random(0, 250) == 0){ //ランダムで明るくする
              V = GetTwinkleBrightness(brightness_level);//キラッと光量
              forceV = true;
              twinkle_interval = twinkle_cooltime;
            }
          }
        }

        // 点灯モード ForceTwinkle の処理
        if(mode == Mode::ForceTwinkle){//強制キラッ
          V = GetMidiBrightness(brightness_level);//キラッと光量
          forceV = true;
        }

        // 輝度反映
        V = (V * this->brightness) / 256;
      }

      // H 色相
      float H;
      {
        switch (mode) {
        case Mode::NaturalAndTwinkle:
          H = rainbow_cnt;
          break;
        case Mode::DigitalAndTwinkle:
          H = (((int)rainbow_cnt)>>5 )<<5;
          break;
        case Mode::Natural:
          H = rainbow_cnt;
          break;
        case Mode::ForceTwinkle:
          // MEMO: 元実装でここは空だったが、サンプル全体の実装を見ると意図としてはこれで良さそう
          // TODO: Midi用強制キラキラ対応は別モジュールの方が良さそう
          H = rainbow_cnt;
          break;
        }
      }

      // S 彩度
      uint8_t S;
      {
        // 最大値に固定
        S = 255;
      }

      // 虹色用カウンタ 0-360で一周（色環と一致）
      {
        // 自動で色環指示値を回す
        rainbow_cnt += step;  

        // 色環が回ってしまったら一周分引く 0-360
        if (360 < rainbow_cnt) { rainbow_cnt -= 360; }
        // 色環が回ってしまったら一周分足す 0-360
        if (rainbow_cnt < 0) { rainbow_cnt += 360; }
      }

      {
        const uint16_t hue = (uint16_t) (H * (float)(65535.0 / 360.0));
        return color_t(hue, S, V, false, false, forceV);
      }
    }

  public:
    // 点灯モードを取得
    Mode::type_t get_mode() const { return mode; }
    // 点灯モードを設定
    void set_mode(Mode::type_t v) { mode = v; }

    // 明るさレベル最大値を取得
    static constexpr BrightnessLevel::type_t get_max_brightness_level() {
#if __cpp_constexpr >= 201304
      return sizeof(BrightnessTable)/sizeof(BrightnessTable[0]) - 1;
#else
      return 4 - 1;
#endif
    }
    // 明るさレベルを取得
    BrightnessLevel::type_t get_brightness_level(void) const { return brightness_level; }
    // 明るさレベルを設定
    void set_brightness_level(BrightnessLevel::type_t level) {
      if (level <= get_max_brightness_level()) {
        brightness_level = level;
      }
    }

    // フレーム当たりの色相変化量を取得
    float get_step() const { return step; }
    // フレーム当たりの色相変化量を設定
    void set_step(float v) { step = v; }

  private:
    // 初期化で与える値
    Mode::type_t mode : Mode::RequireBits; // 点灯モード
    BrightnessLevel::type_t brightness_level : BrightnessLevel::RequireBits; // 明るさレベル

    // 自動で更新する値
    twinkle_interval_t twinkle_interval;
    float rainbow_cnt; // カウンタ　色環に一致（0-360）
    float step; // 1フレームで色相環を回す角度
  };

  // LED調光モジュールクラス用インターフェース
  template<class T_MoePCB_Params>
  class ModuleInterface : public MoePCB_LEDColorCalculator_ModuleInterfaceBase<T_MoePCB_Params>
  {
  public:
    // 調光モードを Rainbow に設定する
    void set_mode_rainbow(typename T_MoePCB_Params::LED::led_index_t led_idx, float phase_shift, float step = 1.2f, Mode::type_t mode = Mode::Natural, BrightnessLevel::type_t brightness_level = 1) {
      void * const p = this->allocateInterface(led_idx);
      new(p) SpecificInterface(phase_shift, step, mode, brightness_level);
    }

    // 調光モード Rainbow の調光モジュールインターフェースを取得する
    // 調光モードが Rainbow 以外の場合は nullptr が返る
    SpecificInterface* get_interface_rainbow(typename T_MoePCB_Params::LED::led_index_t led_idx) {
      auto const pBase = this->getInterface(led_idx);
      if (pBase == nullptr) { return nullptr; }
      auto const pI = reinterpret_cast<SpecificInterface *>(pBase);
      if (pBase->identifier() == (void *) IdValue) {
        return pI;
      } else {
        return nullptr;
      }
    }
  };
};

#endif
