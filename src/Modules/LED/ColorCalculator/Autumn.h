/*!
 * Modules/LED/ColorCalculator/Autumn.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorCalculator_Autumn_h
#define Modules_LED_ColorCalculator_Autumn_h

#include "Base.h"
#include <new>

// 暖色でゲーミング + たまにキラキラ
class MoePCB_LEDColorCalculator_Autumn
{
private:
  // クラスを識別するための固有の定数
  // TODO: 適当な関数の配置先アドレス等コンパイル時に一意に定まる値を流用したい (コンパイラの警告に引っかかるため出来ていない)
  static constexpr uintptr_t IdValue = 0x10;

public:
  // 明るさレベル
  class BrightnessLevel {
    friend class MoePCB_LEDColorCalculator_Autumn;

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
    static inline constexpr uint8_t BrightnessTable[4] = { 10, 25,  65, 175 }; // 通常輝度
    static inline constexpr uint8_t TwinkleTable[4]    = { 40, 65, 155, 255 }; // ランダムきらきら輝度
    constexpr uint8_t GetBrightness(BrightnessLevel::type_t brightness_level) { return BrightnessTable[brightness_level]; }
    constexpr uint8_t GetTwinkleBrightness(BrightnessLevel::type_t brightness_level) { return TwinkleTable[brightness_level]; }
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
#endif

  public:
    SpecificInterface() = delete;
    SpecificInterface(int phase_shift, float step, BrightnessLevel::type_t brightness_level)
      : brightness_level(brightness_level)
      , twinkle_interval(random(0, twinkle_cooltime))
      , autumn_cnt(phase_shift)
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
        V = GetBrightness(brightness_level); // 基本光量

        // 連続して同じLEDをピカピカしないように考慮
        if (twinkle_interval > 0) {
          --twinkle_interval;
        }
        else {
          if(random(0, 250) == 0){ //ランダムで明るくする
            V = GetTwinkleBrightness(brightness_level); //キラッと光量
            forceV = true;
          }
        }

        // 輝度反映
        V = (V * this->brightness) / 256;
      }

      // H 色相
      uint16_t H;
      {
        // [-120, 120) の autumn_cnt を [120, 240] で折り返す値 hue に変換
        float hue;
        if (autumn_cnt < 0) {
          hue = -autumn_cnt;
        } else {
          hue = autumn_cnt;
        }

        // autumn_cnt 更新
        autumn_cnt += step;
        if (autumn_cnt >= 120.0f) {
          autumn_cnt -= 240.0f;
        }
        if (autumn_cnt < -120.0f) {
          autumn_cnt += 240.0f;
        }

        // 色相計算
        H = (uint16_t) (hue * (float)(65535.0 / 360.0));
      }

      // S 彩度
      uint8_t S;
      {
        S = 255; // 彩度MAX
      }

      return color_t(H, S, V, false, false, forceV);
    }

  public:
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
    BrightnessLevel::type_t brightness_level : BrightnessLevel::RequireBits; // 明るさレベル

    // 自動で更新する値
    twinkle_interval_t twinkle_interval;
    float autumn_cnt; // カウンタ　[-30, 30)
    float step; // 1フレームで色相環を回す角度
  };

  // LED調光モジュールクラス用インターフェース
  template<class T_MoePCB_Params>
  class ModuleInterface : public MoePCB_LEDColorCalculator_ModuleInterfaceBase<T_MoePCB_Params>
  {
  public:
    // 調光モードを Autumn に設定する
    void set_mode_autumn(typename T_MoePCB_Params::LED::led_index_t led_idx, int phase_shift, float step = 1.0f, BrightnessLevel::type_t brightness_level = 1) {
      void * const p = this->allocateInterface(led_idx);
      new(p) SpecificInterface(phase_shift, step, brightness_level);
    }

    // 調光モード Autumn の調光モジュールインターフェースを取得する
    // 調光モードが Autumn 以外の場合は nullptr が返る
    SpecificInterface* get_interface_autumn(typename T_MoePCB_Params::LED::led_index_t led_idx) {
      auto const pBase = this->getInterface(led_idx);
      if (pBase == nullptr) { return nullptr; }
      if (pBase->identifier() == (void *) IdValue) {
        return reinterpret_cast<SpecificInterface *>(pBase);
      } else {
        return nullptr;
      }
    }
  };
};

#endif
