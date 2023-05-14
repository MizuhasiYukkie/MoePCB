/*!
 * NewMoePCB_Timer.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef NewMoePCB_Timer_h
#define NewMoePCB_Timer_h

#include "NewMoePCB_Timer.h"

class MoePCB_Timer {
private:
  MoePCB_Timer() = delete;
  MoePCB_Timer(const MoePCB_Timer&) = delete;
  MoePCB_Timer(MoePCB_Timer&&) = delete;
  MoePCB_Timer& operator= (const MoePCB_Timer&) = delete;
  MoePCB_Timer& operator= (MoePCB_Timer&&) = delete;
  ~MoePCB_Timer() = delete;

public:
  static void preBegin() { }
  static void begin();
  static void postBegin() { }

  static void preEnd() { }
  static void end();
  static void postEnd() { }

  static void preUpdate() { }
  static void update() { }
  static void postUpdate() { }

  static void preInterrupt() { }
  static void interrupt() { }
  static void postInterrupt() { }

public:
  static void enable();
  static void disable();
};

#endif
