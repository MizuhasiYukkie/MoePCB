/*!
 * Modules/TouchSensor/Main.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_TouchSensor_Main_h
#define Modules_TouchSensor_Main_h

#include "Instance.h"

class MoePCB_TouchSensorModule {
private:
  // cite: https://lpha-z.hatenablog.com/entry/2020/04/05/231500
  class Util {
    template<size_t...> struct index_sequence {};

    template<size_t I, size_t... Is> struct S : S<I-1, I-1, Is...> {};
    template<size_t... Is> struct S<0, Is...> { index_sequence<Is...> f(); };

    template<size_t N> using make_index_sequence = decltype(S<N>{}.f());
  };

public:
  template<class T_MoePCB_Params>
  class Module;

private:
  // モジュールの内部インターフェースを持つクラス
  template<class T_MoePCB_Params, class = Util::make_index_sequence<T_MoePCB_Params::TouchSensor::Length>>
  class InnerClass;

  // モジュールの内部インターフェースを持つクラス (の特殊化)
  template<class T_MoePCB_Params, size_t... I>
  class InnerClass<T_MoePCB_Params, Util::index_sequence<I...>> {
    friend Module<T_MoePCB_Params>;

  private:
    typedef typename T_MoePCB_Params::TouchSensor::touch_sensor_index_t index_t;

    static constexpr index_t length = T_MoePCB_Params::TouchSensor::Length;

  protected:
    InnerClass()
      : sensors({ T_MoePCB_Params::TouchSensor::Port::Get(I)... })
    {}
    InnerClass(const InnerClass&) = delete;
    InnerClass(InnerClass&&) = delete;
    InnerClass& operator= (const InnerClass&) = delete;
    InnerClass& operator= (InnerClass&&) = delete;
    ~InnerClass() = default;

  public:
    inline void preBegin() { }
    inline void begin() { for (index_t idx = 0; idx < length; ++idx) { sensors[idx].begin(); } }
    inline void postBegin() { }

    inline void preEnd() { }
    inline void end() { }
    inline void postEnd() { }

    inline void preUpdate() { }
    inline void update() { for (index_t idx = 0; idx < length; ++idx) { sensors[idx].update(); } }
    inline void postUpdate() { }

    inline void preInterrupt() { }
    inline void interrupt() { }
    inline void postInterrupt() { }

  public:
    // タッチセンサのインスタンスを取得する
    inline MoePCB_TouchSensor& getRawTouchSensor(index_t idx) { return sensors[idx]; }

  private:
    // MoePCB_TouchSensor のインスタンスの配列
    MoePCB_TouchSensor sensors[length];
  };

public:
  // モジュールの公開インターフェースを持つクラス
  template<class T_MoePCB_Params>
  class Module
  {
  protected:
    Module() = default;
    Module(const Module&) = delete;
    Module(Module&&) = delete;
    Module& operator= (const Module&) = delete;
    Module& operator= (Module&&) = delete;
    ~Module() = default;

  public:
    // タッチセンサモジュールのインスタンスを取得する
    inline InnerClass<T_MoePCB_Params>& getTouchSensor() { return inner_instance; }

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
