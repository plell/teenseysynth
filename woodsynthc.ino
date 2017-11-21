#include <ADC.h> 
#include <MozziGuts.h>
#include <mozzi_midi.h>
#include <Oscil.h>
#include <FastLED.h>
#include <RollingAverage.h>
#include <EventDelay.h>
#include <LowPassFilter.h>
#include <MIDI.h>

#define NUM_LEDS 4
#define DATA_PIN 12

CRGB leds[NUM_LEDS];

EventDelay OSC1Delay;
EventDelay OSC2Delay;

int var2, var3, var4, var5, var6, var7, switch1, push1, mapped_push1;
float var1, knob1, bend;
int knob2, knob3, knob4, knob5, knob6, knob7;
int average_knob4, average_knob5, note, velocity, channel;
int hue = 0; 
int hue2 = 0;
int bright = 40;
int bright2 = 40;
int sat = 250;

RollingAverage <int, 12> aAverage; // how_many_to_average has to be power of 2
RollingAverage <int, 12> bAverage; // how_many_to_average has to be power of 2

#include <tables/sin512_int8.h>
#include <tables/cos512_int8.h>
#include <tables/triangle512_int8.h>
#include <tables/triangle_analogue512_int8.h>
#include <tables/square_no_alias512_int8.h>
#include <tables/square_analogue512_int8.h>
#include <tables/saw512_int8.h>
#include <tables/saw_analogue512_int8.h>
#include <tables/halfsinwindow512_uint8.h>


Oscil<512, AUDIO_RATE> OSC1;
Oscil<512, AUDIO_RATE> OSC2;
Oscil<512, AUDIO_RATE> OSC1octave;

Oscil<SIN512_NUM_CELLS, CONTROL_RATE> LFO(SIN512_DATA);
Oscil<SIN512_NUM_CELLS, CONTROL_RATE> LFO2(SIN512_DATA);
Oscil<TRIANGLE512_NUM_CELLS, CONTROL_RATE> kDelSamps(TRIANGLE512_DATA);
LowPassFilter lpf;
LowPassFilter lpf2;
LowPassFilter lpf3;


char gain = 1;
char gain2 = 1;
float gainall = 0.f;
#define CONTROL_RATE 64

//AudioDelayFeedback <64> aDel;
//byte del_samps;



void setup(){
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(57600);
  startMozzi(CONTROL_RATE); // a literal control rate here
  LFO.setFreq(1.3f);
  LFO2.setFreq(1.3f);
  lpf.setResonance(240);
  lpf2.setResonance(240);
  lpf3.setResonance(30);
  OSC1Delay.set(1000);
  OSC2Delay.set(1000);
  OSC1.setFreq(0);
  OSC2.setFreq(0);
  OSC1octave.setFreq(0);
  pinMode(4, INPUT);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

  
void updateControl(){

if (MIDI.read()) {                    // Is there a MIDI message incoming ?
    byte type = MIDI.getType();
switch (type) {
      case NoteOn:
        note = MIDI.getData1();
        velocity = MIDI.getData2();
        channel = MIDI.getChannel();
        if (velocity > 0) {
          Serial.println(String("Note On:  ch=") + channel + ", note=" + note + ", velocity=" + velocity);
        } else {
          Serial.println(String("Note Off: ch=") + channel + ", note=" + note);
        }
        break;
      case NoteOff:
        note = MIDI.getData1();
        velocity = MIDI.getData2();
        channel = MIDI.getChannel();
        Serial.println(String("Note Off: ch=") + channel + ", note=" + note + ", velocity=" + velocity);
        break;
}
}

var1 = analogRead(0);
var2 = analogRead(1);
var3 = analogRead(2);
var4 = analogRead(3);
var5 = analogRead(4);
var6 = analogRead(5);
var7 = analogRead(6);
push1 = analogRead(8);


knob1 = map(var1, 0, 1023, 0, 10);
knob2 = map(var2, 0, 1023, 1, 10); //shape
knob3 = map(var3, 0, 1023, 0, 400); //blink
knob4 = map(var4, 0, 1023, 0, 1300); //OSC1 freq
knob5 = map(var5, 0, 1023, 0, 1300); //OSC2 freq
knob6 = map(var6, 0, 1023, 0, 8); //LFO
knob7 = map(var7, 0, 1023, 0, 250); //buttload
mapped_push1 = map(push1, 0, 1023, 0, 600);
switch1 = digitalRead(4);
bend = aAverage.next(mapped_push1);
average_knob4 = aAverage.next(knob4);
average_knob5 = bAverage.next(knob5);
//Serial.println(knob7);
//frequency controls
if (push1 < 50){
  OSC1.setFreq(average_knob4 + mtof(note));
  OSC2.setFreq(average_knob5 + mtof(note));
}
else {
OSC1.setFreq(average_knob4 + bend + mtof(note));
OSC2.setFreq(average_knob5 + bend + mtof(note));
bright = 50;
bright2 = 50;
}
//blinks gears

if(OSC1Delay.ready()){
    gain = 1-gain; // flip 0/1
    bright = 12;
    OSC1Delay.start();
  }
if(OSC2Delay.ready()){
    gain2 = 1-gain2; // flip 0/1
    bright2 = 12;
    OSC2Delay.start();
  }

//OSC 1 instructions
if (switch1 == 0){
  ///blink
  if (knob3 > 30) {
  OSC1Delay.set(knob3);
  }
  else{
  gain = 1;  
  }
  //LFO
  LFO.setFreq(knob6);
  byte cutoff_freq = LFO.next();
  lpf.setCutoffFreq(cutoff_freq);
  //shape
        if (knob2 < 0){
          OSC1.setTable(SIN512_DATA);
          hue = 0;
        }
        else if (knob2 < 1){
          OSC1.setTable(COS512_DATA);
          hue = 30;
        }
        else if (knob2 < 2){
          OSC1.setTable(TRIANGLE512_DATA);
          hue = 60;
         }
        else if (knob2 < 3){
          OSC1.setTable(TRIANGLE_ANALOGUE512_DATA);
          hue = 90;
        }
        else if (knob2 < 4){
          OSC1.setTable(SQUARE_NO_ALIAS512_DATA);
          hue = 120;
        }
        else if (knob2 < 5){
          OSC1.setTable(SQUARE_ANALOGUE512_DATA);
          hue = 150;
        }
        else if (knob2 < 6){
          OSC1.setTable(SAW512_DATA);
          hue = 180;
        }
        else if (knob2 < 7){
          OSC1.setTable(SAW_ANALOGUE512_DATA);
          hue = 210;
        }
        else if (knob2 < 8){
          OSC1.setTable(HALFSINWINDOW512_DATA);
          hue = 240;
        }
        else {
          OSC1.setTable(HALFSINWINDOW512_DATA);
          hue = 255;
        }


if (cutoff_freq > 230) {
  bright = 12;
}

}
//OSC 2 instructions
else {
  ///blink
  if (knob3 > 30) {
  OSC2Delay.set(knob3);
  }
  else{
  gain2 = 1;  
  }
  //LFO
  LFO2.setFreq(knob6);
  byte cutoff_freq2 = LFO2.next();
  lpf2.setCutoffFreq(cutoff_freq2);
  //shape
        if (knob2 < 0){
          OSC2.setTable(SIN512_DATA);
          hue2 = 0;
        }
        else if (knob2 < 1){
          OSC2.setTable(COS512_DATA);
          hue2 = 30;
        }
        else if (knob2 < 2){
          OSC2.setTable(TRIANGLE512_DATA);
          hue2 = 60;
        }
        else if (knob2 < 3){
          OSC2.setTable(TRIANGLE_ANALOGUE512_DATA);
          hue2 = 90;
        }
        else if (knob2 < 4){
          OSC2.setTable(SQUARE_NO_ALIAS512_DATA);
          hue2 = 120;
        }
        else if (knob2 < 5){
          OSC2.setTable(SQUARE_ANALOGUE512_DATA);
          hue2 = 150;
        }
        else if (knob2 < 6){
          OSC2.setTable(SAW512_DATA);
          hue2 = 180;
        }
        else if (knob2 < 7){
          OSC2.setTable(SAW_ANALOGUE512_DATA);
          hue2 = 210;
        }
        else if (knob2 < 8){
          OSC2.setTable(HALFSINWINDOW512_DATA);
          hue2 = 240;
        }
        else {
          OSC2.setTable(HALFSINWINDOW512_DATA);
          hue2 = 255;
        }

   //harmony
   
if (cutoff_freq2 > 230) {
  bright2 = 12;
}
}

lpf3.setCutoffFreq(knob7);


    leds[0] = CHSV(hue,sat,bright);
    leds[1] = CHSV(hue,sat,bright);
    leds[2] = CHSV(hue2,sat,bright2);
    leds[3] = CHSV(hue2,sat,bright2);

if (bright > 0){bright-=4;}
if (bright2 > 0){bright2-=4;}

gainall = knob1/10;
Serial.print(knob1);
Serial.print(" and gainall is: ");
Serial.println(gainall);

FastLED.show();

}


int updateAudio(){
return (lpf3.next((lpf.next((OSC1.next())*gain + lpf2.next(OSC2.next()*gain2)))))*gainall;
}


void loop(){
  audioHook();
}
