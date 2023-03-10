## MoePCB  
MoePCB は  "[萌基板](https://fox-factory.booth.pm/item_lists/8aYTRoR8)" を誰でも扱えるようにするためのライブラリです。

## 対応機種  
KNMK...からはじまる萌基板シリーズに対応しています。

## 使い方  
`Fran.cputemp_raw();`  
* CPU内蔵温度検知を使用するには一度ADCの生データを取得します。例：返り値305.0

`Fran.cputemp(26.0, 305.0);`  
* その際の室温（例26.0度）を合わせて与えると大体のCPU温度が返ってきます。


## 動作条件    
このライブラリの動作には以下のライブラリが必要です:

[Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)

タッチセンシングを使用する際は
[ADCTouch](https://github.com/martin2250/ADCTouch)

USB-MIDIを使用する際は
[MIDIUSB](https://github.com/arduino-libraries/MIDIUSB)

## 書き込み方法
ツール → ボード → Arduino AVR Boards → Arduino Leonardo を選択  
  
方法１：USBポートに萌基板差し込み選択、左上の矢印２番目「マイコンボードに書き込む」。うまくいかない場合は別売りのAVRISP mkⅡを用意し方法２へ  
  
方法２：萌基板のICSP端子とAVRISPの各端子を接続（はんだ付けする、もしくは6Pクリップを用意）→書き込み装置を使って書き込む  
  
調子が悪い、書き込めない場合は一度”ブートローダーを書き込む”を実行するとUSBからの書き込みを含めて復活することがあります。

## 参考資料  
回路図など [こちら](https://github.com/MizuhasiYukkie/MOE-PCB)

萌基板の紹介ページは [こちら](http://yuki-factory.main.jp/moe-pcb.html)
