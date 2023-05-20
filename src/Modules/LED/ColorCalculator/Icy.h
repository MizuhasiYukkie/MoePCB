/*!
 * Modules/LED/ColorCalculator/Icy.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorCalculator_Icy_h
#define Modules_LED_ColorCalculator_Icy_h

#include "Base.h"
#include <new>

// 白色点灯 + 寒色でランダム点滅
class MoePCB_LEDColorCalculator_Icy
{
private:
  // クラスを識別するための固有の定数
  // TODO: 適当な関数の配置先アドレス等コンパイル時に一意に定まる値を流用したい (コンパイラの警告に引っかかるため出来ていない)
  static constexpr uintptr_t IdValue = 0x40;

public:
  // 点灯モード
  // Natural : ナチュラルキラキラ
  // Digital : デジタル的
  class Mode
  {
    friend class MoePCB_LEDColorCalculator_Icy;

  public:
    typedef uint8_t type_t;

  public:
    static constexpr type_t Natural = 0; // A : ナチュラルキラキラ
    static constexpr type_t Digital = 1; // B : デジタル的

  private:
    static constexpr type_t RequireBits = 1; // 内部用 : 必要なビット幅
  };

  // 明るさレベル
  class BrightnessLevel {
    friend class MoePCB_LEDColorCalculator_Icy;

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

    // 明るさレベルに応じた輝度
#if __cpp_constexpr >= 201304
    // C++14以降向け (こちらの実装でもビルド、実行できるが警告が出る)
    // cite: https://cpprefjp.github.io/lang/cpp14/relaxing_constraints_on_constexpr.html
    static inline constexpr uint8_t BrightnessTable[4] = {  3, 10,  20,  30 }; // 通常輝度
    static inline constexpr uint8_t TwinkleTable[4]    = { 40, 65, 165, 255 }; // ランダムきらきら輝度
    constexpr uint8_t GetBrightness(BrightnessLevel::type_t brightness_level) { return BrightnessTable[brightness_level]; }
    constexpr uint8_t GetTwinkleBrightness(BrightnessLevel::type_t brightness_level) { return TwinkleTable[brightness_level]; }
#else
    constexpr uint8_t GetBrightness(BrightnessLevel::type_t brightness_level) {
      // 基本光量
      switch (brightness_level) {
        case 0: return 3;
        case 1: return 10;
        case 2: return 20;
        case 3: return 30;
        default: return 0;
      }
    }
    constexpr uint8_t GetTwinkleBrightness(BrightnessLevel::type_t brightness_level) {
      // ランダムきらきら輝度
      switch (brightness_level) {
        case 0: return 40;
        case 1: return 65;
        case 2: return 165;
        case 3: return 255;
        default: return 0;
      }
    }
#endif

  public:
    SpecificInterface() = delete;
    SpecificInterface(Mode::type_t mode, BrightnessLevel::type_t brightness_level)
      : mode(mode)
      , brightness_level(brightness_level)
      , twinkle_interval(random(0, twinkle_cooltime))
      , last_V(GetBrightness(brightness_level))
      , last_S(0)
      , last_H(random(25500, 45500)) // 水色〜青のランダム色
    { }
    ~SpecificInterface() = default;

  private:
    color_t update_sub(uint8_t rand_max, uint8_t V_default, uint8_t V_min, uint8_t V_max) {
      uint8_t  V = V_default; // 輝度
      uint16_t H = last_H;    // 色相
      uint8_t  S = 0;         // 彩度
      bool forceV = false;    // 輝度強制変更
      bool forceS = false;    // 彩度強制変更

      // 連続して同じLEDをピカピカしないように考慮
      if (twinkle_interval > 0) {
        --twinkle_interval;
      }
      else {
        if (random(0, rand_max) == 0) {
          V = random(V_min, V_max); //明るさを範囲でランダムで変更
          if (mode == Mode::Digital) { last_V = V; }
          if (random(0, 100) < 95) { // 95/100の確率で水色系のランダム色
            H = random(25500, 45500); // 水色〜青のランダム色
            // ランダムに彩度を決定
            // 元実装はフィルタ処理の最後にconstrainするまでfloat範囲なのでここで256以上を返せる
            // 新実装ではフィルタ処理の彩度減衰ペースを落とすことで見た目を近づける
            const auto s = random(210, 330);
            S = (s >= 256) ? 255 : ((uint8_t)s); // int 範囲から uint8_t 範囲へ縮小
          } else { // 5/100の確率で黄色
            H = 7650; // 稀に黄色
            S = 180;// 黄色の時は彩度少し抑える
          }

          twinkle_interval = twinkle_cooltime;
          last_S = S;
          last_H = H;
          forceV = true;
          forceS = true;
        }
      }

      if (mode == Mode::Digital) {
        V = last_V;
        S = last_S;
        forceV = true;
        forceS = true;
      }

      // 輝度反映
      V = (V * this->brightness) / 256;

      return color_t(H, S, V, false, forceS, forceV);
    }

  protected:
    void* identifier() const override { return (void *) IdValue; }
    void begin() override { }
    void end() override { }

    color_t update() override
    {
      switch (mode) {
        case Mode::Natural:
          // 点灯モード Natural : ランダムに輝度を変更
          return update_sub(200, GetBrightness(brightness_level), GetTwinkleBrightness(brightness_level), GetTwinkleBrightness(brightness_level));
        case Mode::Digital:
          // 点灯モード Digital : ランダムに輝度を変更
          return update_sub(50, GetBrightness(brightness_level), GetBrightness(brightness_level), GetTwinkleBrightness(brightness_level));
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

  private:
    // 初期化で与える値
    Mode::type_t mode : Mode::RequireBits; // 点灯モード
    BrightnessLevel::type_t brightness_level : BrightnessLevel::RequireBits; // 明るさレベル

    // 自動で更新する値
    twinkle_interval_t twinkle_interval;
    uint8_t last_V; // V 輝度
    uint8_t last_S; // S 彩度
    uint16_t last_H; // H 色相
  };

  // LED調光モジュールクラス用インターフェース
  template<class T_MoePCB_Params>
  class ModuleInterface : public MoePCB_LEDColorCalculator_ModuleInterfaceBase<T_MoePCB_Params>
  {
  public:
    // 調光モードを Icy に設定する
    void set_mode_icy(typename T_MoePCB_Params::LED::led_index_t led_idx, Mode::type_t mode = Mode::Natural, BrightnessLevel::type_t brightness_level = 1) {
      void * const p = this->allocateInterface(led_idx);
      new(p) SpecificInterface(mode, brightness_level);
    }

    // 調光モード Icy の調光モジュールインターフェースを取得する
    // 調光モードが Icy 以外の場合は nullptr が返る
    SpecificInterface* get_interface_icy(typename T_MoePCB_Params::LED::led_index_t led_idx) {
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
