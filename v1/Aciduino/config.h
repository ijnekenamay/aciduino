#ifndef __CONFIG_H__
#define __CONFIG_H__

//
// CV/Gate/Trigger Config
//
// CV_MODE(0) for standard CV/Gate/Trigger output
// CV_MODE(1) for extended CV output with accent and glide
#define CV_MODE              0

// CV Output Configuration
#define CV_VOLTAGE_RANGE     5.0    // 5V range
#define CV_RESOLUTION        4096   // 12-bit DAC resolution (MCP4922)
#define CV_REFERENCE_VOLTAGE 5.0    // Reference voltage

// Note to CV conversion (1V/octave standard)
#define CV_BASE_NOTE         48     // Middle C (C4)
#define CV_BASE_VOLTAGE     2.5     // 2.5V for middle C
#define CV_VOLTS_PER_OCTAVE 1.0     // 1V per octave

// DAC Configuration (MCP4922)
#define DAC_CS_PIN          10      // DAC Chip Select pin
#define DAC_SCK_PIN        13      // DAC Serial Clock pin
#define DAC_MOSI_PIN       11      // DAC Master Out Slave In pin

// Gate/Trigger Configuration
#define GATE_HIGH_VOLTAGE    5.0    // Gate high voltage
#define GATE_LOW_VOLTAGE     0.0    // Gate low voltage
#define TRIGGER_PULSE_WIDTH  1      // Trigger pulse width in milliseconds

// Track Configuration
#define TRACK1_CHANNEL      1
#define TRACK2_CHANNEL      2

//
// CV Controller config (replaces MIDI controller)
//
#define USE_CV_CTRL
#define CV_CTRL_TUNNING     79
#define CV_CTRL_CUTOFF      80
#define CV_CTRL_RESONANCE   81
#define CV_CTRL_ENVMOD      82
#define CV_CTRL_DECAY       83
#define CV_CTRL_ACCENT      84
#define CV_CTRL_WAVE        85

//
// User interface config
//
#define SEQUENCER_MIN_BPM     50
#define SEQUENCER_MAX_BPM     177

//
// Generative config
//
#define ACCENT_PROBABILITY_GENERATION   50
#define GLIDE_PROBABILITY_GENERATION    30
#define TIE_PROBABILITY_GENERATION      80
#define REST_PROBABILITY_GENERATION     10

//
// Sequencer config
//
#define TRACK_NUMBER       2 // you can go up to 8 but no interface ready to control it
#define STEP_MAX_SIZE      16
#define NOTE_LENGTH        3 // min: 1 max: 5 DO NOT EDIT BEYOND!!!
#define NOTE_VELOCITY      90
#define ACCENT_VELOCITY    127

// Sequence Direction Modes
#define SEQUENCE_FORWARD    0  // 順方向
#define SEQUENCE_REVERSE    1  // 逆方向
#define SEQUENCE_PINGPONG   2  // 往復
#define SEQUENCE_RANDOM     3  // ランダム
#define SEQUENCE_MODES      4  // モード数

//
// Hardware config
//
#define POT_NUMBER    4
#define BUTTON_NUMBER 6

// Hardware config
#define ADC_RESOLUTION        1024
#define POT_SENSITIVITY       2

// Pin configuration(double check your schematic before configure those pins)
// Pots
#define GENERIC_POT_1_PIN     A3
#define GENERIC_POT_2_PIN     A2
#define GENERIC_POT_3_PIN     A1
#define GENERIC_POT_4_PIN     A0


  // Shift Register Configuration
  // LED: 74HC595 (output) / BUTTON: 74HC165 (input)
  #define LED_SHIFT_DATA_PIN        7       // D7 - 74HC595 DS
  #define LED_SHIFT_CLOCK_PIN       8       // D8 - 74HC595 SHCP
  #define LED_SHIFT_LATCH_PIN       9       // D9 - 74HC595 STCP
  // Share clock/latch with LED SR to save pins; BUTTON data uses D12 (MISO)
  #define BUTTON_SHIFT_DATA_PIN     12      // D12 - 74HC165 QH (DATA to MCU)
  #define BUTTON_SHIFT_CLOCK_PIN    LED_SHIFT_CLOCK_PIN
  #define BUTTON_SHIFT_LATCH_PIN    LED_SHIFT_LATCH_PIN

// LED Configuration (via Shift Register)
#define LED_COUNT             8       // シフトレジスタで制御するLED数
#define LED_SHIFT_MASK        0xFF    // 8ビットマスク

// Individual LED definitions (for code compatibility)
#define GENERIC_LED_1         0       // LED 1 (シフトレジスタ bit 0)
#define GENERIC_LED_2         1       // LED 2 (シフトレジスタ bit 1)
#define GENERIC_LED_3         2       // LED 3 (シフトレジスタ bit 2)
#define GENERIC_LED_4         3       // LED 4 (シフトレジスタ bit 3)
#define GENERIC_LED_5         4       // LED 5 (シフトレジスタ bit 4)
#define GENERIC_LED_6         5       // LED 6 (シフトレジスタ bit 5)
#define GENERIC_LED_7         6       // LED 7 (シフトレジスタ bit 6)
#define GENERIC_LED_8         7       // LED 8 (シフトレジスタ bit 7)

// Individual BUTTON definitions (for code compatibility)
#define GENERIC_BUTTON_1         0       // ボタン 1 (シフトレジスタ bit 0)
#define GENERIC_BUTTON_2         1       // ボタン 2 (シフトレジスタ bit 1)
#define GENERIC_BUTTON_3         2       // ボタン 3 (シフトレジスタ bit 2)
#define GENERIC_BUTTON_4         3       // ボタン 4 (シフトレジスタ bit 3)
#define GENERIC_BUTTON_5         4       // ボタン 5 (シフトレジスタ bit 4)
#define GENERIC_BUTTON_6         5       // ボタン 6 (シフトレジスタ bit 5)

// CV/Gate/Trigger Output Pins (Arduino Nano)
#define GATE_TRACK1_PIN      2       // Gate output for Track 1 (D2)
#define GATE_TRACK2_PIN      3       // Gate output for Track 2 (D3)
#define TRIGGER_CLOCK_PIN    4       // Clock trigger output (D4)
#define TRIGGER_START_PIN    5       // Start trigger output (D5)
#define TRIGGER_STOP_PIN     6       // Stop trigger output (D6)

// CV Input Pins
#define CV_CLOCK_IN_PIN      A4
#define CV_RESET_IN_PIN      A5
#define CV_LFO_IN_PIN        A6      // External LFO/CV input
#define CV_MOD_IN_PIN        A7      // External modulation CV input

#define CV_INPUT_VOLTAGE_RANGE 5.0   // CV入力電圧範囲
#define CV_INPUT_RESOLUTION    1024  // CV入力分解能（10ビット）
#define CV_INPUT_THRESHOLD     512   // CV入力閾値（2.5V相当）

// CV Modulation Parameters
#define CV_MOD_TEMPO        0  // テンポ変調
#define CV_MOD_TRANSPOSE    1  // トランスポーズ変調
#define CV_MOD_ACCENT       2  // アッセント強度変調
#define CV_MOD_GLIDE        3  // グライド時間変調
#define CV_MOD_LENGTH       4  // ノート長変調
#define CV_MOD_MODES        5  // 変調モード数

typedef enum {
  GENERIC_POT_1,
  GENERIC_POT_2,
  GENERIC_POT_3,
  GENERIC_POT_4
} POT_HARDWARE_INTERFACE;

typedef enum {
  BUTTON_1,
  BUTTON_2,
  BUTTON_3,
  BUTTON_4,
  BUTTON_5,
  BUTTON_6
} BUTTON_HARDWARE_INTERFACE;

typedef enum {
  LED_1,
  LED_2,
  LED_3,
  LED_4,
  LED_5,
  LED_6,
  LED_7,
  LED_8
} LED_HARDWARE_INTERFACE;

#endif
