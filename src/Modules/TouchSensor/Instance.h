/*!
 * Modules/TouchSensor/Instance.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_TouchSensor_Instance_h
#define Modules_TouchSensor_Instance_h

#include <Arduino.h>

// タッチセンサ 1つ
class MoePCB_TouchSensor
{
public:
  class State {
  public:
    typedef uint8_t type_t;

  public:
    static constexpr type_t Mask_Pressing = (1 << (sizeof(type_t) * 8 - 2)) - 1;
    static constexpr type_t Freezing = 1 << (sizeof(type_t) * 8 - 2);
    static constexpr type_t On_Release = 1 << (sizeof(type_t) * 8 - 1);
  };


private:
  // オフセット決定時、センサ値読み取り時のサンプリング数
  static constexpr int Init_Sampling = 500;
  static constexpr int Update_Sampling = 20;

  // スレッショルド
  // (センサ値 - オフセット) > スレッショルドの場合タッチされたと判定する
  static constexpr int Threshold = 50;

public:
  MoePCB_TouchSensor() = delete;
  MoePCB_TouchSensor(uint8_t port)
    : offset(0)
    , state(0)
    , port(port)
  { }
  MoePCB_TouchSensor(const MoePCB_TouchSensor&) = delete;
  MoePCB_TouchSensor(MoePCB_TouchSensor&&) = default;
  MoePCB_TouchSensor& operator= (const MoePCB_TouchSensor&) = delete;
  MoePCB_TouchSensor& operator= (MoePCB_TouchSensor&&) = delete;
  ~MoePCB_TouchSensor() = default;

  // ステータス初期化
  void begin();

  // ステータス更新
  void update();

  // 次に押し上げされるまで更新を止める
  inline void freezeWhilePressing() { this->state |= State::Freezing; }

  // 現在のタッチ状態を取得する
  inline State::type_t getCurrentState() { return this->state; }

  // 現在タッチされているかを返す
  inline bool isPressing() { return (this->state & State::Mask_Pressing) && !isOnRelease(); }
  // 長押ししたフレーム数を返す
  inline int getPressingCount() { return (int)(this->state & State::Mask_Pressing); }
  // 立ち上がりかを返す
  inline bool isOnTouch() { return (this->state & State::Mask_Pressing) == 1; }
  // 立ち下がりかを返す
  inline bool isOnRelease() { return this->state & State::On_Release; }

private:
  int offset;
  State::type_t state;
  uint8_t port;
};

#endif
