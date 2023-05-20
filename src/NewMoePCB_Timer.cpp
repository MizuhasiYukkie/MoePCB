/*!
 * NewMoePCB_Timer.cpp - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */
#include "NewMoePCB_Timer.h"

#include <Arduino.h>

void MoePCB_Timer::begin()
{
#if 1
  //タイマーA：20-25msごとにコンペアAクリア＆割り込みを設定
  TCCR4B = 0;
  TCCR4A = 0;
  TCCR4C = 0;
  TCCR4D = 0;
  TCCR4E = 0;
  TCCR4B = 0b1100; // 1/2048
  OCR4C = 156;     // 0-255  <-- 16,000,000 MHz / 2048 / 50Hz
  // OCR4C =195;//0-255  <-- 16,000,000 MHz / 2048 / 40Hz
#else
  //タイマー1：20-25msごとにコンペアAクリア＆割り込みを設定
  TCCR1B = 0;  TCCR1A = 0;  TCCR1C = 0;
  TCCR1B = (1<<WGM12)|(1<<CS02)|(1<<CS00);//0b00000010;// 1/1024
  OCR1A =312;//16bit  <-- 16,000,000 MHz / 1024 / 50Hz
  //OCR1A =390;//16bit  <-- 16,000,000 MHz / 1024 / 40Hz
#endif
}

void MoePCB_Timer::end()
{
  // FIXME: 実装する
}

void MoePCB_Timer::enable()
{
#if 1
  //タイマー４開始処理
  TIFR4 = (1 << OCF4A);
  TCNT4 = 0;
  TIMSK4 = (1 << OCIE4A);
#else
  //タイマー1開始処理
  TIFR1 = (1<<OCF1A);
  TCNT1 = 0;
  TIMSK1 = (1<<OCIE1A);
#endif
}

void MoePCB_Timer::disable()
{
  // FIXME: 実装する
}


#if 1
//タイマー４ 20-25ms割り込み
extern void MoePCB_Task(void);
ISR(TIMER4_COMPA_vect)
{
  MoePCB_Task();
}
#else
//タイマー１版 20-25ms割り込み　※未使用
//    ISR (TIMER1_COMPA_vect) {
//      MoePCB_Task();
//    }
#endif
