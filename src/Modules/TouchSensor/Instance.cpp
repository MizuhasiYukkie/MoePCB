/*!
 * Modules/TouchSensor/Instance.cpp - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */
#include "Instance.h"

#include <ADCTouch.h>

void MoePCB_TouchSensor::begin()
{
  // ADCnピンのデジタル入力緩衝部を禁止する
  // デジタル入力緩衝部での消費電力を削減する
  switch (this->port) {
    case A0: DIDR0 |= _BV(ADC7D); break;
    case A1: DIDR0 |= _BV(ADC6D); break;
    case A2: DIDR0 |= _BV(ADC5D); break;
    case A3: DIDR0 |= _BV(ADC4D); break;
    case A4: DIDR0 |= _BV(ADC1D); break;
    case A5: DIDR0 |= _BV(ADC0D); break;
    case A6: DIDR2 |= _BV(ADC8D); break;
    case A7: DIDR2 |= _BV(ADC9D); break;
    case A8: DIDR2 |= _BV(ADC10D); break;
    case A9: DIDR2 |= _BV(ADC11D); break;
    case A10: DIDR2 |= _BV(ADC12D); break;
    case A11: DIDR2 |= _BV(ADC13D); break;
  }

  this-> offset = ADCTouch.read(this->port, Init_Sampling);
}

void MoePCB_TouchSensor::update() {
  const int raw = ADCTouch.read(this->port, Update_Sampling);
  const int sense  = raw - this->offset;

  const bool isPressing = Threshold < sense;
  const State::type_t rawState = this->state;
  const bool isFreezing = rawState & State::Freezing;
  const bool isLastRelease = rawState & State::On_Release;
  const State::type_t lastPressingCnt = isLastRelease ? 0 : (rawState & State::Mask_Pressing);
  State::type_t nextState = 0;

  if (isPressing) {
    if (isFreezing) {
      nextState = rawState;
    } else if (lastPressingCnt >= State::Mask_Pressing) {
      nextState = State::Mask_Pressing;
    } else {
      nextState = lastPressingCnt + 1;
    }
  } else {
    if (!isFreezing && lastPressingCnt > 0) {
      nextState = lastPressingCnt | State::On_Release;
    }
  }

  this->state = nextState;
}
