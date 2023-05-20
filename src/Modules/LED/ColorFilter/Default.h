/*!
 * Modules/LED/ColorFilter/Default.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorFilter_Default_h
#define Modules_LED_ColorFilter_Default_h

// TODO: このインクルード削除したい
#include <Adafruit_NeoPixel.h>

// LED調光フィルタクラス (デフォルト)
// 基本は輝度、彩度の変化をイージングして反映する
// キラキラ変化を要求された場合はイージングせず指定値を直接反映する
template<class T_MoePCB_Params, class T_BundledColorCalculator>
class MoePCB_LEDColorFilter_Default
{
  template<template<class U_LEDParams, class U_LEDColorCalculator> class T_LEDColorFilter, class... T_LEDColorCalculators>
  friend class MoePCB_LED_Module;

protected:
  typedef typename T_BundledColorCalculator::color_raw_t color_raw_t;
  typedef typename T_BundledColorCalculator::color_t color_t;

  typedef typename T_MoePCB_Params::LED::led_index_t led_index_t;
  static constexpr led_index_t length = T_MoePCB_Params::LED::Length;

protected:
  MoePCB_LEDColorFilter_Default()
    : last_color()
    , flag(false)
  {}
  ~MoePCB_LEDColorFilter_Default() = default;

protected:
  // 32-bit packed WRGB value で返す
  uint32_t filter(led_index_t led_idx, color_t color) {
    if (flag) {
      if (color.forceH) {
        last_color[led_idx].H = color.H;
      } else {
        // color.H = (uint8_t)(last[led_idx*3+0] = step<float, 4>(last[led_idx*3+0], color.H));
      }

      if (color.forceS) {
        last_color[led_idx].S = color.S;
      } else {
        const auto next = step<uint8_t, 10>(last_color[led_idx].S, color.S);
        last_color[led_idx].S = next;
        color.S = next;
      }

      if (color.forceV) {
        last_color[led_idx].V = color.V;
      } else {
        const auto next = step<uint8_t, 10>(last_color[led_idx].V, color.V);
        last_color[led_idx].V = next;
        color.V = next;
      }
    }
    // TODO: ここで Adafruit_NeoPixel 直接指定するのはいまいち
    return Adafruit_NeoPixel::ColorHSV(color.H, color.S, color.V);
  }

  // フレームごとの更新処理
  void update() { /* Nothing To Do */ }

private:
  // 現在値と目標値からこのフレームでの値を決定する
  template<typename T, size_t N>
  T step(T current, T target) {
    if (current == target) {
      return current;
    } else if (current > target) {
      const T diff = (current - target) / N + 1;
      return current - diff;
    } else {
      const T diff = (target - current) / N + 1;
      return current + diff;
    }
  }

public:
  // デフォルトLED調光フィルタの有効状態を設定する
  void setEnabled(bool v) { flag = v; }
  // デフォルトLED調光フィルタの有効状態を取得する
  bool isEnabled(void) const { return flag; }

private:
  color_raw_t last_color[length];
  bool flag;
};

#endif
