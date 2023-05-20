/*!
 * Modules/LED/ColorCalculator/Breath.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorCalculator_Breath_h
#define Modules_LED_ColorCalculator_Breath_h

#include "Base.h"
#include <new>

// ゆっくり呼吸するような白色点滅
class MoePCB_LEDColorCalculator_Breath
{
private:
  // クラスを識別するための固有の定数
  // TODO: 適当な関数の配置先アドレス等コンパイル時に一意に定まる値を流用したい (コンパイラの警告に引っかかるため出来ていない)
  static constexpr uintptr_t IdValue = 0x20;

public:
  // LED調光モジュール個別インターフェース
  class SpecificInterface : public MoePCB_LEDColorCalculator_SpecificInterfaceBase
  {
  public:
    SpecificInterface()
      : breath_cnt(0)
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
        //イージングはなし
        if (breath_cnt < 128) {
          V = breath_cnt * 2;
        }
        else {
          V = (255 - breath_cnt) * 2;
        }

        // 輝度反映
        V = (V * this->brightness) / 256;
      }

      // H 色相
      uint16_t H;
      {
        // 色環は関係なし
        H = 0;
      }

      // S 彩度
      uint8_t S;
      {
        //彩度ゼロ
        S = 0;
      }

      // カウンタ
      {
        ++breath_cnt;
      }

      return color_t(H, S, V, false, false, true);
    }

  private:
    // 自動で更新する値
    uint8_t breath_cnt; // カウンタ
  };

  // LED調光モジュールクラス用インターフェース
  template<class T_MoePCB_Params>
  class ModuleInterface : public MoePCB_LEDColorCalculator_ModuleInterfaceBase<T_MoePCB_Params>
  {
  public:
    // 調光モードを Breath に設定する
    void set_mode_breath(typename T_MoePCB_Params::LED::led_index_t led_idx) {
      void * const p = this->allocateInterface(led_idx);
      new(p) SpecificInterface();
    }

    // 調光モード Breath の調光モジュールインターフェースを取得する
    // 調光モードが Breath 以外の場合は nullptr が返る
    SpecificInterface* get_interface_breath(typename T_MoePCB_Params::LED::led_index_t led_idx) {
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
