/*!
 * Modules/LED/ColorCalculator/Sword.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorCalculator_Sword_h
#define Modules_LED_ColorCalculator_Sword_h

#include "Base.h"
#include <new>

// 緋想の剣　位相差をつけるとお好みの色差で光らせられる
// Color によって色が変わる、 Mode によって光り方が変わる
class MoePCB_LEDColorCalculator_Sword
{
private:
  // クラスを識別するための固有の定数
  // TODO: 適当な関数の配置先アドレス等コンパイル時に一意に定まる値を流用したい (コンパイラの警告に引っかかるため出来ていない)
  static constexpr uintptr_t IdValue = 0x70;

private:
  // [in_min, in_max] の範囲をとる型 T の入力値 x を、型 U の範囲 [out_min, out_max] に射影する
  // 内部計算は型 V で行い、計算結果は [Min, Max] でクリッピングされる
  template<typename T, typename U, typename V, V Min, V Max>
  inline static U map_base(T x, T in_min, T in_max, U out_min, U out_max) {
    const V a = (V) x - (V) in_min;
    const V b = (V) out_max - (V) out_min;
    const V c = (V) in_max - (V) in_min;

    if (c == 0) {
      return Max;
    }
    
    const V t = a * b / c + out_min;
    if (t < Min) { return Min; }
    if (t > Max) { return Max; }
    return t;
  }

  // uint8_t の値を uint8_t の範囲に射影する
  inline static uint8_t map_ui8toui8(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max) {
    return map_base<uint8_t, uint8_t, int16_t, 0, 255>(x, in_min, in_max, out_min, out_max);
  }

  // uint8_t の値を uint16_t の範囲に射影する
  inline static uint16_t map_ui8toui16(uint8_t x, uint8_t in_min, uint8_t in_max, uint16_t out_min, uint16_t out_max) {
    return map_base<uint8_t, uint16_t, int32_t, 0, 65535>(x, in_min, in_max, out_min, out_max);
  }

public:
  // 剣の色
  // FireRed : 燃えるような色
  // SkyBlue : 空色
  class Color
  {
    friend class MoePCB_LEDColorCalculator_Sword;

  public:
    typedef uint8_t type_t;

  public:
    static constexpr type_t FireRed = 0; // 燃えるような色
    static constexpr type_t SkyBlue = 1; // 空色

  private:
    static constexpr type_t RequireBits = 1; // 内部用 : 必要なビット幅
  };

  // 発光モード
  // Default : 通常
  // Twinkle : 流れるキラキラ
  class Mode
  {
    friend class MoePCB_LEDColorCalculator_Sword;

  public:
    typedef uint8_t type_t;

  public:
    static constexpr type_t Default = 0; // 通常
    static constexpr type_t Twinkle = 1; // 流れるキラキラ

  private:
    static constexpr type_t RequireBits = 1; // 内部用 : 必要なビット幅
  };

  // 明るさレベル
  class BrightnessLevel {
    friend class MoePCB_LEDColorCalculator_Sword;

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
    // 明るさレベルに応じた輝度
#if __cpp_constexpr >= 201304
    // C++14以降向け (こちらの実装でもビルド、実行できるが警告が出る)
    // cite: https://cpprefjp.github.io/lang/cpp14/relaxing_constraints_on_constexpr.html
    static inline constexpr uint8_t BrightnessTable[4]          = { 10, 25, 65, 175 }; // 通常輝度
    static inline constexpr uint8_t BrightnessDecreaseTable[4]  = {  9, 18, 60, 130 }; // Twinkle で下げる輝度
    constexpr uint8_t GetBrightness(BrightnessLevel::type_t brightness_level) { return BrightnessTable[brightness_level]; }
    constexpr uint8_t GetBrightnessDecrease(BrightnessLevel::type_t brightness_level) { return BrightnessDecreaseTable[brightness_level]; }
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
    constexpr uint8_t GetBrightnessDecrease(BrightnessLevel::type_t brightness_level) {
      // ランダムきらきら輝度
      switch (brightness_level) {
        case 0: return 9;
        case 1: return 18;
        case 2: return 60;
        case 3: return 130;
        default: return 0;
      }
    }
#endif

  public:
    SpecificInterface() = delete;
    SpecificInterface(uint8_t phase_shift, Color::type_t color, Mode::type_t mode, BrightnessLevel::type_t brightness_level)
      : color(color)
      , mode(mode)
      , brightness_level(brightness_level)
      , sword_cnt(phase_shift)
    { }
    ~SpecificInterface() = default;

  protected:
    void* identifier() const override { return (void *) IdValue; }
    void begin() override { }
    void end() override { }

    color_t update() override
    {
      const uint8_t local_sword_cnt = sword_cnt;

      // V 輝度
      uint8_t V;
      bool forceV = true;
      {
        V = GetBrightness(brightness_level); //基本光量

        if (mode == Mode::Twinkle){
          //輝度少し弄る
          const uint8_t decrease = GetBrightnessDecrease(brightness_level);
          V -= (uint8_t)((uint16_t)local_sword_cnt * decrease / 256);
          forceV = true;
        }

        // 輝度反映
        V = (V * this->brightness) / 256;
      }

      // H 色相
      uint16_t H;
      {
        switch (color) {
        case Color::FireRed:
        {
          // local_sword_cnt = [0, 256) -> H = [1450, 10200) の折り返し
          constexpr uint16_t s_min = 1450;
          constexpr uint16_t s_max = 10200;
          constexpr uint8_t threshold = 128;
          if (local_sword_cnt < threshold) {
            H = map_ui8toui16(local_sword_cnt, 0, threshold, s_min, s_max);
          }else{
            H = map_ui8toui16(local_sword_cnt, threshold, 255, s_max, s_min);
          }
          break;
        }
        case Color::SkyBlue:
        {
          // local_sword_cnt = [0, 256) -> H = [25500, 46400) の折り返し
          constexpr uint16_t s_min = 25500;
          constexpr uint16_t s_max = 46400;
          constexpr uint8_t threshold = 128;
          if (local_sword_cnt < threshold) {
            H = map_ui8toui16(local_sword_cnt, 0, threshold, s_max, s_min);
          }else{
            H = map_ui8toui16(local_sword_cnt, threshold, 255, s_min, s_max);
          }
          break;
        }
        }
      }

      //彩度
      uint8_t S;
      {
        #if 0
        // Note: 元コードの計算式に寄せた実装
        constexpr uint8_t s_min = 212;
        constexpr uint8_t s_max = 255;
        constexpr uint8_t threshold = 150;
        if (local_sword_cnt < threshold) {
          S = map_ui8toui8(local_sword_cnt, 0, threshold, s_max, s_min);
        }else{
          S = local_sword_cnt;
        }
        #else
        // 自然な実装
        constexpr uint8_t s_min = 150;
        constexpr uint8_t s_max = 255;
        constexpr uint8_t threshold = 150;
        if (local_sword_cnt < threshold) {
          S = map_ui8toui8(local_sword_cnt, 0, threshold, s_max, s_min);
        }else{
          S = map_ui8toui8(local_sword_cnt, threshold, 255, s_min, s_max);
        }
        #endif
      }

      // カウンタ更新
      {
        ++sword_cnt;
      }

      return color_t(H, S, V, false, false, forceV);
    }

  public:
    // 剣の色を取得
    Color::type_t get_color() const { return color; }
    // 剣の色を設定
    void set_color(Color::type_t v) { color = v; }

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

  private:
    // 初期化で与える値
    Color::type_t color : Color::RequireBits; // 剣の色
    Mode::type_t mode : Mode::RequireBits; // 発光モード
    BrightnessLevel::type_t brightness_level : BrightnessLevel::RequireBits; // 明るさレベル

    // 自動で更新する値
    uint8_t sword_cnt; // カウンタ [0, 256)
  };

  // LED調光モジュールクラス用インターフェース
  template<class T_MoePCB_Params>
  class ModuleInterface : public MoePCB_LEDColorCalculator_ModuleInterfaceBase<T_MoePCB_Params>
  {
  public:
    void set_mode_sword(typename T_MoePCB_Params::LED::led_index_t led_idx, uint8_t phase_shift, Color::type_t color, Mode::type_t mode = Mode::Default, BrightnessLevel::type_t brightness_level = 1) {
      void * const p = this->allocateInterface(led_idx);
      new(p) SpecificInterface(phase_shift, color, mode, brightness_level);
    }

    SpecificInterface* get_interface_sword(typename T_MoePCB_Params::LED::led_index_t led_idx) {
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
