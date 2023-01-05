/*!
 * KNMK-0003A Hinaduino向けソースコード
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

#define KAMI   A1    //タッチセンシングのポート設定（基板により違うので注意）
#define MUNE   A0
#define SKIRT  A3
#define RIBBON A2


int   kami_offset;  //タッチセンシングの初期値
int   mune_offset;
int   skirt_offset;
int   ribbon_offset;
bool  kami_flag;    //タッチセンシングフラグ
bool  skirt_flag;
bool  ribbon_flag;

uint8_t PATTERN_MODE = 0;//点灯パターン 0-2
uint8_t SUB_PATTERN_MODE = 0;//サブ点灯パターン 0-1

MoePCB Hina(10);  //インスタンス生成（RGBLED数）

//タイマーにより自動で実行
void MoePCB_Task(){

  switch (PATTERN_MODE){
    case 0:
      Hina.breath(LED1);          //LED0:裏面RGBLED
      if(SUB_PATTERN_MODE==0){    //リボンを触るとサブモードが切り替わる
        Hina.autumn(LED5,   0  ); //LED5:秋的光モード
        Hina.autumn(LED4, 200  );
        Hina.autumn(LED3, 200*2);
        Hina.autumn(LED2, 200*3);
        Hina.autumn(LED10,200*4);
        Hina.autumn(LED9, 200*5);
        Hina.autumn(LED8, 200*6);
        Hina.autumn(LED7, 200*7);
        Hina.autumn(LED6, 200*8);
      }else{
        Hina.autumn(LED5,   0  ); //LED5:秋的光モード、色差少なめ
        Hina.autumn(LED4,  20  );
        Hina.autumn(LED3,  20*2);
        Hina.autumn(LED2,  20*3);
        Hina.autumn(LED10, 20*4);
        Hina.autumn(LED9,  20*5);
        Hina.autumn(LED8,  20*6);
        Hina.autumn(LED7,  20*7);
        Hina.autumn(LED6,  20*8);        
      }
      break;
    case 1:
      Hina.breath(LED1);           //LED0:裏面RGBLED
      if(SUB_PATTERN_MODE==0){    //リボンを触るとサブモードが切り替わる
        Hina.rainbow(LED6,  0  ,A);  //LED5:虹色モード
        Hina.rainbow(LED7, 30  ,A);
        Hina.rainbow(LED8, 30*2,A);
        Hina.rainbow(LED9, 30*3,A);
        Hina.rainbow(LED10,30*4,A);
        Hina.rainbow(LED2, 30*5,A);
        Hina.rainbow(LED3, 30*6,A);
        Hina.rainbow(LED4, 30*7,A);
        Hina.rainbow(LED5, 30*8,A);
      }else{
        Hina.rainbow(LED6,  0  ,A);  //LED5:虹色モード
        Hina.rainbow(LED7, 300  ,A);
        Hina.rainbow(LED8, 300*2,A);
        Hina.rainbow(LED9, 300*3,A);
        Hina.rainbow(LED10,300*4,A);
        Hina.rainbow(LED2, 300*5,A);
        Hina.rainbow(LED3, 300*6,A);
        Hina.rainbow(LED4, 300*7,A);
        Hina.rainbow(LED5, 300*8,A);
      }
      break;
    case 2:
      Hina.gaming(LED1, 0);   //LED1:裏面RGBLED
      Hina.gaming(LED6, 0);   //LED5:ゲーミングモード
      Hina.gaming(LED7,20);   //LED4:ゲーミングモード（位相差20度）
      Hina.gaming(LED8,20*2);
      Hina.gaming(LED9,20*3);
      Hina.gaming(LED10,20*4);
      Hina.gaming(LED2,20*5);
      Hina.gaming(LED3,20*6);
      Hina.gaming(LED4,20*7);
      Hina.gaming(LED5,20*8);
      break;
  }

  Hina.update();        //計算＆LEDに送信
}



void setup() {
//  Serial.begin(9600);//シリアル通信を使いたいとき

  Hina.begin();//萌基板初期化

  //タッチセンシングの初期値を登録
  kami_offset   = ADCTouch.read(KAMI,  500);//読むポート, サンプリング数
  mune_offset   = ADCTouch.read(MUNE,  500);
  skirt_offset  = ADCTouch.read(SKIRT, 500);
  ribbon_offset = ADCTouch.read(RIBBON,500);
}


void loop() {

/* CPU内蔵温度感知機能を使う場合は校正した方がいいです（個体差大）
    static uint8_t cnt;
    if(30<cnt){
      cnt=0;
      Serial.print(Hina.cputemp_raw()); Serial.print(" / ");//CPU温度ADC値を見たい時
      Serial.print(Hina.cputemp(26.0, 305.0));//305.0のところは各自の校正値を入れる：校正時の周辺の実測温度、その時のCPU温度ADC値（上段）    
      Serial.println(" C");
    }
    cnt++;
    //50度超えると暑いよ〜
    if(50 < Hina.cputemp(26.0, 305.0) )Hina.heat(true);//cputempに校正値を与えると気温が返ってくる
    else Hina.heat(false);

    //10度下回ると寒いよ〜
    if(Hina.cputemp(26.0, 305.0) < 10 )Hina.cold(true);//cputempに校正値を与えると気温が返ってくる
    else Hina.cold(false);
*/    
   
    int kami_sense   = ADCTouch.read(KAMI,20)   -kami_offset;   //髪の毛タッチ
    int mune_sense   = ADCTouch.read(MUNE,20)   -mune_offset;   //お胸タッチ
    int skirt_sense  = ADCTouch.read(SKIRT,20)  -skirt_offset;  //スカートタッチ
    int ribbon_sense = ADCTouch.read(RIBBON,20) -ribbon_offset; //リボンタッチ

//  タッチセンシングの値を見たいとき
//  Serial.println(kami_sense);
//  Serial.println(mune_sense);
//  Serial.println(skirt_sense);
//  Serial.println(ribbon_sense);

    //髪タッチ検知
    if(50 < kami_sense){
        //フラグがまだ立っていなければ以下を実行
        if(kami_flag==0)Hina.brightness_add();
        kami_flag=1;//フラグを立てることで立ち上がり時のみ実行
    }else kami_flag=0;//離したのでタッチフラグクリア

    //胸タッチ検出
    if(50 < mune_sense)Hina.angry(true);
    else Hina.angry(false);

    //スカートタッチ検出
    if(50 < skirt_sense){
        //フラグがまだ立っていなければ以下を実行
        if(skirt_flag==0){          
          Hina.acknowledge();   //了解コール
          SUB_PATTERN_MODE=0;   //サブモードを戻しておく
          if (PATTERN_MODE == 0)     PATTERN_MODE = 1;
          else if(PATTERN_MODE == 1) PATTERN_MODE = 2;
          else                       PATTERN_MODE = 0;
        }
        skirt_flag=1;//フラグを立てることで立ち上がり時のみ実行
    }else{
        skirt_flag=0;//離したのでタッチフラグクリア
    }
    
    //リボンタッチ検出
    if(PATTERN_MODE!=2){//ゲーミング時はリボン検出しない
      if(50 < ribbon_sense){
          //フラグがまだ立っていなければ以下を実行
          if(ribbon_flag==0){          
            Hina.acknowledge();//了解コール
            if (SUB_PATTERN_MODE == 0) SUB_PATTERN_MODE = 1;
            else                       SUB_PATTERN_MODE = 0;
          }
          ribbon_flag=1;//フラグを立てることで立ち上がり時のみ実行
      }else{
          ribbon_flag=0;//離したのでタッチフラグクリア
      }
    }
    
    delay(15);
}
