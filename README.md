## MoePCB  
MoePCB は "萌基板" を誰でも扱えるようにするためのライブラリです。

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

回路図などの参考資料は [こちら](https://github.com/MizuhasiYukkie/MOE-PCB)

萌基板の詳細は [こちら](http://yuki-factory.main.jp/moe-pcb.html)
