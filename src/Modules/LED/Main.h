/*!
 * Modules/LED/Main.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_LED_Main_h
#define Modules_LED_Main_h

#include "Instance.h"

#include "ColorCalculator/Base.h"
#include "ColorCalculator/Simple.h"
#include "ColorCalculator/Gaming.h"
#include "ColorCalculator/Rainbow.h"
#include "ColorCalculator/Breath.h"
#include "ColorCalculator/Icy.h"
#include "ColorCalculator/Autumn.h"
#include "ColorCalculator/Sword.h"

#include "ColorFilter/Empty.h"
#include "ColorFilter/Default.h"
#include "ColorFilter/FullSet.h"

template<template<class U_LEDParams, class U_LEDColorCalculator> class T_LEDColorFilter, class... T_LEDColorCalculators>
class MoePCB_LED_Module {
public:
  template<class T_MoePCB_Params>
  class Module;

private:
  template<class T_MoePCB_Params>
  class InnerClass;

private:
  // 1つ以上の ColorCalculator を1つに集約したクラス
  template<class T_MoePCB_Params, class T_ColorCalculator_Head, class... T_ColorCalculator_Tail>
  class BundledColorCalculator
    : public T_ColorCalculator_Head::template ModuleInterface<T_MoePCB_Params>
    , public T_ColorCalculator_Tail::template ModuleInterface<T_MoePCB_Params>...
  {
    friend InnerClass<T_MoePCB_Params>;

  private:
    // クラス内専用計算関数隠蔽クラス
    class Util {
    public:
      // 最大値を求める (引数長 0 のとき)
      static constexpr size_t get_max(void) { return 0; }

      // 最大値を求める (引数長 1 以上のとき)
      template <class T, class... U>
      static constexpr size_t get_max(T a, U... b)
      {
        return (a > get_max(b...)) ? a : get_max(b...);
      }
    };

  private:
    // TODO: T_ColorCalculator_Head, T_ColorCalculator_Tail がすべて同じ型を利用している保証をする
    typedef typename T_MoePCB_Params::LED::led_index_t led_index_t;
  public:
    typedef typename T_ColorCalculator_Head::SpecificInterface::color_raw color_raw_t;
    typedef typename T_ColorCalculator_Head::SpecificInterface::color color_t;

  private:
    // T_ColorCalculator_Tail で渡された派生クラスのそれぞれのサイズのうち、最も大きい値
    static constexpr size_t max_base_size = Util::get_max(
      sizeof(typename T_ColorCalculator_Head::SpecificInterface),
      sizeof(typename T_ColorCalculator_Tail::SpecificInterface)...);

    // 搭載しているLED個数
    static constexpr led_index_t length = T_MoePCB_Params::LED::Length;

    // アロケート済みフラグ配列のサイズ
    static constexpr size_t allocated_flag_array_length = (length + (8 - 1)) / 8;

  protected:
    BundledColorCalculator() = default;
    BundledColorCalculator(const BundledColorCalculator&) = delete;
    BundledColorCalculator(BundledColorCalculator&&) = delete;
    BundledColorCalculator& operator= (const BundledColorCalculator&) = delete;
    BundledColorCalculator& operator= (BundledColorCalculator&&) = delete;
    ~BundledColorCalculator() = default;

  protected:
    void begin() {
      for (uint8_t i = 0; i < allocated_flag_array_length; ++i) {
        interfaces_allocated[i] = 0;
      }
      for (uint8_t i = 0; i < allocated_flag_array_length; ++i) {
        interfaces_enabled[i] = 0;
      }
      for (size_t i = 0; i < max_base_size * length; ++i) {
        memory_space[i] = 0;
      }
    }
    void end() {
      this->clear_all();
    }

  public:
    // すべてのLED調光設定を削除
    void clearModeAll() {
      for (led_index_t i = 0; i < length; ++i) {
        this->deleteInterface(i);
      }
    }
    // LED調光設定を削除
    void clearMode(led_index_t led_idx) {
      this->deleteInterface(led_idx);
    }

  public:
    // すべてのLED調光設定の輝度を変更
    void setBrightnessAll(uint8_t brightness) {
      for (led_index_t i = 0; i < length; ++i) {
        this->setBrightness(i, brightness);
      }
    }
    // LED調光設定の輝度を変更
    void setBrightness(led_index_t led_idx, uint8_t brightness) {
      auto const pI = this->getInterface(led_idx);
      if (pI == nullptr) { return; }
      pI->setBrightness(brightness);
    }

    // LED調光設定の有効化状態を確認
    bool isEnabled(led_index_t led_idx) const {
      const uint8_t idx = led_idx / 8;
      const uint8_t bit = 1 << (led_idx % 8);
      return interfaces_enabled[idx] & bit;
    }

    // すべてのLED調光設定の有効化状態を設定
    void setEnabledAll(bool state) {
      for (uint8_t i = 0; i < allocated_flag_array_length; ++i) {
        if (state) {
          interfaces_enabled[i] = 0xff;
        } else {
          interfaces_enabled[i] = 0;
        }
      }
    }
    // LED調光設定の有効化状態を設定
    void setEnabled(led_index_t led_idx, bool state) {
      const uint8_t idx = led_idx / 8;
      const uint8_t bit = 1 << (led_idx % 8);
      if (state) {
        interfaces_enabled[idx] |= bit;
      } else {
        interfaces_enabled[idx] &= ~bit;
      }
    }

  protected:
    // LED調光モジュールがアロケート済みか確認
    bool isAllocated(led_index_t led_idx) const {
      const uint8_t idx = led_idx / 8;
      const uint8_t bit = 1 << (led_idx % 8);
      return interfaces_allocated[idx] & bit;
    }

  private:
    // LED調光モジュールがアロケート済みであると設定する (malloc 相当)
    bool setAllocated(led_index_t led_idx) {
      const uint8_t idx = led_idx / 8;
      const uint8_t bit = 1 << (led_idx % 8);
      return interfaces_allocated[idx] |= bit;
    }
    // LED調光モジュールが解放済みであると設定する (free 相当)
    bool clearAllocated(led_index_t led_idx) {
      const uint8_t idx = led_idx / 8;
      const uint8_t bit = 1 << (led_idx % 8);
      return interfaces_allocated[idx] &= ~bit;
    }

    // LED調光モジュールをデストラクトし、メモリを開放する (delete 相当)
    void deleteInterface(led_index_t led_idx) {
      auto const pI = getInterface(led_idx);
      if (pI == nullptr) { return; }
      pI->~MoePCB_LEDColorCalculator_SpecificInterfaceBase();
      this->clearAllocated(led_idx);
    }

  protected:
    // アロケートのみ行いアドレスを返す
    void* allocateInterface(led_index_t led_idx) override {
      this->deleteInterface(led_idx);
      this->setAllocated(led_idx);
      return &memory_space[max_base_size * led_idx];
    }

    // アドレス取得のみ行う
    MoePCB_LEDColorCalculator_SpecificInterfaceBase* getInterface(led_index_t led_idx) override {
      if (!this->isAllocated(led_idx)) {
        return nullptr;
      }
      void * const p = &memory_space[max_base_size * led_idx];
      return reinterpret_cast<MoePCB_LEDColorCalculator_SpecificInterfaceBase*>(p);
    }

  private:
    // interface がアロケート済みか判定するフラグ列
    uint8_t interfaces_allocated[allocated_flag_array_length];
    // interface の update を行うか決定するフラグ列
    uint8_t interfaces_enabled[allocated_flag_array_length];

    // MoePCB_LED_Interface 用のメモリ領域
    // MoePCB_LED_Interfaces で渡された派生クラスの
    // いずれであっても確実に保持できるようなメモリサイズにしている
    unsigned char memory_space[max_base_size * length];
  };

private:
  // モジュールの内部インターフェースを持つクラス
  template<class T_MoePCB_Params>
  class InnerClass {
    friend Module<T_MoePCB_Params>;

  private:
    typedef MoePCB_LED_Base<T_MoePCB_Params> led_base_t;
    typedef BundledColorCalculator<T_MoePCB_Params, T_LEDColorCalculators...> color_calculator_t;
    typedef T_LEDColorFilter<T_MoePCB_Params, BundledColorCalculator<T_MoePCB_Params, T_LEDColorCalculators...>> color_filter_t;
    typedef typename T_MoePCB_Params::LED::led_index_t led_index_t;

  protected:
    InnerClass() = default;
    InnerClass(const InnerClass &) = delete;
    InnerClass(InnerClass &&) = delete;
    InnerClass& operator= (const InnerClass &) = delete;
    InnerClass& operator= (InnerClass &&) = delete;
    ~InnerClass() = default;

  protected:
    inline void preBegin() { }
    inline void begin() {
      raw_led.begin();
      color_calculator.begin();
    }
    inline void postBegin() { }

    inline void preEnd() { }
    inline void end() {
      raw_led.end();
      color_calculator.end();
    }
    inline void postEnd() { }

    inline void preUpdate() { }
    inline void update() { }
    inline void postUpdate() { }

    inline void preInterrupt() { }
    inline void interrupt() {
      bool need_show = false;
      for (led_index_t led_idx = 0; led_idx < T_MoePCB_Params::LED::Length; ++led_idx) {
        if (color_calculator.isEnabled(led_idx)) {
          auto const pI = color_calculator.getInterface(led_idx);
          if (pI == nullptr) { continue; }
          const auto filtered_color = color_filter.filter(led_idx, pI->update());
          raw_led.setPixelColor((uint16_t) led_idx, filtered_color);
          need_show = true;
        }
      }
      if (need_show) {
        raw_led.show();
      }
      color_filter.update();
    }
    inline void postInterrupt() { }

  public:
    // LEDを直接制御するための Adafruit_NeoPixel 相当のインスタンスを取得する
    led_base_t& getRawLED() { return this->raw_led; }

    // LEDの色計算モジュールのインスタンスを取得する
    color_calculator_t& getColorCalculator() { return this->color_calculator; }

    // LEDの色調整モジュールのインスタンスを取得する
    color_filter_t& getColorFilter() { return this->color_filter; }

  private:
    led_base_t raw_led;
    color_calculator_t color_calculator;
    color_filter_t color_filter;
  };

public:
  // モジュールの公開インターフェースを持つクラス
  template<class T_MoePCB_Params>
  class Module
  {
  protected:
    Module() = default;
    Module(const Module &) = delete;
    Module(Module &&) = delete;
    Module& operator= (const Module &) = delete;
    Module& operator= (Module &&) = delete;
    ~Module() = default;

  public:
    // LEDモジュールのインスタンスを取得する
    inline InnerClass<T_MoePCB_Params>& getMainLED() { return inner_instance; }

  protected:
    inline void preBegin() { inner_instance.preBegin(); }
    inline void begin() { inner_instance.begin(); }
    inline void postBegin() { inner_instance.postBegin(); }

    inline void preEnd() { inner_instance.preEnd(); }
    inline void end() { inner_instance.end(); }
    inline void postEnd() { inner_instance.postEnd(); }

    inline void preUpdate() { inner_instance.preUpdate(); }
    inline void update() { inner_instance.update(); }
    inline void postUpdate() { inner_instance.postUpdate(); }

    inline void preInterrupt() { inner_instance.preInterrupt(); }
    inline void interrupt() { inner_instance.interrupt(); }
    inline void postInterrupt() { inner_instance.postInterrupt(); }

  private:
    InnerClass<T_MoePCB_Params> inner_instance;
  };
};

#endif
