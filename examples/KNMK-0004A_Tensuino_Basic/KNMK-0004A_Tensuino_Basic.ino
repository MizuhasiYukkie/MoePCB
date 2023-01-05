/*!
 * KNMK-0004A Tensuino向けソースコード
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
 * ※USB端子経由で書き込めない場合はICSP端子からAVRISP mkⅡなどを用いて書き込んでください。
 */
 
#include <MoePCB.h>
#include <ADCTouch.h>

#define KAMI   A0    //タッチセンシングのポート設定（基板により違うので注意）
#define MUNE   A1
#define SKIRT  A3
#define TSURUGI A2


int   kami_offset;  //タッチセンシングの初期値
int   mune_offset;
int   skirt_offset;
int   tsurugi_offset;
bool  kami_flag;    //タッチセンシングフラグ
bool  skirt_flag;
bool  tsurugi_flag;

uint8_t PATTERN_MODE = 0;//点灯パターン 0-2
uint8_t SUB_PATTERN_MODE = 0;//サブ点灯パターン 0-1

MoePCB Tenshi(16);  //インスタンス生成（RGBLED数）

//タイマーにより自動で実行
void MoePCB_Task(){

  switch (PATTERN_MODE){
    case 0:
      Tenshi.breath(LED1);          //LED0:裏面RGBLED
      Tenshi.rainbow(LED2,  0  ,A); //LED1:虹色光モード
      Tenshi.rainbow(LED3, 60  ,A); //LED2:虹色光モード、位相差40
      Tenshi.rainbow(LED4, 60*2,A);
      Tenshi.rainbow(LED5, 60*3,A);
      Tenshi.rainbow(LED6, 60*4,A);
      //剣を触るとサブモードが切り替わる
      if     (SUB_PATTERN_MODE==0)for(int i=0;i<10;i++) Tenshi.sword(i+6,28*i,A,A);
      else if(SUB_PATTERN_MODE==1)for(int i=0;i<10;i++) Tenshi.sword(i+6,28*i,A,B);
      break;
    case 1:
      Tenshi.breath(LED1);          //LED0:裏面RGBLED
      Tenshi.rainbow(LED2,  0  ,B); //LED1:虹色光モード
      Tenshi.rainbow(LED3, 60  ,B); //LED2:虹色光モード、位相差40
      Tenshi.rainbow(LED4, 60*2,B);
      Tenshi.rainbow(LED5, 60*3,B);
      Tenshi.rainbow(LED6, 60*4,B);
      //剣を触るとサブモードが切り替わる
      if     (SUB_PATTERN_MODE==0)for(int i=0;i<10;i++) Tenshi.sword(i+6,28*i,B,B);
      else if(SUB_PATTERN_MODE==1)for(int i=0;i<10;i++) Tenshi.sword(i+6,28*i,B,A);
      break;
    case 2:
      Tenshi.gaming(LED1, 0);   //LED1:裏面RGBLED
      Tenshi.gaming(LED2, 0);   //LED5:ゲーミングモード
      Tenshi.gaming(LED3,10);   //LED4:ゲーミングモード（位相差20度）
      Tenshi.gaming(LED4,10*2);
      Tenshi.gaming(LED5,10*3);
      Tenshi.gaming(LED6,10*4);
      for(int i=0;i<10;i++) Tenshi.gaming(i+6,50+10*i);
      break;
  }

  Tenshi.update();        //計算＆LEDに送信
}



void setup() {
//  Serial.begin(9600);//シリアル通信を使いたいとき

  Tenshi.begin();//萌基板初期化

  //タッチセンシングの初期値を登録
  kami_offset   = ADCTouch.read(KAMI,  500);//読むポート, サンプリング数
  mune_offset   = ADCTouch.read(MUNE,  500);
  skirt_offset  = ADCTouch.read(SKIRT, 500);
  tsurugi_offset = ADCTouch.read(TSURUGI,500);
}


void loop() {

/* CPU内蔵温度感知機能を使う場合は校正した方がいいです（個体差大）
    static uint8_t cnt;
    if(30<cnt){
      cnt=0;
      Serial.print(Tenshi.cputemp_raw()); Serial.print(" / ");//CPU温度ADC値を見たい時
      Serial.print(Tenshi.cputemp(26.0, 305.0));//305.0のところは各自の校正値を入れる：校正時の周辺の実測温度、その時のCPU温度ADC値（上段）    
      Serial.println(" C");
    }
    cnt++;
    //50度超えると暑いよ〜
    if(50 < Tenshi.cputemp(26.0, 305.0) )Tenshi.heat(true);//cputempに校正値を与えると気温が返ってくる
    else Tenshi.heat(false);

    //10度下回ると寒いよ〜
    if(Tenshi.cputemp(26.0, 305.0) < 10 )Tenshi.cold(true);//cputempに校正値を与えると気温が返ってくる
    else Tenshi.cold(false);
*/    
   
    int kami_sense   = ADCTouch.read(KAMI,20)   -kami_offset;   //髪の毛タッチ
    int mune_sense   = ADCTouch.read(MUNE,20)   -mune_offset;   //お胸タッチ
    int skirt_sense  = ADCTouch.read(SKIRT,20)  -skirt_offset;  //スカートタッチ
    int tsurugi_sense = ADCTouch.read(TSURUGI,20) -tsurugi_offset; //剣タッチ

//  タッチセンシングの値を見たいとき
//  Serial.println(kami_sense);
//  Serial.println(mune_sense);
//  Serial.println(skirt_sense);
//  Serial.println(tsurugi_sense);

    //髪タッチ検知
    if(50 < kami_sense){
        //フラグがまだ立っていなければ以下を実行
        if(kami_flag==0)Tenshi.brightness_add();
        kami_flag=1;//フラグを立てることで立ち上がり時のみ実行
    }else kami_flag=0;//離したのでタッチフラグクリア

    //胸タッチ検出
    if(50 < mune_sense)Tenshi.angry(true);
    else Tenshi.angry(false);

    //スカートタッチ検出
    if(50 < skirt_sense){
        //フラグがまだ立っていなければ以下を実行
        if(skirt_flag==0){          
          Tenshi.acknowledge();   //了解コール
          SUB_PATTERN_MODE=0;   //サブモードを戻しておく
          if (PATTERN_MODE == 0)     PATTERN_MODE = 1;
          else if(PATTERN_MODE == 1) PATTERN_MODE = 2;
          else                       PATTERN_MODE = 0;
        }
        skirt_flag=1;//フラグを立てることで立ち上がり時のみ実行
    }else{
        skirt_flag=0;//離したのでタッチフラグクリア
    }
    
    //剣タッチ検出
    if(PATTERN_MODE!=2){//ゲーミング時は剣検出しない
      if(50 < tsurugi_sense){
          //フラグがまだ立っていなければ以下を実行
          if(tsurugi_flag==0){
            if (SUB_PATTERN_MODE == 0) SUB_PATTERN_MODE = 1;
            else                       SUB_PATTERN_MODE = 0;
          }
          delay(200);//チャタリング対策
          tsurugi_flag=1;//フラグを立てることで立ち上がり時のみ実行
      }else{
          tsurugi_flag=0;//離したのでタッチフラグクリア
      }
    }
    
    delay(15);
}
