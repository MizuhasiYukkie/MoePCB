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
 * 【Adafruit_NeoPixel】 https://github.com/adafruit/Adafruit_NeoPixel
 * 【MoePCB】 https://github.com/MizuhasiYukkie/MoePCB
 * 【MIDIUSB】 https://github.com/arduino-libraries/MIDIUSB
 * 
 * ツール→ボード："Arduino Leonardo"を選択
 * ツール→シリアルポート：「USBで接続済みの萌基板のポート」（ArduinoLeonardoと表示されます）を選択
 * 「マイコンボードに書き込む」を実行するとここに書いてあるプログラムが書き込まれます。
 * ※USB端子経由で書き込めない場合はICSP端子からAVRISP mkⅡなどを用いて書き込んでください。
 */
 
#include <MoePCB.h>
#include <MIDIUSB.h>

MoePCB Fran(7);  //インスタンス生成（RGBLED数）

//タイマーにより自動で実行
void MoePCB_Task(){
  
  //光らせたいパターンを選んでLEDのIDをセットすると自動で処理
  Fran.breath (LED1);          //LED1:裏面RGBLED
  Fran.rainbow(LED4, 0,   C);  //LED4:レインボーモード,点灯パターンC（キラキラしない）
  Fran.rainbow(LED3,60,   C);  //LED3:レインボーモード（位相差60度）
  Fran.rainbow(LED2,60*2, C);
  Fran.rainbow(LED5,60*3, C);
  Fran.rainbow(LED6,60*4, C);
  Fran.rainbow(LED7,60*5, C);
  
  Fran.update();        //計算＆LEDに送信
}

void setup() {
//  Serial.begin(9600);//シリアル通信を使いたいとき

  Fran.begin();//萌基板初期化

}


void loop() {

  //MIDI受信
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) {
/*
    //受信データを見たい場合
    Serial.print("Received: ");
    Serial.print(rx.header);
    Serial.print("-");
    Serial.print(rx.byte1);
    Serial.print("-");
    Serial.print(rx.byte2);
    Serial.print("-");
    Serial.println(rx.byte3);
*/
      if(rx.header==9){//まずノートオンのみに絞る（９）
        if(rx.byte3!=0){//ベロシティがゼロではない時（ノートオン＋ベロシティ０でノートオフとする機材もあるため）
          switch(random(0,5)){
            case 0:
              Fran.rainbow(LED4, 0,   D);  //LED4:レインボーモード,点灯パターンD（強制キラッ）
            break;
            case 1:
              Fran.rainbow(LED3,60,   D);  //LED3:レインボーモード（位相差60度）
            break;
            case 2:
              Fran.rainbow(LED2,60*2, D);
            break;
            case 3:
              Fran.rainbow(LED5,60*3, D);
            break;
            case 4:
              Fran.rainbow(LED6,60*4, D);
            break;
            case 5:
              Fran.rainbow(LED7,60*5, D);
            break;
          }
        }
      }
    }
  } while (rx.header != 0);
    
    delay(5);
}
