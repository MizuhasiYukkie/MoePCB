/*!
 * Modules/LED/ColorFilter/FullSet.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_ColorFilter_FullSet_h
#define Modules_LED_ColorFilter_FullSet_h

// TODO: このインクルード削除したい
#include <Adafruit_NeoPixel.h>

// LED調光フィルタクラス (全部乗せ)
// 萌基板で実装されていたすべてのフィルタ処理を詰めたフィルタ
// TODO: 各フィルタをモジュール化して分割する
template<class T_MoePCB_Params, class T_BundledColorCalculator>
class MoePCB_LEDColorFilter_FullSet
{
  template<template<class U_LEDParams, class U_LEDColorCalculator> class T_LEDColorFilter, class... T_LEDColorCalculators>
  friend class MoePCB_LED_Module;

protected:
  typedef typename T_BundledColorCalculator::color_raw_t color_raw_t;
  typedef typename T_BundledColorCalculator::color_t color_t;

  typedef typename T_MoePCB_Params::LED::led_index_t led_index_t;
  static constexpr led_index_t length = T_MoePCB_Params::LED::Length;

protected:
  MoePCB_LEDColorFilter_FullSet()
    : last_color()
    , reset(true)
    , enable(false)
    , angry(false)
    , cold(false)
    , heat(false)
    , drunk(false)
    , counters{}
  {}
  ~MoePCB_LEDColorFilter_FullSet() = default;

protected:
  // 32-bit packed WRGB value で返す
  uint32_t filter(led_index_t led_idx, color_t color) {
    if (enable) {
      if (reset) {
        last_color[led_idx].H = color.H;
        last_color[led_idx].S = color.S;
        last_color[led_idx].V = color.V;
      }

      if (color.forceH) {
        last_color[led_idx].H = color.H;
      } else {
        uint16_t hue = color.H;

        // 怒りゲージによる色相上書き (約353° ≒ 仄かに紫がかった赤色)
        hue = morph(64250, hue, get_angry_counter());

        // 寒さゲージによる色相上書き (約190° ≒ 水色)
        hue = morph(34600, hue, get_cold_counter());

        // 暑さゲージによる色相上書き (0° ≒ 赤色)
        hue = morph(0, hue, get_heat_counter());

        // 酔いゲージによる色相上書き (約353° ≒ 仄かに紫がかった赤色)
        hue = morph(64250, hue, get_drunk_counter());

        color.H = hue;
      }

      {
        const auto last_S = (color.forceS)? color.S : last_color[led_idx].S;

        // 元実装は分母 10 だが、整数計算化し端数切り上げを導入したことで刻み幅が増大しているため、少し分母を大きくして調整
        auto next = step<uint8_t, 24>(last_S, color.S);

        // 怒り、寒さ、暑さ、酔いゲージにより彩度設定を無視して最大彩度になる
        next = max(next, get_angry_counter());
        next = max(next, get_cold_counter());
        next = max(next, get_heat_counter());
        next = max(next, get_drunk_counter());

        last_color[led_idx].S = next;
        color.S = next;
      }

      {
        const auto last_V = (color.forceV)? color.V : last_color[led_idx].V;

        // 元実装は分母 30 だが、整数計算化し端数切り上げを導入したことで刻み幅が増大しているため、少し分母を大きくして調整
        auto next = step<uint8_t, 40>(last_V, color.V);

        // 怒りゲージにより明るさ設定を無視して最大輝度になる
        next = max(next, get_angry_counter());
        if (get_angry_counter() > 250 && next >= get_angry_pulsation_counter()) {
          next -= get_angry_pulsation_counter();
        }

        last_color[led_idx].V = next;
        color.V = next;
      }
    }
    // TODO: ここで Adafruit_NeoPixel 直接指定するのはいまいち
    return Adafruit_NeoPixel::ColorHSV(color.H, color.S, color.V);
  }

  // フレームごとの更新処理
  void update() {
    if (enable) {
      reset = false;

      // 怒りゲージ更新
      if (angry) {
        if (get_angry_counter() < (256-6)) { get_angry_counter() += 6; }
        else if (get_angry_counter() < (256-1)) { get_angry_counter() += 1; }

        // 脈動ゲージ
        get_angry_pulsation_counter() += 8;
        if (get_angry_pulsation_counter() > 160) { get_angry_pulsation_counter() = 0; }
      } else {
        get_angry_pulsation_counter() = 0;
        if (get_angry_counter() >= 4) { get_angry_counter() -= 4; }
        else if (get_angry_counter() >= 1) { get_angry_counter() -= 1; }
      }

      // 寒さゲージ更新
      if (cold) {
        if (get_cold_counter() < (256-1)) { get_cold_counter() += 1; }
      }else{
        if (get_cold_counter() > 0) { get_cold_counter() -= 1; }
      }

      // 暑さゲージ更新
      if (heat) {
        if (get_heat_counter() < (256-1)) { get_heat_counter() += 1; }
      }else{
        if (get_heat_counter() > 0) { get_heat_counter() -= 1; }
      }

      // 酔いゲージ更新
      if (drunk) {
        if (get_drunk_counter() < (256-1)) { get_drunk_counter() += 1; }
      }else{
        if (get_drunk_counter() > 0) { get_drunk_counter() -= 1; }
      }
    }
  }

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

  // モーフィング関数
  // 最終地点の色環に近い方に回す
  // 現在値と目標値があまりに近いと計算結果がずれる
  uint16_t morph(uint16_t target, uint16_t value, uint8_t gauge){
    if (gauge == 0) {
      // ゲージがゼロなら入力そのまま返す
      return value;
    }
    if (gauge == 255) {
      // ゲージが最大なら目標値をそのまま返す
      return target;
    }

    if (target - value < 32768) {
      // 加算して近づける
      const uint16_t diff = target - value;
      if (diff >= 256) {
        // 本当は255で割るのが正解だが計算速度をとって256で割る
        return value + diff / 256 * gauge;
      } else {
        // 差が小さいと乗算前に端数が切り捨てられて値が変化しないので、刻み幅16に変える
        // diff < 16 だとやはりうまくいかなくなるが、その時はもはや目視できないだろうということで目をつぶる
        return value + diff / 16 * (gauge / 16);
      }
    } else {
      // 減算して近づける
      const uint16_t diff = value - target;
      if (diff >= 256) {
        // 本当は255で割るのが正解だが計算速度をとって256で割る
        return value - diff / 256 * gauge;
      } else {
        // 差が小さいと乗算前に端数が切り捨てられて値が変化しないので、刻み幅16に変える
        // diff < 16 だとやはりうまくいかなくなるが、その時はもはや目視できないだろうということで目をつぶる
        return value - diff / 16 * (gauge / 16);
      }
    }
  }

public:
  // 直前の色をリセットする
  void resetLastColor() { reset = true; }

  // 全部乗せLED調光フィルタの有効状態を設定する
  void setEnabled(bool v) { enable = v; }
  // 全部乗せLED調光フィルタの有効状態を取得する
  bool isEnabled(void) const { return enable; }

  // 全部乗せLED調光フィルタの怒りモードを設定する
  void set_state_angry(bool v) { angry = v; }
  // 全部乗せLED調光フィルタの怒りモードを取得する
  bool get_state_angry(void) const { return angry; }

  // 全部乗せLED調光フィルタの寒さモードを設定する
  void set_state_cold(bool v) { cold = v; }
  // 全部乗せLED調光フィルタの寒さモードを取得する
  bool get_state_cold(void) const { return cold; }

  // 全部乗せLED調光フィルタの暑さモードを設定する
  void set_state_heat(bool v) { heat = v; }
  // 全部乗せLED調光フィルタの暑さモードを取得する
  bool get_state_heat(void) const { return heat; }

  // 全部乗せLED調光フィルタの酔いモードを設定する
  void set_state_drunk(bool v) { drunk = v; }
  // 全部乗せLED調光フィルタの酔いモードを取得する
  bool get_state_drunk(void) const { return drunk; }

private:
  // 怒りモードカウンタ
  uint8_t& get_angry_counter() { return counters[0]; }
  // 怒りモード脈動カウンタ
  uint8_t& get_angry_pulsation_counter() { return counters[1]; }

  // 寒さモードカウンタ
  uint8_t& get_cold_counter() { return counters[2]; }

  // 暑さモードカウンタ
  uint8_t& get_heat_counter() { return counters[3]; }

  // 酔いモードカウンタ
  uint8_t& get_drunk_counter() { return counters[4]; }

private:
  color_raw_t last_color[length];
  bool reset : 1;
  bool enable : 1;
  bool angry : 1;
  bool cold : 1;
  bool heat : 1;
  bool drunk : 1;
  uint8_t counters[5];
};

#endif
