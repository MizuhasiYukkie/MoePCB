/*!
 * MoePCB.h - Library for Moe-PCB (Moe Kiban)
 * 
 * Copyright (c) 2022 Mizuhasi Yukkie 
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
*/
#ifndef USE_NEW_MOE_PCB
    #ifndef MoePCB_h
    #define MoePCB_h
    #include "Arduino.h"

    #include <Adafruit_NeoPixel.h>

    #define LED0 13         // 通常の単色LED接続ピン
    #define RGBLED_PIN 6    // NeoPixel接続ピン
    #define MAX_LED_NUM 30  // 効率の良い初期化方法がわからないため、ひとまずLED数30までということで初期化している

    #define A 0 //サブ点灯パターン
    #define B 1
    #define C 2
    #define D 3
    #define E 4

    #define LED1 0
    #define LED2 1
    #define LED3 2
    #define LED4 3
    #define LED5 4
    #define LED6 5
    #define LED7 6
    #define LED8 7
    #define LED9 8
    #define LED10 9
    #define LED11 10
    #define LED12 11
    #define LED13 12
    #define LED14 13
    #define LED15 14
    #define LED16 15
    #define LED17 16
    #define LED18 17
    #define LED19 18
    #define LED20 19
    #define LED21 20


    class MoePCB : public Adafruit_NeoPixel
    {
      public:
        //LED数を与えてインスタンスを作成する
        MoePCB( uint8_t);

        void begin(void);             // 開始処理
        void update(void);            // 自動インターポーレート
        void rainbow(int, int, uint8_t);//光らせたいLED番号、ベース色からの差分、点灯サブパターン
        void breath(int);             // 光らせたいLED番号
        void icy(int, uint8_t);       // 光らせたいLED番号、点灯サブパターン
        void autumn(int, int);        //光らせたいLED番号、ベース色からの差分
        void gaming(int, int);        //光らせたいLED番号、ベース色からの差分
        void sword(int, int, uint8_t, uint8_t );//光らせたいLED番号、ベース色からの差分、点灯サブパターン
        void lvmeter(int, int, uint8_t);       //光らせたいLED番号、レベルメーターの範囲を0-255と仮定してそのうちどこにアタッチしたいか
        void lvmeter_input(int);      //レベルメーターへの値の入力（0-255）一応300くらいまでは受け付けている
        uint8_t brightness=1;         //明るさ　0-3の４段階
        
        void brightness_add();//明るさを１段階追加する　最大→最小へ循環
        void acknowledge();//了解コール
        void angry(bool);
        void cold(bool);
        void heat(bool);
        void drunk(bool);
        float cputemp_raw(void);//CPU温度のADC値を返す
        float cputemp(float, float);//CPU温度を返す 現在の温度、その時のCPUADCraw値を入力

      private:
        const uint8_t _brightnessTable[4]={10,25,65,175};//明るさレベルに応じた明るさ値
        const uint8_t _twinkleTable[4]   ={40,65,155,255};//明るさレベルに応じたランダムで変更するきらきら値

        Adafruit_NeoPixel _pixels;
        uint8_t _brightness;          // 明るさ 0-3
        uint8_t _led_num;             // LEDの個数を保存
        float H_raw[MAX_LED_NUM];     // LEDの彩度追従値 0-255　雑にMAX_LED_NUM個分で初期化しているので暇があれば最適化すること
        float S_raw[MAX_LED_NUM];     // LEDの彩度追従値 0-255　雑にMAX_LED_NUM個分で初期化しているので暇があれば最適化すること
        float V_raw[MAX_LED_NUM];     // LEDの照度追従値 0-255　雑にMAX_LED_NUM個分で初期化しているので暇があれば最適化すること

        float H[MAX_LED_NUM];
        float S[MAX_LED_NUM];         // LEDの彩度指示値 0-255　雑にMAX_LED_NUM個分で初期化しているので暇があれば最適化すること
        float V[MAX_LED_NUM];         // LEDの照度指示値 0-255　雑にMAX_LED_NUM個分で初期化しているので暇があれば最適化すること

        float rainbow_cnt;            // レインボーモードのカウンタ　色環に一致（0-360）
        uint8_t general_cnt;          // 汎用カウンタ(0-255)
        uint8_t gaming_cnt;           // ゲーミングモード用カウンタ (0-255)
        uint8_t FuryGauge;            // 怒りゲージ (0-255)
        uint8_t ColdGauge;            // 寒いよゲージ (0-255)
        uint8_t HeatGauge;            // 暑いよゲージ (0-255)
        uint8_t DrunkGauge;           // 酔ってるよゲージ (0-255)
        int    _LevelMeter;           // レベルメーターゲージ(0-255)
        int    _LevelPeak;            // レベルメーターピーク値(0-255)        
        uint8_t pulsation;            // 脈動させるためのゲージ(0-255)
        bool angly_flag=0;            // 怒りモードフラグ　これが１だとゲージが自動で増える
        bool cold_flag=0;             // 寒いよモードフラグ　これが１だとゲージが自動で増える
        bool heat_flag=0;             // 暑いよモードフラグ　これが１だとゲージが自動で増える
        bool drunk_flag=0;            // 酔ってるよモードフラグ　これが１だとゲージが自動で増える
        float morph(float, float, uint8_t);//モーフィング関数

    };

    #endif
#else
#include "./NewMoePCB.h"
#endif
