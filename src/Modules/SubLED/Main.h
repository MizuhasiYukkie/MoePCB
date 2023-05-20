/*!
 * Modules/SubLED/Main.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_SubLED_Main_h
#define Modules_SubLED_Main_h

#include <Arduino.h>

// 背面 ステータスLED
class MoePCB_SubLEDModule {
public:
  template<class T_MoePCB_Params>
  class Module;

private:
  // モジュールの内部インターフェースを持つクラス
  template<class T_MoePCB_Params>
  class InnerClass {
    friend Module<T_MoePCB_Params>;

  private:
    typedef typename T_MoePCB_Params::SubLED::pin_t pin_t;

  private:
    static constexpr pin_t pin = T_MoePCB_Params::SubLED::Pin;

  protected:
    InnerClass()
      : state(false)
    { }
    InnerClass(const InnerClass&) = delete;
    InnerClass(InnerClass&&) = delete;
    InnerClass& operator= (const InnerClass&) = delete;
    InnerClass& operator= (InnerClass&&) = delete;
    ~InnerClass() = default;

  protected:
    inline void preBegin() {
      pinMode(pin, OUTPUT);

      set(true);
      show();
    }
    inline void begin() { }
    inline void postBegin() { clear(); }

    inline void preEnd() { }
    inline void end() { }
    inline void postEnd() { }

    inline void preUpdate() { }
    inline void update() { }
    inline void postUpdate() { }

    inline void preInterrupt() { }
    inline void interrupt() { }
    inline void postInterrupt() { }

  public:
    // 点灯状態指定 (show を呼び出すまで反映されない)
    inline void set(bool v) { this->state = v; }

    // 点灯状態取得 (実際の点灯状態ではなく内部的に指定している状態を返す)
    inline bool get() const { return this->state; }

    // 点灯状態反映
    inline void show() { digitalWrite(pin, (this->state) ? LOW : HIGH); }

    // 消灯 (直ちに消灯する)
    inline void clear() { set(false); show(); }

  private:
    bool state : 1;
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
    // サブLEDモジュールのインスタンスを取得する
    inline InnerClass<T_MoePCB_Params>& getSubLED() { return inner_instance; }

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
