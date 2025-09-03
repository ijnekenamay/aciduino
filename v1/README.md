# Aciduino CV/Gate/Trigger 改造版

このプロジェクトは、元のAciduino v1をMIDI入出力からアナログ電圧（CV、Gate、Trigger）入出力に改造したものです。

## 主な変更点

### 1. 出力信号

- **CV出力**: 2つのトラック用の1V/octave制御電圧出力
- **Gate出力**: 各トラックのノートオン/オフを制御するゲート信号
- **Trigger出力**: クロック、スタート、ストップのトリガーパルス

### 2. 入力信号

- **外部クロック入力**: アナログ電圧による外部クロック同期
- **外部リセット入力**: アナログ電圧によるリセット信号

### 3. 設定可能なパラメータ

- CV電圧範囲: 0-5V
- 基準音: 中央C（C4、2.5V）
- 1V/octave標準準拠
- アッセントとグライドのCV変調（拡張モード）

### 4. 新機能

- **シーケンス方向制御**: 順方向、逆方向、往復、ランダムの4モード
- **CV入力制御**: 外部LFO/CVによるリアルタイムパラメータ変調
- **即興演奏支援**: 演奏中のシーケンス方向変更とパラメータ変調

## ハードウェア接続

### 対応ボード

この改造版は**Arduino Nano**を想定して設計されています。
Arduino Unoでも動作しますが、ピン設定の調整が必要です。

#### 使用部品の機能対応表

| 部品                   | 数量  | 機能           | 対応する操作                   |
| ---------------------- | ----- | -------------- | ------------------------------ |
| Arduino Nano           | 1     | メイン制御     | 全体的なシーケンサー制御       |
| MCP4922 DAC            | 1     | CV電圧出力     | ピッチ制御（1V/octave）        |
| 74HC595                | 1     | LED制御        | 8個のLED表示制御               |
| 74HC165                | 1     | ボタン制御     | 6個のボタン表示制御            |
| B10K可変抵抗器         | 4     | パラメータ制御 | 各モードでのパラメータ調整     |
| プッシュボタン         | 6     | 操作入力       | モード切り替え、トラック選択   |
| LED                    | 8     | 状態表示       | 各種モードとトラック状態の表示 |
| 3.5mmジャック          | 8     | 信号入出力     | CV/Gate/Trigger信号の入出力    |
| TL072オペアンプ        | 1     | 信号調整       | CV信号のバッファリング         |
| LM7805                 | 1     | 電源制御       | 5V安定化電源                   |
| 抵抗・コンデンサ       | 各種  | 回路制御       | 信号調整、ノイズ除去           |
| **CV入力保護回路**     | **-** | **入力保護**   | **負電圧・高電圧からの保護**   |
| 1N4007ダイオード       | 4     | 逆電圧保護     | 負電圧からの保護               |
| 5.1Vツェナーダイオード | 2     | 過電圧保護     | 高電圧からの保護               |
| 10kΩ抵抗               | 4     | 電流制限       | 入力電流の制限                 |
| 100nFコンデンサ        | 2     | ノイズ除去     | 高周波ノイズの除去             |

### 出力ピン（Arduino Nano）

- Track 1 CV（DAC チャネル A） → MCP4922 VOUTA（Arduino ピンではない）
- Track 2 CV（DAC チャネル B） → MCP4922 VOUTB（Arduino ピンではない）
- `D2` : Track 1 Gate 出力
- `D3` : Track 2 Gate 出力
- `D4` : Clock Trigger 出力
- `D5` : Start Trigger 出力
- `D6` : Stop Trigger 出力

### 制御ピン（Arduino Nano）

- `D7` : LED用74HC595 DATA (DS)
- `D8` : LED/ボタンSR 共通 CLOCK (SHCP/CLK)
- `D9` : LED/ボタンSR 共通 LATCH (STCP/PL)
- `D10` : DAC CS（MCP4922 の CS）
- `D11` : MOSI (SPI → DAC SDI)
- `D12` : ボタン用74HC165 DATA (QH → MCU)
- `D13` : SCK (SPI) ← 内蔵LED と共有（注意）
- `A0` : ポテンショメータ入力4
- `A1` : ポテンショメータ入力3
- `A2` : ポテンショメータ入力2
- `A3` : ポテンショメータ入力1

### 入力ピン

- `A6`: 外部CV入力（LFO）
- `A7`: 外部CV入力（モジュレーション）

### DAC接続（MCP4922使用例 - Arduino Nano）

```
Arduino Nano   MCP4922
-------------  -------
Pin 13 (SCK) → SCK
Pin 11 (MOSI) → SDI
Pin 10 (CS) → CS
5V → VDD
GND → VSS
GND → VOUT
```

### LED用シフトレジスタ接続（74HC595使用）

```
Arduino Nano   74HC595
-------------  -------
Pin D7 (Data) → DS (Data)
Pin D8 (Clock) → SHCP (Shift Clock)
Pin D9 (Latch) → STCP (Storage Clock)
5V → VCC
GND → GND
5V → MR (Master Reset) - または10kΩで5Vに接続
```

### LED接続（シフトレジスタ経由）

```
74HC595 Q0 → LED1 → 220Ω → GND
74HC595 Q1 → LED2 → 220Ω → GND
74HC595 Q2 → LED3 → 220Ω → GND
74HC595 Q3 → LED4 → 220Ω → GND
74HC595 Q4 → LED5 → 220Ω → GND
74HC595 Q5 → LED6 → 220Ω → GND
74HC595 Q6 → LED7 → 220Ω → GND
74HC595 Q7 → LED8 → 220Ω → GND
```

### ボタン用シフトレジスタ接続（74HC165使用）

```
Arduino Nano   74HC165
-------------  -------
Pin D12 (Data in MCU) ← QH (Data out)
Pin D8 (Clock) → CLK
Pin D9 (Latch) → PL (Parallel Load)
5V → VCC
GND → GND
PLは通常HIGH、読み出し時にLOWパルス
```

### ボタン接続（74HC165経由）

```
74HC165 D0 ← ボタン1 ← 10kプルアップ → 5V（押下でGND）
74HC165 D1 ← ボタン2 ← 10kプルアップ → 5V（押下でGND）
74HC165 D2 ← ボタン3 ← 10kプルアップ → 5V（押下でGND）
74HC165 D3 ← ボタン4 ← 10kプルアップ → 5V（押下でGND）
74HC165 D4 ← ボタン5 ← 10kプルアップ → 5V（押下でGND）
74HC165 D5 ← ボタン6 ← 10kプルアップ → 5V（押下でGND）
```

### オペアンプ回路（TL072使用例）

#### CV出力回路（Track 1）

```
MCP4921 VOUT → 10kΩ → TL072 Pin 3 (非反転入力)
                    ↓
               100kΩ → TL072 Pin 2 (反転入力)
                    ↓
               10kΩ → GND
                    ↓
TL072 Pin 1 (出力) → 3.5mmジャック
                    ↓
               0.1μF → GND
```

#### Gate出力回路（Track 1）

```
Arduino Pin 14 → 10kΩ → 3.5mmジャック
                    ↓
               0.1μF → GND
```

#### Trigger出力回路（Clock）

```
Arduino Pin 16 → 10kΩ → 3.5mmジャック
                     ↓
                0.1μF → GND
```

### CV入力保護回路

#### LFO入力保護回路（A0）

```
3.5mmジャック → 10kΩ → 1N4007 → 5.1Vツェナー → Arduino A0
     ↓              ↓        ↓         ↓
    GND            GND      GND       GND
                     ↓
                100nF → GND
```

#### 変調入力保護回路（A1）

```
3.5mmジャック → 10kΩ → 1N4007 → 5.1Vツェナー → Arduino A1
     ↓              ↓        ↓         ↓
    GND            GND      GND       GND
                     ↓
                100nF → GND
```

**保護回路の動作原理**:

1. **10kΩ抵抗**: 入力電流を制限（最大10mA）
2. **1N4007ダイオード**: 負電圧をGNDにバイパス
3. **5.1Vツェナーダイオード**: 5.1V以上の電圧をGNDにバイパス
4. **100nFコンデンサ**: 高周波ノイズを除去

### 電源回路

```
外部電源（9-12V） → LM7805 → 5V
                    ↓
               10μF → GND
                    ↓
               0.1μF → GND
```

### 3.5mmジャック配線

#### 出力ジャック

- **CV出力**: ジャックの先端（Tip）にCV信号、スリーブ（Sleeve）にGND
- **Gate出力**: ジャックの先端（Tip）にGate信号、スリーブ（Sleeve）にGND
- **Trigger出力**: ジャックの先端（Tip）にTrigger信号、スリーブ（Sleeve）にGND

#### 入力ジャック

- **外部クロック**: ジャックの先端（Tip）に外部信号、スリーブ（Sleeve）にGND
- **外部リセット**: ジャックの先端（Tip）に外部信号、スリーブ（Sleeve）にGND

## 使用方法

### 基本的なCV/Gate出力

1. シーケンサーを起動
2. 各ステップのノートがCV電圧として出力される
3. ノートオン時にGate信号がHIGHになる
4. ノートオフ時にGate信号がLOWになる

### 外部クロック同期

1. 外部クロック信号をA6ピンに接続
2. 2.5V以上の電圧でクロック信号として認識
3. 内部クロックと同期して動作

### 拡張CVモード

`config.h`で`CV_MODE`を1に設定すると：

- アッセント時にピッチがわずかに上昇
- グライド時にピッチがわずかに下降

### シーケンス方向制御

**操作方法**: ボタン1 + ボタン3を同時押し

- **順方向**: 通常のシーケンス再生
- **逆方向**: 逆順でシーケンス再生
- **往復**: 順方向→逆方向→順方向の往復再生
- **ランダム**: ランダムなステップ選択で再生

### CV入力制御

**LFO入力（A0）**: 外部LFO信号による変調
**変調入力（A1）**: 外部CVによるパラメータ変調

**変調可能パラメータ**:

- **テンポ**: ±50%の範囲でテンポ変調
- **トランスポーズ**: ±12半音の範囲でピッチ変調
- **アッセント強度**: 0-127の範囲でアッセント強度変調
- **グライド時間**: 0-255の範囲でグライド時間変調
- **ノート長**: 1-5の範囲でノート長変調

**CV入力電圧**: 0-5V（2.5Vが中心値）

### CV入力保護回路

**保護対象**:

- **負電圧保護**: -12Vまでの逆電圧を安全に処理
- **過電圧保護**: +12Vまでの高電圧を安全に処理
- **電流制限**: 入力電流を10mA以下に制限
- **ノイズ除去**: 高周波ノイズを除去

**保護レベル**:

- **安全入力範囲**: -12V ～ +12V
- **推奨入力範囲**: 0V ～ +5V
- **最大入力電流**: 10mA
- **保護動作**: 自動的に安全な範囲に制限

## 設定ファイル

### config.h

```cpp
// CV出力設定
#define CV_VOLTAGE_RANGE     5.0    // 5V範囲
#define CV_RESOLUTION        4096   // 12ビットDAC分解能（MCP4921）
#define CV_BASE_NOTE         48     // 中央C
#define CV_BASE_VOLTAGE     2.5     // 中央Cの電圧
#define CV_VOLTS_PER_OCTAVE 1.0     // 1V/octave

// DAC設定（MCP4921）
#define DAC_CS_PIN          10      // DAC Chip Select
#define DAC_SCK_PIN        13      // DAC Serial Clock
#define DAC_MOSI_PIN       11      // DAC Master Out Slave In

// ゲート/トリガー設定
#define GATE_HIGH_VOLTAGE    5.0    // ゲートHIGH電圧
#define GATE_LOW_VOLTAGE     0.0    // ゲートLOW電圧
#define TRIGGER_PULSE_WIDTH  1      // トリガーパルス幅（ms）
```

### HardwareInterface.ino

DAC制御機能とシフトレジスタ制御機能が統合されています：

- `initDAC()`: MCP4921 DACの初期化
- `writeDAC(channel, value)`: 12ビットDAC値の出力
- `outputCV(track, note)`: ノートからCV電圧への変換と出力
- `initShiftRegister()`: 74HC595シフトレジスタの初期化
- `setLED(led_id, state)`: 個別LEDの制御
- `setAllLEDs(pattern)`: 全LEDのパターン制御
- `clearAllLEDs()`: 全LEDの消灯

## 互換性

この改造版は以下の機器との互換性があります：

- モジュラーシンセサイザー（Eurorack等）
- アナログシンセサイザー
- CV/Gate対応のドラムマシン
- その他のアナログ音響機器

## 注意事項

### Arduino Nano特有の注意点

1. **ピン重複**: A4、A5ピンはI2C通信とCV出力で共有されるため、I2Cデバイスを使用する場合は注意
2. **内蔵LED**: D13ピンには内蔵LEDが接続されているため、外部LEDを接続する場合は電流制限に注意
3. **アナログピンのデジタル出力**: A0-A3ピンをデジタル出力として使用する場合、電圧は5Vではなく3.3Vになる場合があります

### 一般的な注意事項

4. **電圧範囲**: 出力電圧は0-5Vの範囲に制限されています
5. **DAC分解能**: MCP4921の12ビットDACを使用（4096段階）
6. **電流制限**: 各出力ピンの電流制限に注意してください
7. **電源**: 安定した5V電源を使用してください
8. **ノイズ対策**: アナログ信号線はデジタル信号線から離して配線してください
9. **接地**: アナログ回路とデジタル回路の接地を適切に分離してください
10. **CV入力保護**: 保護回路を必ず設置し、負電圧や高電圧からの保護を確保してください
11. **入力電圧範囲**: 推奨入力電圧は0-5Vですが、保護回路により-12V～+12Vまで安全に処理できます

## トラブルシューティング

### CV出力が正しく動作しない

- 電源電圧を確認
- 出力ピンの接続を確認
- `config.h`の設定値を確認

### Gate信号が正しく動作しない

- デジタル出力ピンの接続を確認
- 負荷抵抗の値を確認

### 外部クロックが認識されない

- 入力電圧が2.5V以上であることを確認
- 入力ピンの接続を確認

### CV入力が正しく動作しない

- 保護回路の配線を確認
- 入力電圧が推奨範囲（0-5V）内であることを確認
- 保護回路部品（ダイオード、ツェナー）の向きを確認
- 入力電流が10mA以下であることを確認

## 配線図

### 全体配線図

```
[外部電源9-12V] → [LM7805] → [5V]
                           ↓
                    [Arduino Nano]
                           ↓
              ┌─────────────┴─────────────┐
              ↓             ↓             ↓
        [MCP4921 DAC]  [74HC595]    [制御ピン群]
              ↓             ↓             ↓
        [TL072 オペアンプ] [LED群]   [Gate/Trigger]
              ↓             ↓             ↓
        [3.5mmジャック群] [インジケーター] [3.5mmジャック群]
              ↓
        [CV入力保護回路]
              ↓
        [外部CV/LFO入力]
```

### 信号フロー

```
Arduino → DAC → オペアンプ → CV出力
Arduino → シフトレジスタ → LED群
Arduino → 抵抗 → Gate出力
Arduino → 抵抗 → Trigger出力
外部信号 → 保護回路 → 抵抗 → Arduino入力
```

----------------以下は従来の仕様--------------

# Aciduino

Cheap Roland TB303 Step Sequencer clone aimed for live interaction. Your acid lines for a very low cost.

With some user interface changes for cheap construction of a functional TB303 engine we present you the interface:

![Image of aciduino on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/v1/hardware/acid_step_sequencer-protoboard-v002.png)

### So you want a aciduino PCB?

[ijnekenamay](https://github.com/ijnekenamay) has created one for us, you can [download the files](https://github.com/midilab/aciduino/tree/master/v1/hardware/ijnekenamay-PCB) and send it to a fab house, or asking him to sell one for you.

![Image of aciduino on pcb](https://raw.githubusercontent.com/midilab/aciduino/master/v1/hardware/ijnekenamay-PCB/mk2_image5.JPG)

# Features

## 303 Step Sequencer clone

2 Tracks of famous Roland TB303 sequencer with 14 non volatile pattern memory slots to save your work. Programming using rest, tie, glide and accent on each step. The addition of step length parameter per track makes you able to choose from 1 step to 16 steps of pattern length to make some unsual sequences.

## Generative Engine

Generate automatic harmonized acid lines by pressing a single button and twist some harmonic parameter knobs. Just pick up a harmonic mode(scale), the low and high notes to fit the sequence and you have infinite sequences by pressing random it button!

## Midi Controller

Controls your synthetizer acid midi parameters per track up to 6 parameters.

## Midi Clock

Tight and solid MIDI clock to drive your gears clock needs or receive midi sync signal.

# User Interface

4 knobs and 6 buttons divided into 3 pages

When turning your aciduino on you have the page select, the page leds will be blinking indicating that you are on page select mode.

Any time you press button 1 and button 2 together you will be drive to the page select, there you can choose the disered page and track.

You can put you aciduino into slave mode to be in sync with a master clock source by press and hold the play/stop button until his led is ON. To get back to master mode press and hols again until the led is OFF.

Use this information above as reference for aciduino interface

**[page select]**  
knobs ( none ) ( none ) ( none ) ( none )  
buttons [ track 1 ] [ track 2 ] [live mode] [generative] [step edit] [play/stop]

**[page 1 live mode]**  
knobs (ctrl 1 A/B) (ctrl 2 A/B) (ctrl 3 A/B) (length/tunning)  
buttons [ << pattern ] [ pattern >> ] [ ctrl A/B ] [ << tempo ] [ tempo >> ] [play/stop]  
hold [save pattern] [delete pattern] => hold button 1 or button 2 until led blink to complete the action.

**[page 2 generative mode]**  
knobs (low range) (high range) (tones num) (notes num)  
buttons [ << scale ] [ scale >> ] [ generate ] [ << shift ] [ shift >> ] [play/stop]

**[page 3 step edit]**  
knobs (octave) (note) (tunning) (length)  
buttons [ << step ] [ step >> ] [ rest ] [ glide ] [ accent ] [play/stop]

## [page select]: press button1 and button2 together

Just press the desired page button to navigate thru.

Use track 1 and track 2 buttons to select the track to be use. The selected track is the one with led on.

**interface**  
knobs ( none ) ( none ) ( none ) ( none )  
buttons [ track 1 ] [ track 2 ] [live mode] [generative] [step edit] [play/stop]

## [live mode]

The navigation buttons 1 and 2 are used to change pattern, one of the leds will turn on when you reach the navigation barrier - first patterns led 1 on, last pattern led 2 on.

Holding button 1 or 2 for 2 seconds: save pattern, delete and reset pattern

The midi cc commands will be sent to the selected track only.

ctrl A and ctrl B change between knobs midi data to be sent or controlled. The selected ctrl A is the one with led off and ctrl B the led on.

Global tunning lets you go up or down on scale tunning, from -12 to +12 interval.

**interface**  
knobs (ctrl 1 A/B) (ctrl 2 A/B) (ctrl 3 A/B) (length/tunning)  
buttons [ << pattern ] [ pattern >> ] [ ctrl A/B ] [ << tempo ] [ tempo >> ] [play/stop]  
hold [save pattern] [delete pattern] => hold button 1 or button 2 until led blink to complete the action.

## [generative]

Press generate button to get a fresh new sequence on selected track, turning knobs here doesnt change the current sequence, it only sets the parameters for the next time you press generate button.

harmonic scale mode -(previous) and harmonic scale mode +(next) lets you navigate thru harmonic modes(scales). Pressing harmonic mode - all the way until get led on means no harmonic context apply to sequence generation.

By default harmonize is off, just press once harmonic +(next) to get first scale on list, ionian, next again to got to dorian and so on.

Ramdom range low note selects the first note of sequence range notes that can be generated by probabilty. Ramdom range note defines the range of notes that can be generated starting from ramdom low note.

Ramdom range high note selects the barrier to the most high note allowed to be generated on a sequence.

Using shift buttons you can shitf all the sequence steps one step left or rigth using shift navigation buttons.

**interface**  
knobs (low range) (high range) (tones num) (notes num)  
buttons [ << scale ] [ scale >> ] [ generate ] [ << shift ] [ shift >> ] [play/stop]

## [step edit]

Here you can program you acid lines in the same way you program a original one, but instead of keypad for notes you use 2 knobs, one for octave and other for note to get access to all midi range notes spectrum while editing step by step your sequence using rest, glide and accent.

Using step navigation buttons the led on for prev step button means you are have the first step selected for edit, led on for next step button means you have the last step select for edit.

The rest, glide, tie and accent buttons are applied to selected step, led on/off are used to express the state of each parameter per step.

Octave and note knobs lets you select the note for the selected step.

Sequence lenth knob lets you choose from 1 step to 16 steps or lenth for your sequence.

**interface**  
knobs (octave) (note) (tunning) (length)  
buttons [ << step ] [ step >> ] [ rest ] [ glide/tie ] [ accent ] [play/stop]

# Configure for personal needs

Use config.h to hack your environment as you wish.

# Dependencies

This clone is based on uClock library to generate sequencer BPM clock. The library is now part of project to get DIYers a easy compile time.

You can check more info for this library on [github repo](https://github.com/midilab/uClock/)

# BOM List

- 1x arduino nano
- 1x solderless breadboard protoboard 830 tie points 2 busess
- 1x solderless breadboard jumper cable wire kit box
- 6x 220ohms resistors
- 6x 6mm taticle switch
- 6x 3mm diffused leds
- 4x 10k linear potentiometers

## MIDI connection?

Add more two 220ohms resistors to you boom list and choose one of thoses above methods as main midi conection.

- 1x 5 pins DIN female connector (standard MIDI connector) + sto protoboard - you need solder skills here.
- 1x midi cable. Cut it in a half and connect it directly to protoboard - its a good idea to soldering the wire tip before make the connections.

You can also use a MIDI input board to sync the aciduino with a master clock source if needed. Search over the internet for a MIDI input schematic if you want to build it yourself for aciduino.

## USB Connection?

- Open config.h and set MIDI_MODE to 1.
- Download and install a [serial-to-midi converter like](http://projectgus.github.io/hairless-midiserial/)
- Open your favorite DAW, configure MIDI port to get signal from serial-to-midi converter and just go acidulize with your preferred acid synth plugin. Depends on your operational system you may need a additional MIDI virtual port driver installed.
