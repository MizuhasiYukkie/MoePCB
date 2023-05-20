/*!
 * Modules/LED/Instance.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_Instance_h
#define Modules_LED_Instance_h

#include <Adafruit_NeoPixel.h>

// メインLED
// Adafruit_NeoPixel が並んでいる
// 配置先のピンが固定であることを前提に一部のメンバを削除し、残りは手を加えず再利用
template<typename T_MoePCB_Params>
class MoePCB_LED_Base
  : public Adafruit_NeoPixel
{
  template<template<class U_LEDParams, class U_LEDColorCalculator> class T_LEDColorFilter, class... T_LEDColorCalculators>
  friend class MoePCB_LED_Module;

public:
  typedef typename T_MoePCB_Params::LED::Placement placement_t;

protected:
  MoePCB_LED_Base()
    : Adafruit_NeoPixel(T_MoePCB_Params::LED::Length, T_MoePCB_Params::LED::Port)
  { }
  MoePCB_LED_Base(const MoePCB_LED_Base&) = delete;
  MoePCB_LED_Base(MoePCB_LED_Base&&) = delete;
  MoePCB_LED_Base& operator= (const MoePCB_LED_Base&) = delete;
  MoePCB_LED_Base& operator= (MoePCB_LED_Base&&) = delete;
  ~MoePCB_LED_Base() = default;

protected:
  void begin(void) { Adafruit_NeoPixel::begin(); }
  void updateLength(uint16_t n) = delete;
  void setPin(int16_t p) = delete;
  void updateType(neoPixelType t) = delete;
};

#endif
