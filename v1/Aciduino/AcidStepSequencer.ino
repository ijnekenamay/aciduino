// Acid StepSequencer, a Roland TB303 step sequencer engine clone
// author: midilab contact@midilab.co
// under MIT license
#include "uClock.h"

#define NOTE_STACK_SIZE    3

// CV/Gate/Trigger definitions for analog output
#define CV_CLOCK_TRIGGER     1
#define CV_START_TRIGGER     2
#define CV_STOP_TRIGGER      3
#define CV_NOTE_ON          4
#define CV_NOTE_OFF         5
#define CV_CC               6

// sequencer data
typedef struct
{
  uint8_t note;
  int16_t length;
} STACK_NOTE_DATA;

typedef struct
{
  uint8_t note:7;
  uint8_t accent:1;
  uint8_t glide:1;
  uint8_t rest:1;
  uint8_t tie:1;
  uint8_t reserved:5;
} SEQUENCER_STEP_DATA;
// 2 bytes per step

typedef struct
{
  SEQUENCER_STEP_DATA step[STEP_MAX_SIZE];
  int8_t step_init_point;
  uint8_t step_length;
} SEQUENCER_TRACK_DATA;
// 32 bytes per 16 step + 2 bytes config = 34 bytes [STEP_MAX_SIZE=16]

typedef struct
{
  SEQUENCER_TRACK_DATA data;
  uint8_t step_location;
  uint8_t channel;
  bool mute;
  STACK_NOTE_DATA stack[NOTE_STACK_SIZE];
} SEQUENCER_TRACK;

// main sequencer data is constantly change inside uClock 16PPQN and 96PPQN ISR callbacks, so volatile him!
SEQUENCER_TRACK volatile _sequencer[TRACK_NUMBER];

uint8_t _selected_track = 0;
uint8_t _selected_pattern = 0;

// make sure all above sequencer data are modified atomicly only
// eg. ATOMIC(_sequencer[track]data.step[0].accent = 1); ATOMIC(_sequencer[track].data.step_length = 7);

// shared data to be used for user interface interaction and feedback
bool _playing = false;
uint8_t _harmonize = 0;
uint16_t _step_edit = 0;
uint8_t _last_octave = 3;
uint8_t _last_note = 0;
int8_t _transpose = 0; // zero is centered C
uint8_t _selected_mode = 0;

void sendCVMessage(uint8_t command, uint8_t byte1, uint8_t byte2, uint8_t channel)
{
  // send CV/Gate/Trigger message
  switch (command) {
    case CV_NOTE_ON:
      if (byte1 >= 0 && byte1 <= 127) {
        outputCV(channel, byte1);
        outputGate(channel, true);
      }
      break;
    case CV_NOTE_OFF:
      outputGate(channel, false);
      break;
    case CV_CLOCK_TRIGGER:
      outputClockTrigger();
      break;
    case CV_START_TRIGGER:
      outputStartTrigger();
      break;
    case CV_STOP_TRIGGER:
      outputStopTrigger();
      break;
  }
}

// The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
void ClockOut16PPQN(uint32_t * tick)
{
  uint8_t step, next_step;
  uint16_t length;
  int8_t note;
  
  // Get current step from sequencer
  step = _sequencer[_selected_track].step_location;
  
  // Get next step based on sequence direction
  next_step = getNextStep(step, _sequencer[_selected_track].data.step_length);

  for ( uint8_t track = 0; track < TRACK_NUMBER; track++ ) {

    if ( _sequencer[track].mute == true ) {
      continue;
    }

    length = NOTE_LENGTH;

    // get actual step location.
    _sequencer[track].step_location = uint32_t(*tick + _sequencer[track].data.step_init_point) % _sequencer[track].data.step_length;

    // send note on only if this step are not in rest mode
    if ( _sequencer[track].data.step[_sequencer[track].step_location].rest == 0 ) {

      // check for slide or tie event ahead of _sequencer[track].step_location
      step = _sequencer[track].step_location;
      next_step = step;
      for ( uint8_t i = 1; i < _sequencer[track].data.step_length; i++  ) {
        next_step = ++next_step % _sequencer[track].data.step_length;
        if (_sequencer[track].data.step[step].glide == 1 && _sequencer[track].data.step[next_step].rest == 0) {
          length = NOTE_LENGTH + 5;
          break;
        } else if (_sequencer[track].data.step[next_step].tie == 1 && _sequencer[track].data.step[next_step].rest == 1) {
          length = NOTE_LENGTH + (i * 6);
        } else if ( _sequencer[track].data.step[next_step].rest == 0 || _sequencer[track].data.step[next_step].tie == 0) {
          break;
        }
      }

      // find a free note stack to fit in
      for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
        if ( _sequencer[track].stack[i].length == -1 ) {
          if ( _harmonize == 1 ) {
            note = harmonizer(_sequencer[track].data.step[_sequencer[track].step_location].note);
          } else {
            note = _sequencer[track].data.step[_sequencer[track].step_location].note;
          }
          note += _transpose;
          // in case transpose push note away from the lower or higher midi note range barrier do not play it
          if ( note < 0 || note > 127 ) {
            break;
          }
          _sequencer[track].stack[i].note = note;
          _sequencer[track].stack[i].length = length;
          // send note on with CV and Gate
          if (CV_MODE == 1) {
            outputExtendedCV(track, note, _sequencer[track].data.step[_sequencer[track].step_location].accent, _sequencer[track].data.step[_sequencer[track].step_location].glide);
          } else {
            outputCV(track, note);
          }
          outputGate(track, true);
          break;
        }
      }
    }
  }
}

void clearStackNote(int8_t track = -1)
{
  if ( track <= -1 ) {
    // clear all tracks stack note
    for ( uint8_t i = 0; i < TRACK_NUMBER; i++ ) {
      // clear and send any note off
      for ( uint8_t j = 0; j < NOTE_STACK_SIZE; j++ ) {
        ATOMIC(
          outputGate(i, false);
          _sequencer[i].stack[j].length = -1;
        )
      }
    }
  } else {
    // clear and send any note off
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
              ATOMIC(
          outputGate(track, false);
          _sequencer[track].stack[i].length = -1;
        )
    }
  }

}

// The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
void ClockOut96PPQN(uint32_t * tick)
{
  uint8_t track;

  // Send Clock trigger to external hardware
  outputClockTrigger();

  for ( track = 0; track < TRACK_NUMBER; track++ ) {

    // handle note on stack
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
      if ( _sequencer[track].stack[i].length != -1 ) {
        --_sequencer[track].stack[i].length;
        if ( _sequencer[track].stack[i].length == 0 ) {
          outputGate(track, false);
          _sequencer[track].stack[i].length = -1;
        }
      }
    }
  }
  // user feedback about sequence time events
  tempoInterface(tick);
}

// The callback function wich will be called when clock starts by using Clock.start() method.
void onClockStart()
{
  outputStartTrigger();
  _playing = 1;
}

// The callback function wich will be called when clock stops by using Clock.stop() method.
void onClockStop()
{
  outputStopTrigger();

  // clear all tracks stack note
  clearStackNote();

  _playing = 0;
}

void setTrackChannel(uint8_t track, uint8_t channel)
{
  --track;
  --channel;
  ATOMIC(_sequencer[track].channel = channel);
}

void initAcidStepSequencer(uint8_t mode)
{
  uint8_t track;
  // Initialize CV/Gate/Trigger outputs
  initCVOutputs();

  // Inits the clock
  uClock.init();
  // Set the callback function for the clock output to send CV/Gate/Trigger signals
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  // Set the callback function for the step sequencer on 16ppqn
  uClock.setClock16PPQNOutput(ClockOut16PPQN);
  // Set the callback function for CV/Gate/Trigger Start and Stop messages
  uClock.setOnClockStartOutput(onClockStart);
  uClock.setOnClockStopOutput(onClockStop);
  // Set the clock BPM to 126 BPM
  uClock.setTempo(126);

  // initing sequencer memory data
  for ( track = 0; track < TRACK_NUMBER; track++ ) {

    _sequencer[track].channel = track;
    _sequencer[track].data.step_init_point = 0;
    _sequencer[track].data.step_length = STEP_MAX_SIZE;
    _sequencer[track].step_location = 0;
    _sequencer[track].mute = false;

    // initing note data
    for ( uint16_t i = 0; i < STEP_MAX_SIZE; i++ ) {
      _sequencer[track].data.step[i].note = 48;
      _sequencer[track].data.step[i].accent = 0;
      _sequencer[track].data.step[i].glide = 0;
      _sequencer[track].data.step[i].tie = 0;
      _sequencer[track].data.step[i].rest = 0;
    }
    // initing note stack data
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
      _sequencer[track].stack[i].note = 0;
      _sequencer[track].stack[i].length = -1;
    }
  }
}
