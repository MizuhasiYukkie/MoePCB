/*!
 * Modules/LED/ColorCalculator/Base.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorCalculator_Base_h
#define Modules_LED_ColorCalculator_Base_h

// LED調光モジュール個別インターフェースの基底
// 毎フレーム色情報を計算する
class MoePCB_LEDColorCalculator_SpecificInterfaceBase
{
  template<template<class U_LEDParams, class U_LEDColorCalculator> class T_LEDColorFilter, class... T_LEDColorCalculators>
  friend class MoePCB_LED_Module;

public:
  // 色情報 (HSV形式) のみをもつ
  struct color_raw {
  public:
    typedef uint16_t hue_t;
    typedef uint8_t sat_t;
    typedef uint8_t val_t;

  public:
    color_raw()
      : color_raw(0, 0, 0)
    { }
    color_raw(hue_t H, sat_t S, val_t V)
      : H(H), S(S), V(V)
    { }
    ~color_raw() = default;
    color_raw(const color_raw&) = default;
    color_raw(color_raw&&) = default;
    color_raw& operator=(const color_raw&) = default;
    color_raw& operator=(color_raw&&) = default;

  public:
    hue_t H;
    sat_t S;
    val_t V;
  };

  // 色情報 (HSV形式) と調光情報をもつ
  struct color : public color_raw {
  public:
    color()
      : color(0, 0, 0)
    { }
    color(uint16_t H, uint8_t S, uint8_t V)
      : color(H, S, V, false, false, false)
    { }
    color(const color_raw& raw)
      : color_raw(raw), forceH(true), forceS(true), forceV(true)
    { }
    color(color_raw&& raw)
      : color_raw(raw), forceH(true), forceS(true), forceV(true)
    { }
    color(hue_t H, sat_t S, val_t V, bool forceH, bool forceS, bool forceV)
      : color_raw(H, S, V), forceH(forceH), forceS(forceS), forceV(forceV)
    { }
    ~color() = default;
    color(const color&) = default;
    color(color&&) = default;
    color& operator=(const color&) = default;
    color& operator=(color&&) = default;

  public:
    bool forceH : 1;
    bool forceS : 1;
    bool forceV : 1;
  };

public:
  typedef color_raw color_raw_t;
  typedef color color_t;

public:
  virtual ~MoePCB_LEDColorCalculator_SpecificInterfaceBase() = default;
  virtual void* identifier() const = 0;

protected:
  virtual void begin() = 0;
  virtual void end() = 0;
  virtual color_t update() = 0;

public:
  void setBrightness(uint8_t brightness) { this->brightness = brightness; }
  uint8_t get_brightness() const { return this->brightness; }

protected:
  uint8_t brightness;
};

// LED調光モジュールクラス用インターフェースの基底
// LED調光モジュールクラスにはライブラリ内部向けに、
// 調光モジュールのアロケート、アドレス取得処理を実装する
template<class T_MoePCB_Params>
class MoePCB_LEDColorCalculator_ModuleInterfaceBase
{
public:
  typedef typename T_MoePCB_Params::LED::led_index_t led_index_t;

protected:
  // アロケートのみ行いアドレスを返す
  virtual void* allocateInterface(led_index_t) = 0;
  // アドレス取得のみ行う
  virtual MoePCB_LEDColorCalculator_SpecificInterfaceBase* getInterface(led_index_t) = 0;
};

#endif
