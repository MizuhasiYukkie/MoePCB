/*!
 * KNMK-0002A Cirduino向けソースコード
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

#define KAMI  A1    //タッチセンシングのポート設定（基板により違うので注意）
#define MUNE  A0
#define SKIRT A2

int   kami_offset;  //タッチセンシングの初期値
int   mune_offset;
int   skirt_offset;
bool  kami_flag;    //タッチセンシングフラグ
bool  skirt_flag;

uint8_t PATTERN_MODE = 0;//点灯パターン 0-2

MoePCB Cirno(7);  //インスタンス生成（RGBLED数）

//タイマーにより自動で実行
void MoePCB_Task(){

  switch (PATTERN_MODE){
    case 0:
      Cirno.breath(LED1); //LED1:裏面RGBLED
      Cirno.icy(LED4,A);    //LED4:ひんやり光モード
      Cirno.icy(LED3,A);    //LED3:
      Cirno.icy(LED2,A);
      Cirno.icy(LED5,A);
      Cirno.icy(LED6,A);
      Cirno.icy(LED7,A);
      break;
    
    case 1:
      Cirno.breath(LED1); //LED1:裏面RGBLED
      Cirno.icy(LED4,B);    //LED4:ひんやり光モード
      Cirno.icy(LED3,B);    //LED3:
      Cirno.icy(LED2,B);
      Cirno.icy(LED5,B);
      Cirno.icy(LED6,B);
      Cirno.icy(LED7,B);
      break;
    
    case 2:
      Cirno.gaming(LED1, 0);   //LED1:裏面RGBLED
      Cirno.gaming(LED4, 0);   //LED4:ゲーミングモード
      Cirno.gaming(LED3,20);   //LED3:ゲーミングモード（位相差20度）
      Cirno.gaming(LED2,20*2);
      Cirno.gaming(LED5,20*3);
      Cirno.gaming(LED6,20*4);
      Cirno.gaming(LED7,20*5);
      break;
  }

  Cirno.update();        //計算＆LEDに送信
}



void setup() {
//  Serial.begin(9600);//シリアル通信を使いたいとき

  Cirno.begin();//萌基板初期化

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
      Serial.print(Cirno.cputemp_raw()); Serial.print(" / ");//CPU温度ADC値を見たい時
      Serial.print(Cirno.cputemp(26.0, 305.0));//305.0のところは各自の校正値を入れる：校正時の周辺の実測温度、その時のCPU温度ADC値（上段）    
      Serial.println(" C");
    }
    cnt++;
    //50度超えると暑いよ〜
    if(50 < Cirno.cputemp(26.0, 305.0) )Cirno.heat(true);//cputempに校正値を与えると気温が返ってくる
    else Cirno.heat(false);

    //10度下回ると寒いよ〜
    if(Cirno.cputemp(26.0, 305.0) < 10 )Cirno.cold(true);//cputempに校正値を与えると気温が返ってくる
    else Cirno.cold(false);
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
        if(kami_flag==0)Cirno.brightness_add();
        kami_flag=1;//フラグを立てることで立ち上がり時のみ実行
    }else kami_flag=0;//離したのでタッチフラグクリア

    //胸タッチ検出
    if(50 < mune_sense)Cirno.angry(true);
    else Cirno.angry(false);

    //スカートタッチ検出
    if(50 < skirt_sense){
        //フラグがまだ立っていなければ以下を実行
        if(skirt_flag==0){          
          Cirno.acknowledge();//了解コール
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
