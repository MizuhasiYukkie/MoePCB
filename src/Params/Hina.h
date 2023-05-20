/*!
 * Params/Hina.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Params_Hina_h
#define Params_Hina_h

#include <stdint.h>
#include <Arduino.h>

// 雛の萌基板パラメータ
class MoePCB_Params_Hina
{
public:
  class LED {
  public:
    typedef uint8_t led_index_t;

  public:
    class Placement
    {
    public:
      static constexpr led_index_t Heart       = 0; // LED1  : 胸 (背面)
      static constexpr led_index_t Head        = 5; // LED6  : 頭
      static constexpr led_index_t UpperRight  = 6; // LED7  : 右上 (リボン上)
      static constexpr led_index_t Right       = 7; // LED8  : 右 (リボン上)
      static constexpr led_index_t UpperLeft   = 4; // LED5  : 左上 (手元)
      static constexpr led_index_t Left        = 3; // LED4  : 左 (リボン上)
      static constexpr led_index_t LowerLeft   = 2; // LED3  : 左下外 (スカート上)
      static constexpr led_index_t InnerLeft   = 1; // LED2  : 左下内 (スカート上)
      static constexpr led_index_t InnerRight  = 9; // LED10 : 右下内 (スカート上)
      static constexpr led_index_t LowerRight  = 8; // LED9  : 右下外 (スカート上)
    };

  public:
    static constexpr led_index_t Length = 10;
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
      static constexpr touch_sensor_index_t Hair   = 0; // タッチセンサ1 : 髪
      static constexpr touch_sensor_index_t Chest  = 1; // タッチセンサ2 : 胸
      static constexpr touch_sensor_index_t Skirt  = 2; // タッチセンサ3 : スカート
      static constexpr touch_sensor_index_t Ribbon = 3; // タッチセンサ4 : リボン
    };

    // インデックス -> 接続先ポート
    class Port
    {
    public:
      static constexpr touch_sensor_port_t Hair   = A1; // タッチセンサ1 : 髪
      static constexpr touch_sensor_port_t Chest  = A0; // タッチセンサ2 : 胸
      static constexpr touch_sensor_port_t Skirt  = A2; // タッチセンサ3 : スカート
      static constexpr touch_sensor_port_t Ribbon = A3; // タッチセンサ4 : リボン

    public:
      static constexpr touch_sensor_port_t Get(touch_sensor_index_t idx) {
        return idx == 0 ? Hair : (idx == 1 ? Chest : (idx == 2 ? Skirt : 0));
      }
    };

  public:
    static constexpr touch_sensor_index_t Length = 4;
  };
};

#endif
