/*!
 * Modules/LED/ColorCalculator/Simple.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorCalculator_Simple_h
#define Modules_LED_ColorCalculator_Simple_h

#include "Base.h"
#include <new>

// 指定色をそのまま返す
class MoePCB_LEDColorCalculator_Simple
{
private:
  // クラスを識別するための固有の定数
  // TODO: 適当な関数の配置先アドレス等コンパイル時に一意に定まる値を流用したい (コンパイラの警告に引っかかるため出来ていない)
  static constexpr uintptr_t IdValue = 0x60;

public:
  // LED調光モジュール個別インターフェース
  class SpecificInterface : public MoePCB_LEDColorCalculator_SpecificInterfaceBase
  {
  public:
    SpecificInterface() = default;
    ~SpecificInterface() = default;

  protected:
    void* identifier() const override { return (void *) IdValue; }
    void begin() override { }
    void end() override { }

    color_t update() override
    {
      return color_t(raw);
    }

  public:
    // 色を取得
    color_raw_t get_color() const { return raw; }
    // 色を設定
    void set_color(color_raw_t v) { raw = v; }

    // 色 (色相) を取得
    color_raw::hue_t get_color_hue() const { return raw.H; }
    // 色 (色相) を設定
    void set_color_hue(color_raw::hue_t v) { raw.H = v; }
    // 色 (彩度) を取得
    color_raw::sat_t get_color_sat() const { return raw.S; }
    // 色 (彩度) を設定
    void set_color_sat(color_raw::sat_t v) { raw.S = v; }
    // 色 (輝度) を取得
    color_raw::val_t get_color_val() const { return raw.V; }
    // 色 (輝度) を設定
    void set_color_val(color_raw::val_t v) { raw.V = v; }

  private:
    color_raw_t raw;
  };

  // LED調光モジュールクラス用インターフェース
  template<class T_MoePCB_Params>
  class ModuleInterface : public MoePCB_LEDColorCalculator_ModuleInterfaceBase<T_MoePCB_Params>
  {
  public:
    // 調光モードを Simple に設定する
    void set_mode_simple(typename T_MoePCB_Params::LED::led_index_t led_idx) {
      void * const p = this->allocateInterface(led_idx);
      new(p) SpecificInterface();
    }

    // 調光モード Simple の調光モジュールインターフェースを取得する
    // 調光モードが Simple 以外の場合は nullptr が返る
    SpecificInterface* get_interface_simple(typename T_MoePCB_Params::LED::led_index_t led_idx) {
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
