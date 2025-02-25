# Aciduino

Cheap Roland TB303 Step Sequencer clone aimed for live interaction. Your acid lines for a very low cost.

With some user interface changes for cheap construction of a functional TB303 engine we present you the interface:

![Image of aciduino on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/Hardware/acid_step_sequencer-protoboard-v002.png)

### <span style="color: red; ">So you want a aciduino PCB?</span>
  
[ijnekenamay](https://github.com/ijnekenamay) has created one for us, you can [download the files](https://github.com/midilab/aciduino/tree/master/Hardware/ijnekenamay-PCB) and send it to a fab house, or asking him to sell one for you.  
  
![Image of aciduino on protoboard](https://github.com/midilab/aciduino/raw/master/Hardware/ijnekenamay-PCB/mk2_image5.JPG)

# Features

## 303 Step Sequencer clone
2 Tracks of famous Roland TB303 sequencer with 14 non volatile pattern memory slots to save your work. Programming using rest, glide and accent on each step. The addition of step length parameter per track makes you able to choose from 1 step to 16 steps of pattern length to make some unsual sequences.

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
  
You can put you aciduino into slave mode to be in sync with a master clock source by press and hold the play/stop button until his led is ON. To get back to master mode  press and hols again until the led is OFF. 
  
Use this information above as reference for aciduino interface 

**[page select]**  
knobs    ( none )    ( none )    ( none )    ( none )  
buttons [ track 1 ] [ track 2 ] [live mode] [generative] [step edit] [play/stop]  


**[page 1 live mode]**  
knobs    (ctrl 1 A/B)  (ctrl 2 A/B)  (ctrl 3 A/B)  (length/tunning)  
buttons [ << pattern ] [ pattern >> ] [ ctrl A/B ] [ << tempo ] [ tempo >> ] [play/stop]  
hold    [save pattern] [delete pattern] => hold button 1 or button 2 until led blink to complete the action.  


**[page 2 generative mode]**  
knobs    (low range)  (high range)  (tones num)  (notes num)  
buttons [ << scale ] [ scale >> ] [ generate ] [ << shift ] [ shift >> ] [play/stop]  


**[page 3 step edit]**  
knobs     (octave)    (note)      (tunning)    (length)  
buttons [ << step ] [ step >> ] [   rest   ] [   glide   ] [  accent  ] [play/stop]  


## [page select]: press button1 and button2 together

Just press the desired page button to navigate thru. 

Use track 1 and track 2 buttons to select the track to be use. The selected track is the one with led on.

**interface**  
knobs    ( none )    ( none )    ( none )    ( none )  
buttons [ track 1 ] [ track 2 ] [live mode] [generative] [step edit] [play/stop]  

## [live mode]

The navigation buttons 1 and 2 are used to change pattern, one of the leds will turn on when you reach the navigation barrier - first patterns led 1 on, last pattern led 2 on.

Holding button 1 or 2 for 2 seconds: save pattern, delete and reset pattern

The midi cc commands will be sent to the selected track only.

ctrl A and ctrl B change between knobs midi data to be sent or controlled. The selected ctrl A is the one with led off and ctrl B the led on.

Global tunning lets you go up or down on scale tunning, from -12 to +12 interval.

**interface**  
knobs    (ctrl 1 A/B)  (ctrl 2 A/B)  (ctrl 3 A/B)  (length/tunning)  
buttons [ << pattern ] [ pattern >> ] [ ctrl A/B ] [ << tempo ] [ tempo >> ] [play/stop]  
hold    [save pattern] [delete pattern] => hold button 1 or button 2 until led blink to complete the action.  

## [generative]

Press generate button to get a fresh new sequence on selected track, turning knobs here doesnt change the current sequence, it only sets the parameters for the next time you press generate button.

harmonic scale mode -(previous) and harmonic scale mode +(next) lets you navigate thru harmonic modes(scales). Pressing harmonic mode - all the way until get led on means no harmonic context apply to sequence generation.

By default harmonize is off, just press once harmonic +(next) to get first scale on list, ionian, next again to got to dorian and so on.

Ramdom range low note selects the first note of sequence range notes that can be generated by probabilty. Ramdom range note defines the range of notes that can be generated starting from ramdom low note.

Ramdom range high note selects the barrier to the most high note allowed to be generated on a sequence.

Using shift buttons you can shitf all the sequence steps one step left or rigth using shift navigation buttons.

**interface**  
knobs    (low range)  (high range)  (tones num)  (notes num)  
buttons [ << scale ] [ scale >> ] [ generate ] [ << shift ] [ shift >> ] [play/stop]  

## [step edit]
Here you can program you acid lines in the same way you program a original one, but instead of keypad for notes you use 2 knobs, one for octave and other for note to get access to all midi range notes spectrum while editing step by step your sequence using rest, glide and accent.

Using step navigation buttons the led on for prev step button means you are have the first step selected for edit, led on for next step button means you have the last step select for edit.

The rest, glide and accent buttons are applied to selected step, led on/off are used to express the state of each parameter per step.

Octave and note knobs lets you select the note for the selected step.

Sequence lenth knob lets you choose from 1 step to 16 steps or lenth for your sequence.

**interface**  
knobs     (octave)    (note)      (tunning)    (length)  
buttons [ << step ] [ step >> ] [   rest   ] [   glide   ] [  accent  ] [play/stop]  

# Configure for personal needs

Use config.h to hack your environment as you wish.

# Dependencies

This clone is based on uClock library to generate sequencer BPM clock. The library is now part of project to get DIYers a easy compile time.

You can check more info for this library on: https://github.com/midilab/uClock

# BOM List

* 1x arduino nano
* 1x solderless breadboard protoboard 830 tie points 2 busess
* 1x solderless breadboard jumper cable wire kit box
* 6x 220ohms resistors
* 6x 6mm taticle switch
* 6x 3mm diffused leds
* 4x 10k linear potentiometers

## MIDI connection?

Add more two 220ohms resistors to you boom list and choose one of thoses above methods as main midi conection.

* 1x 5 pins DIN female connector (standard MIDI connector) + sto protoboard - you need solder skills here.
* 1x midi cable. Cut it in  a half and connect it directly to protoboard - its a good idea to soldering the wire tip before make the connections.
  
You can also use a MIDI input board to sync the aciduino with a master clock source if needed. Search over the internet for a MIDI input schematic if you want to build it yourself for aciduino.  
  
## USB Connection?

* Open config.h and set MIDI_MODE to 1. 
* Download and install a serial-to-midi converter like http://projectgus.github.io/hairless-midiserial/ 
* Open your favorite DAW, configure MIDI port to get signal from serial-to-midi converter and just go acidulize with your preferred acid synth plugin. Depends on your operational system you may need a additional MIDI virtual port driver installed.
