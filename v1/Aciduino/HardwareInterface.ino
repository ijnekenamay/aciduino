#include <EEPROM.h>
#include <SPI.h>

// Shift Register Control Variables
uint8_t _led_state = 0x00;  // Current LED state (8 bits)

// Sequence Direction Control Variables
uint8_t _sequence_direction = SEQUENCE_FORWARD;  // Current sequence direction
bool _pingpong_forward = true;  // Ping-pong direction flag
uint8_t _last_random_step = 0;  // Last random step for random mode

// CV Input Control Variables
uint16_t _cv_lfo_value = 0;     // Current LFO CV value
uint16_t _cv_mod_value = 0;     // Current modulation CV value
uint8_t _cv_mod_mode = CV_MOD_TEMPO;  // Current CV modulation mode

// pot data
typedef struct
{
  uint8_t pin;
  uint16_t state;
  bool lock;
} POT_DATA;

// button data
typedef struct
{
  uint8_t pin;
  bool state;
  uint8_t hold_seconds;
  bool hold_trigger;
} BUTTON_DATA;

POT_DATA _pot[POT_NUMBER];
BUTTON_DATA _button[BUTTON_NUMBER];

// pattern memory layout 72 bytes(2 tracks with 16 steps)
// byte1: pattern_exist
// byte2: _transpose
// byte3: _harmonize
// byte4: _selected_mode
// byte5...: _sequencer[0].data, _sequencer[1].data
// EEPROM data access: 1024 bytes total
// total 14 patterns
// use for load and save pattern
#define PATTERN_SIZE (sizeof(SEQUENCER_TRACK_DATA)*TRACK_NUMBER)+4
#define PATTERN_NUMBER  14

SEQUENCER_TRACK_DATA _track_data;

void loadLastPattern()
{
  uint8_t last_pattern;

  EEPROM.get(1023, last_pattern);

  if ( last_pattern < PATTERN_NUMBER ) {
    loadPattern(last_pattern);
  }
}

void resetPattern(uint8_t number)
{
  uint16_t eeprom_address = PATTERN_SIZE;

  // get address base for pattern
  eeprom_address *= number;

  // load defaults
  ATOMIC(
    _transpose = 0;
    _harmonize = 0;
    _selected_mode = 0;
  );
  // reset sequencer data
  for ( uint8_t track = 0; track < TRACK_NUMBER; track++ ) {
    ATOMIC(_sequencer[track].mute = true);
    clearStackNote(track);    
    _track_data.step_init_point = 0;
    _track_data.step_length = STEP_MAX_SIZE;
    // initing note data
    for ( uint8_t i = 0; i < STEP_MAX_SIZE; i++ ) {
      _track_data.step[i].note = 48;
      _track_data.step[i].accent = 0;
      _track_data.step[i].glide = 0;
      _track_data.step[i].rest = 0;
    }

    ATOMIC(memcpy((void*)&_sequencer[track].data, &_track_data, sizeof(SEQUENCER_TRACK_DATA)));
    ATOMIC(_sequencer[track].mute = false);
  }  
  // mark pattern slot as not in use 
  EEPROM.write(eeprom_address, 0);
}

void loadPattern(uint8_t number) 
{
  uint16_t eeprom_address = PATTERN_SIZE;
  uint8_t pattern_exist, harmonize, selected_mode = 0;
  int8_t transpose = 0;
  
  if ( number >= PATTERN_NUMBER ) {
    return;
  }

  // get address base for pattern
  eeprom_address *= number;
  
  // do we have pattern data to read it here?
  EEPROM.get(eeprom_address, pattern_exist);
  if ( pattern_exist != 1 ) {
    resetPattern(number);
    // save last pattern loaded
    EEPROM.write(1023, number);    
    return;
  }

  // global pattern config
  EEPROM.get(++eeprom_address, transpose);
  EEPROM.get(++eeprom_address, harmonize);
  EEPROM.get(++eeprom_address, selected_mode);
  // constrains to avoid trash data from memory
  if ( transpose > 12 ) {
    transpose = 12;
  } else if ( transpose < -12 ) {
    transpose = -12;
  }
  if (selected_mode >= MODES_NUMBER) {
    selected_mode = MODES_NUMBER-1;
  }
  ATOMIC(
    _transpose = transpose;
    _harmonize = harmonize;
    _selected_mode = selected_mode;
  );
  // track data
  for (uint8_t track=0; track < TRACK_NUMBER; track++) {
    ATOMIC(_sequencer[track].mute = true);
    clearStackNote(track);    
    EEPROM.get(++eeprom_address, _track_data); // 34 bytes long
    // constrains to avoid trash data from memory
    if ( _track_data.step_length > STEP_MAX_SIZE ) {
      _track_data.step_length = STEP_MAX_SIZE;
    }
    ATOMIC(memcpy((void*)&_sequencer[track].data, &_track_data, sizeof(SEQUENCER_TRACK_DATA)));
    ATOMIC(_sequencer[track].mute = false);
    eeprom_address += (sizeof(SEQUENCER_TRACK_DATA)-1);
  }

  // save last pattern loaded
  EEPROM.write(1023, number);

  _selected_pattern = number;
  
}

void savePattern(uint8_t number) 
{
  uint16_t eeprom_address = PATTERN_SIZE;
  
  if ( number >= PATTERN_NUMBER ) {
    return;
  }

  // get address base for pattern
  eeprom_address *= number;
  
  // mark pattern slot as in use pattern_exist
  EEPROM.write(eeprom_address, 1);
  // global pattern config
  EEPROM.write(++eeprom_address, _transpose);
  EEPROM.write(++eeprom_address, _harmonize);
  EEPROM.write(++eeprom_address, _selected_mode);    
  // track data
  for (uint8_t track=0; track < TRACK_NUMBER; track++) {
    memcpy(&_track_data, (void*)&_sequencer[track].data, sizeof(SEQUENCER_TRACK_DATA));
    EEPROM.put(++eeprom_address, _track_data); // 34 bytes long
    eeprom_address += (sizeof(SEQUENCER_TRACK_DATA)-1);
  }
  
}

void connectPot(uint8_t pot_id, uint8_t pot_pin)
{
  _pot[pot_id].pin = pot_pin;
  // get first state data
  _pot[pot_id].state = analogRead(_pot[pot_id].pin);
  _pot[pot_id].lock = true;
}

void connectButton(uint8_t button_id, uint8_t button_pin)
{
  _button[button_id].pin = button_pin;
  // use internal pullup for buttons
  pinMode(_button[button_id].pin, INPUT_PULLUP);
  // get first state data
  _button[button_id].state = digitalRead(_button[button_id].pin);
  _button[button_id].hold_seconds = 0;
  _button[button_id].hold_trigger = false;
}
 
void lockPotsState(bool lock)
{
  for ( uint8_t i = 0; i < POT_NUMBER; i++ ) {
    _pot[i].lock = lock;
  }
}

bool pressed(uint8_t button_id)
{
  bool value;
  
  value = digitalRead(_button[button_id].pin);
  
  // using internal pullup pressed button goes LOW
  if ( value != _button[button_id].state && value == LOW ) {
    _button[button_id].state = value; 
    return true;    
  } else {
    _button[button_id].state = value; 
    return false;
  }
}

bool doublePressed(uint8_t button1_id, uint8_t button2_id)
{
  bool value1, value2;
  
  value1 = digitalRead(_button[button1_id].pin);
  value2 = digitalRead(_button[button2_id].pin);
  
  // using internal pullup pressed button goes LOW
  if ( value1 == LOW && value2 == LOW ) {
    _button[button1_id].state = LOW; 
    _button[button2_id].state = LOW;
    return true;    
  } else {
    return false;
  }
}

bool holded(uint8_t button_id, uint8_t seconds)
{
  bool value;
  
  value = digitalRead(_button[button_id].pin);
  
  // using internal pullup pressed button goes LOW
  if ( _button[button_id].hold_trigger == false && value == LOW ) {
    if ( _button[button_id].hold_seconds == 0 ) {
      _button[button_id].hold_seconds = (uint8_t)(millis()/1000);
    } else if ( abs((uint8_t)(millis()/1000) - _button[button_id].hold_seconds) >= seconds ) {
      _button[button_id].hold_trigger = true; // avoid released triger after action.
      return true;    
    }
    return false;
  } else if ( value == HIGH ) {
    _button[button_id].hold_trigger = false;
    _button[button_id].hold_seconds = 0;
    return false;
  }
}

bool released(uint8_t button_id)
{
  bool value;
  
  value = digitalRead(_button[button_id].pin);
  
  // using internal pullup released button goes HIGH
  if ( value != _button[button_id].state && value == HIGH && _button[button_id].hold_trigger == false ) {
    _button[button_id].state = value; 
    return true;    
  } else {
    _button[button_id].state = value; 
    return false;
  }
}

int16_t getPotChanges(uint8_t pot_id, uint16_t min_value, uint16_t max_value)
{
  uint16_t value, value_ranged, last_value_ranged;
  uint8_t pot_sensitivity = POT_SENSITIVITY;

  // get absolute value
  value = analogRead(_pot[pot_id].pin);
    
  // range that value and our last_value
  value_ranged = (value / (ADC_RESOLUTION / ((max_value - min_value) + 1))) + min_value;
  last_value_ranged = (_pot[pot_id].state / (ADC_RESOLUTION / ((max_value - min_value) + 1))) + min_value; 

  // a lock system to not mess with some data(pots are terrible for some kinda of user interface data controls, but lets keep it low cost!)
  if ( _pot[pot_id].lock == true ) {
    // user needs to move 1/8 of total adc range to get pot unlocked
    if ( abs(value - _pot[pot_id].state) < (ADC_RESOLUTION/8) ) {
      return -1;
    }
  }
  
  if ( abs(value_ranged - last_value_ranged) >= pot_sensitivity ) {
    _pot[pot_id].state = value;
    if ( _pot[pot_id].lock == true ) {
      _pot[pot_id].lock = false;
    }
    if ( value_ranged > max_value ) {
      value_ranged = max_value;
    }
    return value_ranged;    
  } else {
    return -1;
  }  
}

// DAC Control Functions (MCP4921)
void initDAC() {
  pinMode(DAC_CS_PIN, OUTPUT);
  digitalWrite(DAC_CS_PIN, HIGH);
  SPI.begin();
}

void writeDAC(uint8_t channel, uint16_t value) {
  digitalWrite(DAC_CS_PIN, LOW);
  
  // 12-bit data transmission for MCP4921
  // Format: 0b0011 0000 0000 0000 | (channel << 15) | (value & 0x0FFF)
  uint16_t data = 0x3000 | (channel << 15) | (value & 0x0FFF);
  
  SPI.transfer((data >> 8) & 0xFF);
  SPI.transfer(data & 0xFF);
  
  digitalWrite(DAC_CS_PIN, HIGH);
}

// Shift Register Control Functions
void initShiftRegister() {
  pinMode(SHIFT_DATA_PIN, OUTPUT);
  pinMode(SHIFT_CLOCK_PIN, OUTPUT);
  pinMode(SHIFT_LATCH_PIN, OUTPUT);
  
  digitalWrite(SHIFT_DATA_PIN, LOW);
  digitalWrite(SHIFT_CLOCK_PIN, LOW);
  digitalWrite(SHIFT_LATCH_PIN, LOW);
  
  // Initialize all LEDs to OFF
  updateShiftRegister(0x00);
}

void updateShiftRegister(uint8_t data) {
  digitalWrite(SHIFT_LATCH_PIN, LOW);
  
  // Send 8 bits of data
  for (int i = 7; i >= 0; i--) {
    digitalWrite(SHIFT_DATA_PIN, (data >> i) & 0x01);
    digitalWrite(SHIFT_CLOCK_PIN, HIGH);
    digitalWrite(SHIFT_CLOCK_PIN, LOW);
  }
  
  digitalWrite(SHIFT_LATCH_PIN, HIGH);
  _led_state = data;
}

// LED Control Functions (via Shift Register)
void setLED(uint8_t led_id, bool state) {
  if (led_id < 8) {
    if (state) {
      _led_state |= (1 << led_id);  // Set bit
    } else {
      _led_state &= ~(1 << led_id); // Clear bit
    }
    updateShiftRegister(_led_state);
  }
}

void setAllLEDs(uint8_t pattern) {
  _led_state = pattern & LED_SHIFT_MASK;
  updateShiftRegister(_led_state);
}

void clearAllLEDs() {
  _led_state = 0x00;
  updateShiftRegister(_led_state);
}

// Sequence Direction Control Functions
void setSequenceDirection(uint8_t direction) {
  if (direction < SEQUENCE_MODES) {
    _sequence_direction = direction;
    _pingpong_forward = true;  // Reset ping-pong direction
  }
}

uint8_t getSequenceDirection() {
  return _sequence_direction;
}

uint8_t getNextStep(uint8_t current_step, uint8_t step_length) {
  uint8_t next_step = current_step;
  
  switch (_sequence_direction) {
    case SEQUENCE_FORWARD:
      next_step = (current_step + 1) % step_length;
      break;
      
    case SEQUENCE_REVERSE:
      if (current_step == 0) {
        next_step = step_length - 1;
      } else {
        next_step = current_step - 1;
      }
      break;
      
    case SEQUENCE_PINGPONG:
      if (_pingpong_forward) {
        if (current_step >= step_length - 1) {
          _pingpong_forward = false;
          next_step = current_step - 1;
        } else {
          next_step = current_step + 1;
        }
      } else {
        if (current_step <= 0) {
          _pingpong_forward = true;
          next_step = current_step + 1;
        } else {
          next_step = current_step - 1;
        }
      }
      break;
      
    case SEQUENCE_RANDOM:
      do {
        next_step = random(0, step_length);
      } while (next_step == _last_random_step && step_length > 1);
      _last_random_step = next_step;
      break;
  }
  
  return next_step;
}

void cycleSequenceDirection() {
  _sequence_direction = (_sequence_direction + 1) % SEQUENCE_MODES;
  _pingpong_forward = true;  // Reset ping-pong direction
}

// CV Input Control Functions
void readCVInputs() {
  // Read LFO CV input (A0)
  _cv_lfo_value = analogRead(CV_LFO_IN_PIN);
  
  // Read modulation CV input (A1)
  _cv_mod_value = analogRead(CV_MOD_IN_PIN);
}

uint16_t getCVLFOValue() {
  return _cv_lfo_value;
}

uint16_t getCVModValue() {
  return _cv_mod_value;
}

void setCVModMode(uint8_t mode) {
  if (mode < CV_MOD_MODES) {
    _cv_mod_mode = mode;
  }
}

uint8_t getCVModMode() {
  return _cv_mod_mode;
}

// CV Modulation Functions
float getCVModulatedTempo(float base_tempo) {
  // Convert CV value to tempo modulation (-50% to +50%)
  float modulation = ((float)_cv_mod_value / CV_INPUT_RESOLUTION - 0.5) * 2.0;
  return base_tempo * (1.0 + modulation * 0.5);
}

int8_t getCVModulatedTranspose(int8_t base_transpose) {
  // Convert CV value to transpose modulation (-12 to +12 semitones)
  int8_t modulation = ((float)_cv_mod_value / CV_INPUT_RESOLUTION - 0.5) * 24.0;
  return constrain(base_transpose + modulation, -12, 12);
}

uint8_t getCVModulatedAccent(uint8_t base_accent) {
  // Convert CV value to accent intensity (0-127)
  uint8_t modulation = (float)_cv_mod_value / CV_INPUT_RESOLUTION * 127.0;
  return constrain(base_accent + modulation, 0, 127);
}

uint8_t getCVModulatedGlide(uint8_t base_glide) {
  // Convert CV value to glide time (0-255)
  uint8_t modulation = (float)_cv_mod_value / CV_INPUT_RESOLUTION * 255.0;
  return constrain(base_glide + modulation, 0, 255);
}

uint8_t getCVModulatedLength(uint8_t base_length) {
  // Convert CV value to note length (1-5)
  uint8_t modulation = ((float)_cv_mod_value / CV_INPUT_RESOLUTION - 0.5) * 4.0;
  return constrain(base_length + modulation, 1, 5);
}

// CV/Gate/Trigger Output Functions
void initCVOutputs()
{
  // Initialize DAC
  initDAC();
  
  // Initialize Shift Register
  initShiftRegister();
  
  // Initialize Gate output pins
  pinMode(GATE_TRACK1_PIN, OUTPUT);
  pinMode(GATE_TRACK2_PIN, OUTPUT);
  
  // Initialize Trigger output pins
  pinMode(TRIGGER_CLOCK_PIN, OUTPUT);
  pinMode(TRIGGER_START_PIN, OUTPUT);
  pinMode(TRIGGER_STOP_PIN, OUTPUT);
  
  // Initialize CV input pins
  pinMode(CV_CLOCK_IN_PIN, INPUT);
  pinMode(CV_RESET_IN_PIN, INPUT);
  pinMode(CV_LFO_IN_PIN, INPUT);
  pinMode(CV_MOD_IN_PIN, INPUT);
  
  // Set all outputs to low initially
  digitalWrite(GATE_TRACK1_PIN, LOW);
  digitalWrite(GATE_TRACK2_PIN, LOW);
  digitalWrite(TRIGGER_CLOCK_PIN, LOW);
  digitalWrite(TRIGGER_START_PIN, LOW);
  digitalWrite(TRIGGER_STOP_PIN, LOW);
  
  // Set CV outputs to middle C (2.5V) using DAC
  writeDAC(0, noteToCV(48)); // Track 1 to Middle C
  writeDAC(1, noteToCV(48)); // Track 2 to Middle C
  
  // Initialize LEDs with pattern
  setAllLEDs(0x55); // Alternating pattern for testing
}

// Convert MIDI note to CV voltage (1V/octave)
uint16_t noteToCV(uint8_t note)
{
  // Calculate voltage offset from base note
  int16_t semitone_offset = note - CV_BASE_NOTE;
  float voltage_offset = (semitone_offset / 12.0) * CV_VOLTS_PER_OCTAVE;
  float target_voltage = CV_BASE_VOLTAGE + voltage_offset;
  
  // Constrain voltage to valid range
  if (target_voltage < 0.0) target_voltage = 0.0;
  if (target_voltage > CV_VOLTAGE_RANGE) target_voltage = CV_VOLTAGE_RANGE;
  // Convert voltage to DAC value
  uint16_t dac_value = (uint16_t)((target_voltage / CV_VOLTAGE_RANGE) * CV_RESOLUTION);
  return dac_value;
}

// Output CV voltage for a track using DAC
void outputCV(uint8_t track, uint8_t note)
{
  uint16_t cv_value = noteToCV(note);
  
  if (track == 0) {
    writeDAC(0, cv_value); // Channel A for Track 1
  } else if (track == 1) {
    writeDAC(1, cv_value); // Channel B for Track 2
  }
}

// Output Gate signal for a track
void outputGate(uint8_t track, bool state)
{
  if (track == 0) {
    digitalWrite(GATE_TRACK1_PIN, state ? HIGH : LOW);
  } else if (track == 1) {
    digitalWrite(GATE_TRACK2_PIN, state ? HIGH : LOW);
  }
}

// Output Clock trigger pulse
void outputClockTrigger()
{
  digitalWrite(TRIGGER_CLOCK_PIN, HIGH);
  delay(TRIGGER_PULSE_WIDTH);
  digitalWrite(TRIGGER_CLOCK_PIN, LOW);
}

// Output Start trigger pulse
void outputStartTrigger()
{
  digitalWrite(TRIGGER_START_PIN, HIGH);
  delay(TRIGGER_PULSE_WIDTH);
  digitalWrite(TRIGGER_START_PIN, LOW);
}

// Output Stop trigger pulse
void outputStopTrigger()
{
  digitalWrite(TRIGGER_STOP_PIN, HIGH);
  delay(TRIGGER_PULSE_WIDTH);
  digitalWrite(TRIGGER_STOP_PIN, LOW);
}

// Read external clock input
bool readExternalClock()
{
  return analogRead(CV_CLOCK_IN_PIN) > 512; // Threshold at 2.5V
}

// Read external reset input
bool readExternalReset()
{
  return analogRead(CV_RESET_IN_PIN) > 512; // Threshold at 2.5V
}

// Extended CV mode with accent and glide modulation
void outputExtendedCV(uint8_t track, uint8_t note, bool accent, bool glide)
{
  uint16_t base_cv = noteToCV(note);
  uint16_t final_cv = base_cv;
  
  if (CV_MODE == 1) {
    // Add accent modulation (slight pitch bend up)
    if (accent) {
      final_cv += 20; // Small pitch increase for accent
    }
    
    // Add glide modulation (slight pitch bend down)
    if (glide) {
      final_cv -= 15; // Small pitch decrease for glide
    }
    
    // Constrain to valid range
    if (final_cv > CV_RESOLUTION - 1) final_cv = CV_RESOLUTION - 1;
    if (final_cv < 0) final_cv = 0;
  }
  
  outputCV(track, note);
}
