/*!
 * Modules/Thermometer/Main.h - Library for Moe-PCB (Moe Kiban)
 *
 * Copyright (c) 2022-2023 Mizuhasi Yukkie, Onozawa Hiro
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

#ifndef Modules_Thermometer_Main_h
#define Modules_Thermometer_Main_h

#include <Arduino.h>

class MoePCB_ThermometerModule {
public:
  template<class T_MoePCB_Params>
  class Module;

private:
  // モジュールの内部インターフェースを持つクラス
  template<class T_MoePCB_Params>
  class InnerClass {
    friend Module<T_MoePCB_Params>;

  public:
    typedef int16_t adc_raw_t;

    // デフォルトの校正値
    // Mizuhasi Yukkie 氏のコメントをもとにテキトーに設定
    class DefaultReferenceData {
    public:
      static constexpr float Temperture[2] = { 10.0f, 18.0f };
      static constexpr adc_raw_t ADC_Raw[2] = { 275, 295 };
    };

  protected:
    InnerClass() = default;
    InnerClass(const InnerClass&) = delete;
    InnerClass(InnerClass&&) = delete;
    InnerClass& operator= (const InnerClass&) = delete;
    InnerClass& operator= (InnerClass&&) = delete;
    ~InnerClass() = default;

  protected:
    static inline void preBegin() { }
    static inline void begin() { }
    static inline void postBegin() { }

    static inline void preEnd() { }
    static inline void end() { }
    static inline void postEnd() { }

    static inline void preUpdate() { }
    static inline void update() { }
    static inline void postUpdate() { }

    inline void preInterrupt() { }
    inline void interrupt() { }
    inline void postInterrupt() { }

  public:
    // ADC の生の値を返す [0, 1023] が返る想定
    static inline adc_raw_t getADCRaw() { return getADCRawAverage(1); }

    // ADC の生の値を複数サンプリングし、平均を返す [0, 1023] が返る想定
    static adc_raw_t getADCRawAverage(uint16_t sample) {
      // MUX = 1 00111; 温度感知器
      // REFS = 11;     A/D変換部の基準電圧選択に 2.56V内部基準電圧 を選択する必要がある
      ADCSRB = _BV(MUX5);
      ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0));
      ADCSRA |= _BV(ADEN);

      //  温度感知器の駆動部の伝播遅延は概ね2μsです。
      //  従って2回の逐次比較が必要とされます。
      //  正しい温度測定は2回目のそれです。
      // とあるので、1回読み捨てる
      ADCSRA |= _BV(ADSC); // 変換開始
      while (bit_is_set(ADCSRA,ADSC));// 変換終了

      int32_t _value = 0;
      for(uint16_t _counter = 0; _counter < sample; _counter++)
      {
        ADCSRA |= _BV(ADSC); // 変換開始
        while (bit_is_set(ADCSRA,ADSC));// 変換終了

        _value += ADCW;
      }

      // Commented by Mizuhasi Yukkie :
      // 室温 294.0-306.0くらい 電源入れた直後は289.0とか
      // 冷蔵庫に少し入れたあと 274.0-277.0
      return static_cast<adc_raw_t>(_value / sample);
    }

    // ADC の生の値を温度に変換する
    // Reference : 校正済みの基準データ、基準温度とその時の adcRaw の値のペアを2組持たせる
    template<class Reference = DefaultReferenceData>
    static float convertADCRawToTemperature(adc_raw_t adcRaw){
      constexpr float diffTemperature = Reference::Temperture[1] - Reference::Temperture[0];
      constexpr adc_raw_t diffADCRaw = Reference::ADC_Raw[1] - Reference::ADC_Raw[0];

      constexpr float trend = diffADCRaw / diffTemperature;
      constexpr float offset = Reference::Volatage[1] - Reference::Temperture[1] * trend;

      return (adcRaw - offset) / trend;
    }
  };

public:
  // モジュールの公開インターフェースを持つクラス
  template<class T_MoePCB_Params>
  class Module {
  protected:
    Module() = default;
    Module(const Module&) = delete;
    Module(Module&&) = delete;
    Module& operator= (const Module&) = delete;
    Module& operator= (Module&&) = delete;
    ~Module() = default;

  public:
    // 温度計モジュールのインスタンスを取得する
    inline InnerClass<T_MoePCB_Params>& getThermometer() { return inner_instance; }

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
