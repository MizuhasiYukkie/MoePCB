/*!
 * NewMoePCB_Core.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef NewMoePCB_Core_h
#define NewMoePCB_Core_h

#include "NewMoePCB_Timer.h"

// 萌基板 本体
// 各モジュールをまとめた構造体的な役割
// これ自身は便利機能を提供しない
template<class T_MoePCB_Params, class... T_MoePCB_Module>
class MoePCB_Core
  : public T_MoePCB_Module::template Module<T_MoePCB_Params>...
{
public:
  typedef T_MoePCB_Params params_t;

public:
  MoePCB_Core() = default;
  MoePCB_Core(const MoePCB_Core&) = delete;
  MoePCB_Core(MoePCB_Core&&) = delete;
  MoePCB_Core& operator= (const MoePCB_Core&) = delete;
  MoePCB_Core& operator= (MoePCB_Core&&) = delete;
  ~MoePCB_Core() = default;

// swallow idiom
// やっていることは T_MoePCB_Module で渡された複数の基底クラスのメンバ関数 funcName を順番に呼び出すこと
#define MOEPCB_Macro_Swallow(funcName) do { int swallow[] = { (T_MoePCB_Module::template Module<T_MoePCB_Params>::funcName(), 0)... }; (void)&swallow; } while(0)

protected:
  // 基底クラスで定義されている(はず)のメンバ関数を呼ばれないよう削除しておく
  void preBegin() = delete;
  void postBegin() = delete;

  void preEnd() = delete;
  void postEnd() = delete;

  void preUpdate() = delete;
  void postUpdate() = delete;

  void preInterrupt() = delete;
  void postInterrupt() = delete;

public:
  // (コンストラクトではない) 初期化
  inline void begin()
  {
    MOEPCB_Macro_Swallow(preBegin);
    MOEPCB_Macro_Swallow(begin);

    MoePCB_Timer::begin();
    MoePCB_Timer::enable();

    MOEPCB_Macro_Swallow(postBegin);
  }

  // (デストラクトではない) 破棄
  inline void end()
  {
    MOEPCB_Macro_Swallow(preEnd);
    MOEPCB_Macro_Swallow(end);
    MOEPCB_Macro_Swallow(postEnd);
  }

  // 更新 (内部状態を更新する必要がないときは呼ばなくても問題ないもの)
  inline void update() {
    MOEPCB_Macro_Swallow(preUpdate);
    MOEPCB_Macro_Swallow(update);
    MOEPCB_Macro_Swallow(postUpdate);
  }

  // 更新 (割り込みで常に一定間隔で呼び出す必要があるもの)
  inline void interrupt() {
    MOEPCB_Macro_Swallow(preInterrupt);
    MOEPCB_Macro_Swallow(interrupt);
    MOEPCB_Macro_Swallow(postInterrupt);
  }

#undef MOEPCB_Macro_Swallow

};

#endif
