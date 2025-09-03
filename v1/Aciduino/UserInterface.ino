/*
[page select]: press button1 and button2 together
knobs: none, none, none, none

buttons: track 1, track 2, [live mode], [generative], [step edit], play/stop
*/

uint32_t _page_blink_timer = 0; 
uint8_t _bpm_blink_timer = 1;
uint8_t _selected_page = 0;

void configureInterface()
{
  // Buttons config (via 74HC165)
  initButtonShift();
  connectButton(GENERIC_BUTTON_1);
  connectButton(GENERIC_BUTTON_2);
  connectButton(GENERIC_BUTTON_3);
  connectButton(GENERIC_BUTTON_4);
  connectButton(GENERIC_BUTTON_5);
  connectButton(GENERIC_BUTTON_6);  

  // Pots config
  connectPot(GENERIC_POT_1, GENERIC_POT_1_PIN);
  connectPot(GENERIC_POT_2, GENERIC_POT_2_PIN);
  connectPot(GENERIC_POT_3, GENERIC_POT_3_PIN);
  connectPot(GENERIC_POT_4, GENERIC_POT_4_PIN);

  // Leds config (74HC595 via shift register)
  clearAllLEDs();

  // first read to fill our registers
  getPotChanges(GENERIC_POT_1, 0, ADC_RESOLUTION);
  getPotChanges(GENERIC_POT_2, 0, ADC_RESOLUTION);
  getPotChanges(GENERIC_POT_3, 0, ADC_RESOLUTION);
  getPotChanges(GENERIC_POT_4, 0, ADC_RESOLUTION);
}

void processInterface()
{
  static int16_t tempo;
  
  // Read CV inputs for real-time modulation
  readCVInputs();

  // set external sync on/off
  if ( holded(GENERIC_BUTTON_6, 2) ) {
    if ( uClock.getMode() == uClock.INTERNAL_CLOCK ) {
      setExternalSync(true);
    } else {
      setExternalSync(false);
    }
  }
    
  // global controllers play and tempo
  // play/stop
  if ( pressed(GENERIC_BUTTON_6) ) {
    if ( _playing == false ) {
      // Starts the clock, tick-tac-tick-tac...
      uClock.start();
    } else {
      // stop the clock
      uClock.stop();
    }
  }

  // internal/external led control
  if ( uClock.getMode() == uClock.INTERNAL_CLOCK ) {
    if ( _playing == false ) {
      setLED(LED_6, false);
    }
  } else {
    // external clock keeps the timer led always on
    setLED(LED_6, true);
  }

  // page select request
  if ( doublePressed(GENERIC_BUTTON_1, GENERIC_BUTTON_2) ) {
    lockPotsState(true);
    _selected_page = 0;
  }
  
  // sequence direction change (button 1 + button 3)
  if ( doublePressed(GENERIC_BUTTON_1, GENERIC_BUTTON_3) ) {
    cycleSequenceDirection();
    // Visual feedback for sequence direction
    uint8_t direction = getSequenceDirection();
    setAllLEDs(0x00);  // Clear all LEDs
    setLED(direction, true);  // Light up LED corresponding to direction
  }

  switch ( _selected_page ) {

    // Select Track/Page
    case 0:
      processPageButtons();
      processPageLeds();
      break;
      
    // Midi controller  
    case 1:
      processControllerButtons();
      processControllerLeds();
      processControllerPots();  
      break;
      
    // Generative  
    case 2:
      processGenerativeButtons();
      processGenerativeLeds();
      processGenerativePots();  
      break;
      
    // Sequencer
    case 3:
      processSequencerButtons();
      processSequencerLeds();
      processSequencerPots();  
      break;        
  }

}

void processPageButtons()
{

  if ( pressed(GENERIC_BUTTON_1) ) {
    _selected_track = 0;
  }

  if ( pressed(GENERIC_BUTTON_2) ) {
    _selected_track = 1;
  }

  if ( pressed(GENERIC_BUTTON_3) ) {
    lockPotsState(true);
    _selected_page = 1;
  }

  if ( pressed(GENERIC_BUTTON_4) ) {
    lockPotsState(true);
    _selected_page = 2;
  }

  if ( pressed(GENERIC_BUTTON_5) ) {
    lockPotsState(true);
    _selected_page = 3;
  }
  
}

void processPageLeds()
{
  static bool blink_state = true;
  
  // blink interface here for button 3 to 5
  if ( millis() - _page_blink_timer >= 300 ) {
    blink_state = !blink_state;
    _page_blink_timer = millis();
  }

  setLED(LED_3, blink_state);
  setLED(LED_4, blink_state);
  setLED(LED_5, blink_state);

  if ( _selected_track == 0 ) {
    setLED(LED_1, true);
    setLED(LED_2, false);
  } else if ( _selected_track == 1 ) {
    setLED(LED_1, false);
    setLED(LED_2, true);
  } 
}
      
void tempoInterface(uint32_t * tick) 
{
  if (uClock.getMode() == uClock.INTERNAL_CLOCK) { 
    // BPM led indicator
    if ( !(*tick % (96)) || (*tick == 0) ) {  // first compass step will flash longer
      _bpm_blink_timer = 8;
      setLED(LED_6 , true);
    } else if ( !(*tick % (24)) ) {   // each quarter led on
      setLED(LED_6 , true);
    } else if ( !(*tick % _bpm_blink_timer) ) { // get led off
      setLED(LED_6 , false);
      _bpm_blink_timer = 1;
    }
  }
}
