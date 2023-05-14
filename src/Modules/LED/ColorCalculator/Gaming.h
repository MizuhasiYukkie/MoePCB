/*!
 * Modules/LED/ColorCalculator/Gaming.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorCalculator_Gaming_h
#define Modules_LED_ColorCalculator_Gaming_h

#include "Base.h"
#include <new>

// ゲーミングモード
class MoePCB_LEDColorCalculator_Gaming
{
private:
  // クラスを識別するための固有の定数
  // TODO: 適当な関数の配置先アドレス等コンパイル時に一意に定まる値を流用したい (コンパイラの警告に引っかかるため出来ていない)
  static constexpr uintptr_t IdValue = 0x30;

public:
  // 明るさレベル
  class BrightnessLevel {
    friend class MoePCB_LEDColorCalculator_Gaming;

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
    //明るさレベルに応じた輝度
#if __cpp_constexpr >= 201304
    // C++14以降向け (こちらの実装でもビルド、実行できるが警告が出る)
    // cite: https://cpprefjp.github.io/lang/cpp14/relaxing_constraints_on_constexpr.html
    static inline constexpr uint8_t BrightnessTable[4] = { 25, 65, 175, 250 }; // 通常輝度
    constexpr uint8_t GetBrightness(BrightnessLevel::type_t brightness_level) { return BrightnessTable[brightness_level]; }
#else
    constexpr uint8_t GetBrightness(BrightnessLevel::type_t brightness_level) {
      // 基本光量より１段明るい
      switch (brightness_level) {
        case 0: return 25;
        case 1: return 65;
        case 2: return 175;
        case 3: return 250;
        default: return 0;
      }
    }
#endif

  public:
    SpecificInterface() = delete;
    SpecificInterface(uint8_t phase_shift, uint8_t step, BrightnessLevel::type_t brightness_level)
      : brightness_level(brightness_level)
      , gaming_cnt(phase_shift)
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
      {
        V = GetBrightness(brightness_level); // 基本光量より１段明るい

        // 輝度反映
        V = (V * this->brightness) / 256;
      }

      // H 色相
      uint16_t H;
      {
        // [0, 256) -> [0, 65536) の範囲変換なので、256倍するだけでよい
        H = (uint16_t)gaming_cnt * 256;
      }

      // S 彩度
      uint8_t S;
      {
        // 最大値に固定
        S = 255;
      }

      // ゲーミング用カウンタ 0-255で一周
      {
        // オーバーフローすることで勝手に次の周に進む
        gaming_cnt += step;
      }

      return color_t(H, S, V);
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
    uint8_t get_step() const { return step; }
    // フレーム当たりの色相変化量を設定
    void set_step(uint8_t v) { step = v; }

  private:
    // 初期化で与える値
    BrightnessLevel::type_t brightness_level : BrightnessLevel::RequireBits; // 明るさレベル

    // 自動で更新する値
    uint8_t gaming_cnt; // カウンタ
    uint8_t step; // 1フレームあたりのカウンタ増分
  };

  // LED調光モジュールクラス用インターフェース
  template<class T_MoePCB_Params>
  class ModuleInterface : public MoePCB_LEDColorCalculator_ModuleInterfaceBase<T_MoePCB_Params>
  {
  public:
    // 調光モードを Gaming に設定する
    void set_mode_gaming(typename T_MoePCB_Params::LED::led_index_t led_idx, uint8_t phase_shift, uint8_t step = 6, BrightnessLevel::type_t brightness_level = 1) {
      void * const p = this->allocateInterface(led_idx);
      new(p) SpecificInterface(phase_shift, step, brightness_level);
    }

    // 調光モード Gaming の調光モジュールインターフェースを取得する
    // 調光モードが Gaming 以外の場合は nullptr が返る
    SpecificInterface* get_interface_gaming(typename T_MoePCB_Params::LED::led_index_t led_idx) {
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
