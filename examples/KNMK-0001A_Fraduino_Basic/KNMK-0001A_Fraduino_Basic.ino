/*!
 * KNMK-0001A Fraduino向けソースコード
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

#define KAMI  A0    //タッチセンシングのポート設定（基板により違うので注意）
#define MUNE  A1
#define SKIRT A2

int   kami_offset;  //タッチセンシングの初期値
int   mune_offset;
int   skirt_offset;
bool  kami_flag;    //タッチセンシングフラグ
bool  skirt_flag;

uint8_t PATTERN_MODE = 0;//点灯パターン 0-2

MoePCB Fran(7);  //インスタンス生成（RGBLED数）

//タイマーにより自動で実行
void MoePCB_Task(){

  switch (PATTERN_MODE){
    case 0:
      //光らせたいパターンを選んでLEDのIDをセットすると自動で処理
      Fran.breath (LED1);          //LED1:裏面RGBLED
      Fran.rainbow(LED4, 0,   A);  //LED4:レインボーモード,点灯パターンA（自動でキラキラ）
      Fran.rainbow(LED3,60,   A);  //LED3:レインボーモード（位相差60度）
      Fran.rainbow(LED2,60*2, A);
      Fran.rainbow(LED5,60*3, A);
      Fran.rainbow(LED6,60*4, A);
      Fran.rainbow(LED7,60*5, A);
      break;
    
    case 1:
      Fran.breath (LED1);          //LED1:裏面RGBLED
      Fran.rainbow(LED4, 0,   B);  //LED4:レインボーモード,点灯パターンB（少しデジタル感）
      Fran.rainbow(LED3,60,   B);  //LED3:レインボーモード（位相差60度）
      Fran.rainbow(LED2,60*2, B);
      Fran.rainbow(LED5,60*3, B);
      Fran.rainbow(LED6,60*4, B);
      Fran.rainbow(LED7,60*5, B);
      break;
    
    case 2:
      Fran.gaming(LED1, 0);   //LED1:裏面RGBLED
      Fran.gaming(LED4, 0);   //LED4:ゲーミングモード
      Fran.gaming(LED3,20);   //LED3:ゲーミングモード（位相差20度）
      Fran.gaming(LED2,20*2);
      Fran.gaming(LED5,20*3);
      Fran.gaming(LED6,20*4);
      Fran.gaming(LED7,20*5);
      break;
  }

  Fran.update();        //計算＆LEDに送信
}



void setup() {
//  Serial.begin(9600);//シリアル通信を使いたいとき

  Fran.begin();//萌基板初期化

  //タッチセンシングの初期値を登録
  kami_offset  = ADCTouch.read(KAMI, 500);//読むポート, サンプリング数
  mune_offset  = ADCTouch.read(MUNE, 500);
  skirt_offset = ADCTouch.read(SKIRT,500);
}


void loop() {

/* CPU内蔵温度感知機能を使う場合は校正した方がいいです（個体差大）
    static uint8_t cnt;
    if(30<cnt){
      cnt=0;
      Serial.print(Fran.cputemp_raw()); Serial.print(" / ");//CPU温度ADC値を見たい時
      Serial.print(Fran.cputemp(26.0, 305.0));//305.0のところは各自の校正値を入れる：校正時の周辺の実測温度、その時のCPU温度ADC値（上段）    
      Serial.println(" C");
    }
    cnt++;
    //50度超えると暑いよ〜
    if(50 < Fran.cputemp(26.0, 305.0) )Fran.heat(true);//cputempに校正値を与えると気温が返ってくる
    else Fran.heat(false);

    //10度下回ると寒いよ〜
    if(Fran.cputemp(26.0, 305.0) < 10 )Fran.cold(true);//cputempに校正値を与えると気温が返ってくる
    else Fran.cold(false);
*/    
   
    int kami_sense  = ADCTouch.read(KAMI,20)  -kami_offset;   //髪の毛タッチ
    int mune_sense  = ADCTouch.read(MUNE,20)  -mune_offset;   //お胸タッチ
    int skirt_sense = ADCTouch.read(SKIRT,20) -skirt_offset;  //スカートタッチ

//  タッチセンシングの値を見たいとき
//  Serial.println(kami_sense);
//  Serial.println(mune_sense);
//  Serial.println(skirt_sense);

    //髪タッチ検知
    if(50 < kami_sense){
        //フラグがまだ立っていなければ以下を実行
        if(kami_flag==0)Fran.brightness_add();
        kami_flag=1;//フラグを立てることで立ち上がり時のみ実行
    }else kami_flag=0;//離したのでタッチフラグクリア

    //胸タッチ検出
    if(50 < mune_sense)Fran.angry(true);
    else Fran.angry(false);

    //スカートタッチ検出
    if(50 < skirt_sense){
        //フラグがまだ立っていなければ以下を実行
        if(skirt_flag==0){          
          Fran.acknowledge();//了解コール
          if (PATTERN_MODE == 0)     PATTERN_MODE = 1;
          else if(PATTERN_MODE == 1) PATTERN_MODE = 2;
          else                       PATTERN_MODE = 0;
        }
        skirt_flag=1;//フラグを立てることで立ち上がり時のみ実行
    }else{
        skirt_flag=0;//離したのでタッチフラグクリア
    }
    
    delay(15);
}
