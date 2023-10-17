/*!
 * KNMK-0007A Yukarino向けソースコード
 *
 * Copyright (c) 2023 Mizuhasi Yukkie
 * This software is released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */

/*
 * This program requires the folloing libraries.
 * このコードを使用するためには、下記のライブラリをインストールしてください。
 *
 * 【ADCTouch】 https://github.com/martin2250/ADCTouch
 * 【Adafruit_NeoPixel】 https://github.com/adafruit/Adafruit_NeoPixel
 * 【MoePCB】 https://github.com/MizuhasiYukkie/MoePCB
 *
 * ツール→ボード："Arduino Leonardo"を選択
 * ツール→シリアルポート：「USBで接続済みの萌基板のポート」（ArduinoLeonardoと表示されます）を選択
 * 「マイコンボードに書き込む」を実行するとここに書いてあるプログラムが書き込まれます。
 * ※USB端子経由で書き込めない場合はICSP端子からAVRISP
 * mkⅡなどを用いて書き込んでください。
 */

#include <MoePCB.h>
#include <ADCTouch.h>


// タッチセンシングのポート設定（基板により違うので注意）
#define SKIRT A4
#define MUNE A1
#define KAMI A2
#define RIBBON_R A3  // スキマ右リボン
#define RIBBON_L A0  // スキマ左リボン

// IRプロトコル解析-NEC
#define DECODE_NEC

// タッチセンシングの初期値
int kami_offset;  
int mune_offset;
int skirt_offset;
int ribbon_R_offset;
int ribbon_L_offset;

// タッチセンシングフラグ
bool kami_flag;  
bool mune_flag;
bool skirt_flag;
bool ribbon_R_flag;
bool ribbon_L_flag;

// 他の誰かがIRを発していることを検出する 保持タイマーも兼ねてる
int ir_detectflag = 0;
// 誰かが怒りモード担っているのを検出する　保持タイマーも兼ねてる
int ir_angry_detectflag = 0;
// 霊夢を検出する　保持タイマーも兼ねてる
int reimu_detectflag = 0;
// 夢想封印を検出する　保持タイマーも兼ねてる
int musou_detectflag = 0;
// 夢想封印状態のゲージ
int musou_gauge = 0;

uint8_t PATTERN_MODE = 0;  // 点灯パターン 0-2

#define NO_LED_FEEDBACK_CODE  // IR saves 418 bytes program space
#include <IRremote.hpp>

  //紫特有の単色LED用
  int BLUE_LED_pin =9;//5,[9],[10],[11] 5は駄目だった 11だけ周波数が高い
  int RED_LED_pin =10;
  int BLUE_LED_power=0;
  int RED_LED_power=0;
  int nomal_LED_cnt=0;
  //LED用PWMパワーを50段階で計算したもの。対数
  const int LED_POWER[]={0,3,4,5,7,8,9,11,12,14,15,17,19,20,22,24,26,27,29,31,33,35,38,40,42,45,47,50,53,56,59,62,65,68,72,76,80,85,89,95,100,106,113,121,129,139,151,166,184,210,255};//

MoePCB Yukari(14);  // インスタンス生成（RGBLED数）

// タイマーにより自動で実行
void MoePCB_Task() {



  
  switch (PATTERN_MODE) {
      // 点灯パターン１
    case 0:
      pattern1();
      break;

      // 点灯パターン２
    case 1:
      pattern2();
      break;

      // 点灯パターン３：ゲーミングモード
    case 2:
      //単色LEDには交互に最大輝度を与えてる
      if(nomal_LED_cnt==0)BLUE_LED_power=20;
      if(nomal_LED_cnt==1)BLUE_LED_power=35;
      if(nomal_LED_cnt==2)BLUE_LED_power=50;
      if(nomal_LED_cnt==51)RED_LED_power=20;
      if(nomal_LED_cnt==52)RED_LED_power=35;
      if(nomal_LED_cnt==53)RED_LED_power=50;
      nomal_LED_cnt++;
      if(100<nomal_LED_cnt)nomal_LED_cnt=0;

      Yukari.gaming(LED1, 0);   // LED0:裏面RGBLED
      Yukari.gaming(LED10, 0);  // LED4:ゲーミングモード（位相差）
      Yukari.gaming(LED11, 20);
      Yukari.gaming(LED12, 40);
      Yukari.gaming(LED13, 60);
      Yukari.gaming(LED14, 80);
      Yukari.gaming(LED2, 100);
      Yukari.gaming(LED4, 120);
      Yukari.gaming(LED3, 120);
      Yukari.gaming(LED5, 140);
      Yukari.gaming(LED6, 160);
      Yukari.gaming(LED7, 180);
      Yukari.gaming(LED8, 200);
      Yukari.gaming(LED9, 220);
      break;
  }

  while (!IrReceiver.isIdle())
    ;  // IR受信状態がアイドル状態になるまで待つ　NeoPixel処理は割り込みハンドラを阻害するため
  Yukari.update();  // 計算＆LEDに送信



  //単色LED操作コーナー ***********************************************
  //怒りを検出したとき
  if(0<Yukari.Get_FuryGauge()){
    //怒りはじめ
    if(Yukari.Get_FuryGauge()<250){
      BLUE_LED_power=map(Yukari.Get_FuryGauge(),0,255,0,50);
      RED_LED_power=map(Yukari.Get_FuryGauge(),0,255,0,50);
    }else{
    //怒りきったとき・脈動を拾ってきて入れる
      if(Yukari.Get_pulsation() < 20 )BLUE_LED_power=50;
      if(Yukari.Get_pulsation() < 20 )RED_LED_power=50;
    }
  }  
  //紫特有の単色LED、スムージング操作 パワーは0-50　上の方にある配列使って対数0-255に直してAnalogWriteしてる
  BLUE_LED_power--;
  RED_LED_power--;
  if( 50<BLUE_LED_power )BLUE_LED_power=50;
  if( 50<RED_LED_power  )RED_LED_power=50;
  if( BLUE_LED_power<0 )BLUE_LED_power=0;
  if( RED_LED_power <0 )RED_LED_power=0;

  if(PATTERN_MODE==2){ 
    if(Yukari.brightness==0){
      analogWrite(BLUE_LED_pin, LED_POWER[BLUE_LED_power]/2);
      analogWrite(RED_LED_pin, LED_POWER[RED_LED_power]/2);
    }else{
      analogWrite(BLUE_LED_pin, LED_POWER[BLUE_LED_power]);
      analogWrite(RED_LED_pin, LED_POWER[RED_LED_power]);
    }    
  }else{
    if(Yukari.brightness==0){
      analogWrite(BLUE_LED_pin, LED_POWER[BLUE_LED_power]/4);
      analogWrite(RED_LED_pin, LED_POWER[RED_LED_power]/4);
    }else if(Yukari.brightness==1){
      analogWrite(BLUE_LED_pin, LED_POWER[BLUE_LED_power]/2);
      analogWrite(RED_LED_pin, LED_POWER[RED_LED_power]/2);
    }else{
      analogWrite(BLUE_LED_pin, LED_POWER[BLUE_LED_power]);
      analogWrite(RED_LED_pin, LED_POWER[RED_LED_power]);
    }    
  }
  
  

  //単色LED操作コーナー ***********************************************

}

void setup() {
  //  Serial.begin(9600);  // シリアル通信を使いたいとき
  //  while(!Serial);

  Yukari.begin();  // 萌基板初期化 タイマー無効で開始

  IrSender.begin(3);    // IRremoteはD3から出力する
  IrReceiver.begin(2);  // D2で受信

  // タッチセンシングの初期値を登録
  kami_offset = ADCTouch.read(KAMI, 500);  // 読むポート, サンプリング数
  mune_offset = ADCTouch.read(MUNE, 500);
  skirt_offset = ADCTouch.read(SKIRT, 500);
  ribbon_R_offset = ADCTouch.read(RIBBON_R, 500);
  ribbon_L_offset = ADCTouch.read(RIBBON_L, 500);
}

void loop() {
  /* CPU内蔵温度感知機能を使う場合は校正した方がいいです（個体差大）
      static uint8_t cnt;
      if(30<cnt){
        cnt=0;
        Serial.print(Yukari.cputemp_raw()); Serial.print(" /
     ");//CPU温度ADC値を見たい時 Serial.print(Yukari.cputemp(26.0,
     305.0));//305.0のところは各自の校正値を入れる：校正時の周辺の実測温度、その時のCPU温度ADC値（上段）
        Serial.println(" C");
      }
      cnt++;
      //50度超えると暑いよ〜
      if(50 < Yukari.cputemp(26.0, 305.0)
     )Yukari.heat(true);//cputempに校正値を与えると気温が返ってくる else
     Yukari.heat(false);

      //10度下回ると寒いよ〜
      if(Yukari.cputemp(26.0, 305.0) < 10
     )Yukari.cold(true);//cputempに校正値を与えると気温が返ってくる else
     Yukari.cold(false);
  */

  int kami_sense = ADCTouch.read(KAMI, 10) - kami_offset;  // 髪の毛タッチ
  int mune_sense = ADCTouch.read(MUNE, 10) - mune_offset;  // お胸タッチ
  int skirt_sense = ADCTouch.read(SKIRT, 10) - skirt_offset;  // スカートタッチ
  int ribbon_R_sense = ADCTouch.read(RIBBON_R, 10) - ribbon_R_offset;  // 右のリボンタッチ
  int ribbon_L_sense = ADCTouch.read(RIBBON_L, 10) - ribbon_L_offset;  // 左のリボンタッチ

  //  タッチセンシングの値を見たいとき
  //  Serial.println(kami_sense);
  //  Serial.println(mune_sense);
  //  Serial.println(skirt_sense);

  // 髪タッチ検知
  if (50 < kami_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (kami_flag == 0) Yukari.brightness_add();
    kami_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else
    kami_flag = 0;  // 離したのでタッチフラグクリア

  // 胸タッチ検出で怒る
  if (50 < mune_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (mune_flag == 0) {
      // 胸タッチ検出IRをワンショット送る
      IR_send(ANGRY, Yukari.Get_FuryGauge());
    }
    Yukari.angry(true);
    mune_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else {
    // あるいは誰かがIR越しに怒っていることを検出したら怒る
    if (ir_angry_detectflag != 0)
      Yukari.angry(true);
    else
      // 胸タッチしていない、かつIRでも誰でも怒っていなければ怒りをおさめる
      Yukari.angry(false);
    mune_flag = 0;  // 離したのでタッチフラグクリア
  }

  // スカートタッチ検出
  if (50 < skirt_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (skirt_flag == 0) {
      Yukari.acknowledge();  // 了解コール
      if (PATTERN_MODE == 0)
        PATTERN_MODE = 1;
      else if (PATTERN_MODE == 1)
        PATTERN_MODE = 2;
      else
        PATTERN_MODE = 0;
    }
    skirt_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else {
    skirt_flag = 0;  // 離したのでタッチフラグクリア
  }




  // スキマ右リボンタッチ
  if (50 < ribbon_R_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (ribbon_R_flag == 0) {
      // タッチ検出IRをワンショット送る
      IR_send(RIBBON_R, Yukari.Get_FuryGauge());
    }
    ribbon_R_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else {
    ribbon_R_flag = 0;  // 離したのでタッチフラグクリア
  }

  // スキマ左リボンタッチ
  if (50 < ribbon_L_sense) {
    // フラグがまだ立っていなければ以下を実行
    if (ribbon_L_flag == 0) {
      // タッチ検出IRをワンショット送る
      IR_send(RIBBON_L, Yukari.Get_FuryGauge());
    }
    ribbon_L_flag = 1;  // フラグを立てることで立ち上がり時のみ実行
  } else {
    ribbon_L_flag = 0;  // 離したのでタッチフラグクリア
  }
  

  // IR関係
  // 状態送信
  static int IR_cnt = 0;
  if (100 + random(0, 5) < IR_cnt) {
    IR_cnt = 0;
    if (50 < mune_sense) {
      IR_send(ANGRY, Yukari.Get_FuryGauge());  // 胸タッチ検出
    } else {
      IR_send(PATTERN_MODE, Yukari.Get_general_cnt());
    }
  }
  IR_cnt++;

  // 誰かがいればIRフラグが経つ　しばらく見つからなければフラグは減っていきゼロへ
  if (0 < ir_detectflag) ir_detectflag--;

  // 誰かが怒っていればいればIR怒りフラグが経つ　しばらく見つからなければフラグは減っていきゼロへ
  if (0 < ir_angry_detectflag) ir_angry_detectflag--;

  // 霊夢がいればフラグが経つ　しばらく見つからなければフラグは減っていきゼロへ
  if (0 < reimu_detectflag) reimu_detectflag--;

  // 夢想封印検出フラグ
  if (0 < musou_detectflag) musou_detectflag--;

  // 夢想封印検出しなくなったらゲージを自動でへらす
  if (musou_detectflag == 0) {
    if (0 < musou_gauge) musou_gauge -= 1;
  }

  MoePCB_Task();  // 萌基板タスク

  // IR受信データがあればデコード関数へ
  if (IrReceiver.decode()) IR_decode();

  // そこそこ正確に20ms待機する
  // 前回から20ms経つまで待機。超えていたらすぐ次へ
  static unsigned long past_t;
  while (millis() < (past_t + 20))
    ;
  past_t = millis();
}

// 発信　基板IDを間違えないように。
void IR_send(uint8_t data1, uint8_t data2) {
  IrSender.sendNEC((YUKARI << 8) + data1, data2, 0);
  // 基板ID(8bit) +コマンド(8bit) +データ(8bit) +リピート回数
}

// 受信
void IR_decode() {
  // 受信データ 基板ID、コマンド（orモード）、データ　8bitｘ３
  uint8_t ID;
  uint8_t command;
  uint8_t data;

  if (IrReceiver.decodedIRData.protocol == NEC) {
    ID = highByte(IrReceiver.decodedIRData.address);
    command = lowByte(IrReceiver.decodedIRData.address);
    data = IrReceiver.decodedIRData.command;
    /*
        Serial.print("kiban ID= ");
        Serial.print(ID);
        Serial.print(",   mode/command= ");
        Serial.print(command);
        Serial.print(",   data= ");
        Serial.println(data);
    */
    // 自分以外の誰かが居ることを検出
    if (ID != YUKARI) {
      // 未開発だけどID64までとりあえず認識する
      if ((0 < ID) && (ID < 64)) {
        ir_detectflag = 50 * 4;  // 4秒ほど見つからなければ自動でゼロへ

        // 自分以外の誰かが怒りモードになってる
        if (command == ANGRY) {
          ir_angry_detectflag = 50 * 3;  // 3秒ほど維持する
        }
      }
    }
  }
  IrReceiver.resume();  // Enable receiving of the next value
}

void pattern2() {

  //  誰かを見つけると裏側のLEDをシアンに光らせる
  if((ribbon_L_flag)||(ribbon_R_flag)){
    Yukari.cyanbreath(LED1);
    Yukari.cyanbreath(LED2);
  }else if (ir_detectflag != 0) {
    Yukari.cyanbreath(LED1);
    Yukari.cyanbreath(LED2);
  } else {
    Yukari.breath(LED1);
    Yukari.mute(LED2);
  }

  //******************************
  
  if(ribbon_L_flag){
    if(BLUE_LED_power < 51 )BLUE_LED_power+=(2+Yukari.brightness);
  }
  if(ribbon_R_flag){
    if(RED_LED_power < 51 )RED_LED_power+=(2+Yukari.brightness);
  }
  //******************************
  
  //傘
  if((ribbon_L_flag)&&(ribbon_R_flag)){
    Yukari.gaming(LED6,  10 * 100);
    Yukari.gaming(LED7,  10 * 60);
    Yukari.gaming(LED8,  10 * 20);
    Yukari.gaming(LED9,  10 * 0);
    Yukari.gaming(LED10, 10 * 40);
    Yukari.gaming(LED11, 10 * 80);
  }else{
    Yukari.rainbow(LED6,  10 * 0, A);
    Yukari.rainbow(LED7,  10 * 1, A);
    Yukari.rainbow(LED8,  10 * 2, A);
    Yukari.rainbow(LED9,  10 * 3, A);
    Yukari.rainbow(LED10, 10 * 4, A);
    Yukari.rainbow(LED11, 10 * 5, A);
  }
  
  //単色LEDランダムぴかぴか******************************
  int set_LED_POWER;
  if(!random(0, 400))BLUE_LED_power=50;
  if(!random(0, 400))RED_LED_power=50;
  //単色LEDランダムぴかぴか******************************

  //すきまお目々
  if((ribbon_L_flag)&&(ribbon_R_flag)){
    Yukari.masterspark(LED5, 0);
    Yukari.masterspark(LED4, 4);
    Yukari.masterspark(LED3, 8);
    Yukari.masterspark(LED14, 10);
    Yukari.masterspark(LED13, 6);
    Yukari.masterspark(LED12, 2);
  }else if(ribbon_L_flag){
    Yukari.masterspark(LED5, 0);
    Yukari.masterspark(LED4, 3);
    Yukari.masterspark(LED3, 6);
    Yukari.masterspark(LED14, 9);
    Yukari.masterspark(LED13, 12);
    Yukari.masterspark(LED12, 15);
  }else if(ribbon_R_flag){
    Yukari.masterspark(LED5, 15);
    Yukari.masterspark(LED4, 12);
    Yukari.masterspark(LED3, 9);
    Yukari.masterspark(LED14, 6);
    Yukari.masterspark(LED13, 3);
    Yukari.masterspark(LED12, 0);
  }else{
    Yukari.marisa_twinkle( LED5,  0);
    Yukari.marisa_twinkle( LED4, 20);
    Yukari.marisa_twinkle( LED3, 10);
    Yukari.marisa_twinkle( LED14, 30);
    Yukari.marisa_twinkle(LED13, 40);
    Yukari.marisa_twinkle(LED12, 50);
  }
}
void pattern1() {      // 魔理沙を認識したら同じ光パターンへ
  //  誰かを見つけると裏側のLEDをシアンに光らせる
  if((ribbon_L_flag)||(ribbon_R_flag)){
    Yukari.cyanbreath(LED1);
    Yukari.cyanbreath(LED2);
  }else if (ir_detectflag != 0) {
    Yukari.cyanbreath(LED1);
    Yukari.cyanbreath(LED2);
  } else {
    Yukari.breath(LED1);
    Yukari.mute(LED2);
  }

  //******************************
  
  if(ribbon_L_flag){
    if(BLUE_LED_power < 51 )BLUE_LED_power+=(2+Yukari.brightness);
  }
  if(ribbon_R_flag){
    if(RED_LED_power < 51 )RED_LED_power+=(2+Yukari.brightness);
  }
  //******************************
  
  //傘
  if((ribbon_L_flag)&&(ribbon_R_flag)){
    Yukari.gaming(LED6,  10 * 100);
    Yukari.gaming(LED7,  10 * 60);
    Yukari.gaming(LED8,  10 * 20);
    Yukari.gaming(LED9,  10 * 0);
    Yukari.gaming(LED10, 10 * 40);
    Yukari.gaming(LED11, 10 * 80);
  }else{
    Yukari.icy(LED6, A);
    Yukari.icy(LED7, A);
    Yukari.icy(LED8, A);
    Yukari.icy(LED9, A);
    Yukari.icy(LED10, A);
    Yukari.icy(LED11, A);
  }

  //単色LEDランダムぴかぴか******************************
  int set_LED_POWER;
  if(!random(0, 1000))BLUE_LED_power=50;
  if(!random(0, 1000))RED_LED_power=50;
  //単色LEDランダムぴかぴか******************************


  //すきまお目々
  if((ribbon_L_flag)&&(ribbon_R_flag)){
    Yukari.masterspark(LED5, 0);
    Yukari.masterspark(LED4, 4);
    Yukari.masterspark(LED3, 8);
    Yukari.masterspark(LED14, 10);
    Yukari.masterspark(LED13, 6);
    Yukari.masterspark(LED12, 2);
  }else if(ribbon_L_flag){
    Yukari.masterspark(LED5, 0);
    Yukari.masterspark(LED4, 3);
    Yukari.masterspark(LED3, 6);
    Yukari.masterspark(LED14, 9);
    Yukari.masterspark(LED13, 12);
    Yukari.masterspark(LED12, 15);
  }else if(ribbon_R_flag){
    Yukari.masterspark(LED5, 15);
    Yukari.masterspark(LED4, 12);
    Yukari.masterspark(LED3, 9);
    Yukari.masterspark(LED14, 6);
    Yukari.masterspark(LED13, 3);
    Yukari.masterspark(LED12, 0);
  }else{
    Yukari.marisa_twinkle( LED5,  0);
    Yukari.marisa_twinkle( LED4, 20);
    Yukari.marisa_twinkle( LED3, 10);
    Yukari.marisa_twinkle( LED14, 30);
    Yukari.marisa_twinkle(LED13, 40);
    Yukari.marisa_twinkle(LED12, 50);
  }
}
