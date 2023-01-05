/*!
 * KNMK-0003A Hinaduino向けソースコードs
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

MoePCB Hina(10);  //インスタンス生成（RGBLED数）

//タイマーにより自動で実行
void MoePCB_Task(){
  
  //光らせたいパターンを選んでLEDのIDをセットすると自動で処理
  Hina.breath (LED1);          //LED1:裏面RGBLED
  Hina.autumn(LED5,   0  ); //LED5:秋的光モード
  Hina.autumn(LED4, 200  );
  Hina.autumn(LED3, 200*2);
  Hina.autumn(LED2, 200*3);
  Hina.autumn(LED10,200*4);
  Hina.autumn(LED9, 200*5);
  Hina.autumn(LED8, 200*6);
  Hina.autumn(LED7, 200*7);
  Hina.autumn(LED6, 200*8);
  
  Hina.update();        //計算＆LEDに送信
}

void setup() {
//  Serial.begin(9600);//シリアル通信を使いたいとき

  Hina.begin();//萌基板初期化

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
          switch(random(0,10)){
            case 0:
              Hina.rainbow(LED5, 0,   D);  //LED4:レインボーモード,点灯パターンD（強制キラッ）
            break;
            case 1:
              Hina.rainbow(LED4,60,   D);  //LED3:レインボーモード（位相差60度）
            break;
            case 2:
              Hina.rainbow(LED3,60*2, D);
            break;
            case 3:
              Hina.rainbow(LED2,60*3, D);
            break;
            case 4:
              Hina.rainbow(LED10,60*4, D);
            break;
            case 5:
              Hina.rainbow(LED9,60*5, D);
            break;
            case 6:
              Hina.rainbow(LED8,60*6, D);
            break;
            case 7:
              Hina.rainbow(LED7,60*7, D);
            break;
            case 8:
              Hina.rainbow(LED6,60*8, D);
            break;
          }
        }
      }
    }
  } while (rx.header != 0);
    
    delay(5);
}
