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


## 設計 (ライブラリ実装者向け説明)

### 基本の設計戦略
ライブラリ利用者が以下のようなコードを書けることを目指しています。
```
MoePCB_Core<基板設定, モジュール1, モジュール2, ...> pcb;
pcb.get_XXX_module().set_YYY(ZZZ);
```

MoePCB_Core は萌基板全体制御クラスです。<br>
このクラスはテンプレートクラスになっていて、以下で説明する基板設定クラスとモジュールクラスを与えることで、
利用者にとって最適な萌基板クラスとして利用できるようになります。

基板設定には、FraduinoやCirduinoなどそれぞれの基板ごとに固有の設定値をまとめたクラスを与えます。<br>
(基板設定クラスはライブラリで提供する)

モジュールには、LEDやタッチセンサなど、萌基板に実装されている周辺回路の制御を行うクラスを与えます。<br>
一部のモジュールクラスはそれ自身もテンプレートクラスとなっており、
サブモジュールを与えてより詳細な機能を制御可能なものもあります。

### モジュール
現時点で実装済みのモジュールは以下の通りです。

* LED: メインLED (フルカラーLED)
* SubLED: サブLED (背面単色LED)
* TouchSensor: タッチセンサ
* Thermometer: 温度センサ (マイコン内蔵温度センサ)

TODO: 今後実装していくモジュール
* 環境センサ (Hinaduino, Tensuinoで利用)
* 赤外線 (Patchoulino, Mariduino, Yukarinoで利用)

### 命名規則案

統一しないと後々大変になるので、思い付きで一通り決めました。<br>
TODO: まだ全体に反映できていない + Arduino界隈のお作法を知らないため、規則を修正したうえでコードを直すかもです。

* クラス・テンプレートパラメータ => upper camel case + snake case
* 型 => lower camel case
* private/protected メンバ変数 => lower snake case
* public メンバ変数 => upper snake case
* static メンバ関数 => upper camel case
* private/protected メンバ関数 => lower camel case
* public メンバ関数 => upper camel case
* 関数引数/local変数 => lower camel case

以下サンプルコード
```
template<typename T_TemplateTypeName_SubName>
class MoePCB_ClassName_SubName {
  friend class MoePCB_FriendClassName;

private:
  typedef int16_t private_new_type_name_t;
  static constexpr int16_t private_static_constexpr_variable;

public:
  typedef int16_t public_new_type_name_t;
  static constexpr int16_t Public_Static_Constexpr_Variable;

public:
  static void PublicStaticMethod(int16_t methodArgument) { }

public:
  MoePCB_ClassName_SubName() = default;
  MoePCB_ClassName_SubName(const MoePCB_ClassName_SubName&) = delete;
  MoePCB_ClassName_SubName(MoePCB_ClassName_SubName&&) = delete;
  MoePCB_ClassName_SubName& operator= (const MoePCB_ClassName_SubName&) = delete;
  MoePCB_ClassName_SubName& operator= (MoePCB_ClassName_SubName&&) = delete;
  ~MoePCB_ClassName_SubName() = default;

private:
  void privateMethod(int16_t methodArgument) const { }

public:
  void PublicMethod(int16_t methodArgument) const { }

public:
  int16_t Public_Member;

private:
  int16_t private_member;
};
```
