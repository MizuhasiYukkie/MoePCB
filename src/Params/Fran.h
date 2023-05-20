/*!
 * Params/Fran.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Params_Fran_h
#define Params_Fran_h

#include <stdint.h>
#include <Arduino.h>

// フランの萌基板パラメータ
class MoePCB_Params_Fran
{
public:
  class LED {
  public:
    typedef uint8_t led_index_t;

  public:
    class Placement
    {
    public:
      static constexpr led_index_t Heart      = 0; // LED1 : 胸 (背面)
      static constexpr led_index_t OuterLeft  = 3; // LED4 : 最左
      static constexpr led_index_t Left       = 2; // LED3 : 中左
      static constexpr led_index_t InnerLeft  = 1; // LED2 : 内左
      static constexpr led_index_t InnerRight = 4; // LED5 : 内右
      static constexpr led_index_t Right      = 5; // LED6 : 中右
      static constexpr led_index_t OuterRight = 6; // LED7 : 最右
    };

  public:
    static constexpr led_index_t Length = 7;
    static constexpr int16_t Port = 6;
  };

  class SubLED {
  public:
    typedef uint8_t pin_t;

  public:
    static constexpr pin_t Pin = 13;
  };

  class Thermometer {

  };

  class TouchSensor {
  public:
    typedef uint8_t touch_sensor_port_t;
    typedef uint8_t touch_sensor_index_t;

  public:
    // 配置場所 -> インデックス
    class Placement
    {
    public:
      static constexpr touch_sensor_index_t Hair  = 0; // タッチセンサ1 : 髪
      static constexpr touch_sensor_index_t Chest = 1; // タッチセンサ2 : 胸
      static constexpr touch_sensor_index_t Skirt = 2; // タッチセンサ3 : スカート
    };

    // インデックス -> 接続先ポート
    class Port
    {
    public:
      static constexpr touch_sensor_port_t Hair  = A0; // タッチセンサ1 : 髪
      static constexpr touch_sensor_port_t Chest = A1; // タッチセンサ2 : 胸
      static constexpr touch_sensor_port_t Skirt = A2; // タッチセンサ3 : スカート

    public:
      static constexpr touch_sensor_port_t Get(touch_sensor_index_t idx) {
        return idx == 0 ? Hair : (idx == 1 ? Chest : (idx == 2 ? Skirt : 0));
      }
    };

  public:
    static constexpr touch_sensor_index_t Length = 3;
  };
};

#endif
