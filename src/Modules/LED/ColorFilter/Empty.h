/*!
 * Modules/LED/ColorFilter/Empty.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorFilter_Empty_h
#define Modules_LED_ColorFilter_Empty_h

// TODO: このインクルード削除したい
#include <Adafruit_NeoPixel.h>

// LED調光フィルタクラス (無)
// 入力値に対する処理を一切行わない
template<class T_MoePCB_Params, class T_BundledColorCalculator>
class MoePCB_LEDColorFilter_Empty
{
  template<template<class U_LEDParams, class U_LEDColorCalculator> class T_LEDColorFilter, class... T_LEDColorCalculators>
  friend class MoePCB_LED_Module;

protected:
  typedef typename T_BundledColorCalculator::color_t color_t;

protected:
  // 32-bit packed WRGB value で返す
  uint32_t filter(uint8_t led_idx, color_t color) {
    // 何もフィルタせずそのまま変換する
    // TODO: ここで Adafruit_NeoPixel 直接指定するのはいまいち
    return Adafruit_NeoPixel::ColorHSV(color.H, color.S, color.V);
  }

  // フレームごとの更新処理
  void update() { /* Nothing To Do */ }
};

#endif
