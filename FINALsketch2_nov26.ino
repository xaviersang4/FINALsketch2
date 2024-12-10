#include <Adafruit_NeoPixel.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=68.75000762939453,664.2499980926514
AudioEffectBitcrusher    bitcrusher1;    //xy=184.75000762939453,462.24999809265137
AudioMixer4              mixer1;         //xy=283.75000762939453,618.2499980926514
AudioEffectFreeverb      freeverb1;      //xy=379.75000762939453,459.24999809265137
AudioMixer4              mixer3;         //xy=467.00001525878906,615.0000057220459
AudioMixer4              mixer2;         //xy=622.7500076293945,603.2499980926514
AudioFilterBiquad        biquad1;        //xy=774.5000152587891,601.2500085830688
AudioAnalyzePeak         peak1;          //xy=937.5000152587891,470.0000066757202
AudioAnalyzeNoteFrequency notefreq1;      //xy=941.0000076293945,420.0000009536743
AudioOutputI2S           i2s2;           //xy=952.7500152587891,594.2500076293945
AudioConnection          patchCord1(i2s1, 0, bitcrusher1, 0);
AudioConnection          patchCord2(i2s1, 0, mixer1, 1);
AudioConnection          patchCord3(i2s1, 0, mixer2, 1);
AudioConnection          patchCord4(bitcrusher1, 0, mixer1, 0);
AudioConnection          patchCord5(mixer1, freeverb1);
AudioConnection          patchCord6(mixer1, 0, mixer3, 1);
AudioConnection          patchCord7(freeverb1, 0, mixer3, 0);
AudioConnection          patchCord8(mixer3, 0, mixer2, 0);
AudioConnection          patchCord9(mixer2, biquad1);
AudioConnection          patchCord10(biquad1, 0, i2s2, 0);
AudioConnection          patchCord11(biquad1, 0, i2s2, 1);
AudioConnection          patchCord12(biquad1, notefreq1);
AudioConnection          patchCord13(biquad1, peak1);
AudioControlSGTL5000     sgtl5000_1;     //xy=677.0000076293945,234.75000095367432
// GUItool: end automatically generated code



int pot1 = A12;
int pot2 = A13;
int pot3 = A11;
int mappedPot = 0;
int delaySwitch = 32;
int OnOffSwitch = 0;
int channelButton = 32;
int byBitButton = 31;
int byVerbButton = 30;

bool LastchannelState = LOW;
bool channelState = LOW;
bool byBitState = LOW;
bool byBitLastchannelState = LOW;
bool byVerbState = LOW;
bool byVerbLastchannelState = LOW;
bool byBit = LOW;
bool byRev = LOW;


int cChannelStep = 0;
int effectStep = 0;
int totalCSteps = 3;
int channelLeds[3] = { 33, 28, 29 };
int byPassLED1 = 4;
int byPassLED2 = 5;

//Effect Variables
int frequency = 0;
int resonance = 0;
int current_CrushedBitz = 16;
int current_SampleRate = 44100;
float seconds = 0;
float damp = 0;
float RFilter = 0;
float revFilter = 0;

int currentPotv1 = 0;
int currentPotv2 = 0;
int currentPotv3 = 0;
bool pot1Locked = LOW;
bool pot2Locked = LOW;
bool pot3Locked = LOW;
int lockedValue1 = 0;
int lockedValue2 = 0;
int lodkedValue3 = 0;

// FILTER
int lowFrequencyRange = 5000;
int highFrequencyRange = 2000;
float LFdB = 0.0;
float HFdB = 0.0;
int lowFrequencyRangeLocked = 5000;
int LFdBLocked = 0;
int highFrequencyRangeLocked = 2000;
int HFdBLocked = 0;
int pot4 = A10;
int pot5 = A15;
int switchHL = 9;
int switchOnOffFilter = 6;
int volumePot2 = 0;
int volumePot1 = 0;
int HighLowSwithch = 0;
int filterOnOff = 0;

int numPixels = 8;
int pixelPin = 3;
int peakMap = 0;
int noteMap = 0;
int noteMap2 = 0;
Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(numPixels, pixelPin, NEO_GRB);




void setup() {

  // put your setup code here, to run once:
  AudioMemory(500);
  Serial.begin(9600);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  // sgtl5000_1.micGain(30);

  for (int j = 0; j < 3; j++) {
    pinMode(channelLeds[j], OUTPUT);
  }
  pinMode(pot1, INPUT);
  pinMode(pot2, INPUT);
  pinMode(pot3, INPUT);
  pinMode(pot4, INPUT);
  pinMode(pot5, INPUT);
  pinMode(switchHL, INPUT);
  pinMode(switchOnOffFilter, INPUT);
  pinMode(channelButton, INPUT);
  pinMode(byBitButton, INPUT);
  pinMode(byVerbButton, INPUT);
  pinMode(byPassLED1, OUTPUT);
  pinMode(byPassLED2, OUTPUT);

  bitcrusher1.sampleRate(44100);
  bitcrusher1.bits(16);

neopixel.begin();
neopixel.clear();
neopixel.show();


  mixer2.gain(0, 1);
  mixer2.gain(1, 0);
  mixer2.gain(2, 0);
  mixer2.gain(3, 0);
  neopixel.begin();
  neopixel.clear();
  neopixel.show();
  notefreq1.begin(.15); 
}


//LOOP
void loop() {

  Serial.println(peakMap);
  


  bypassBit();
  bypassVerb();
  byBitStatestep();
  byVerbStatestep();
  potChecker();
  checkchannel();
  checkCrossfade();
  checkBit();
  freeverb();
  biquadFilterCheck();
  vu();

}


//--------------------Effect Switching Code
void potChecker() {

  if (analogRead(pot1) > lockedValue1 + 50 or analogRead(pot1) < lockedValue1 - 50) {
    pot1Locked = LOW;
  }

  if (pot2Locked == HIGH) {
    if (analogRead(pot2) > lockedValue2 + 50 or analogRead(pot2) < lockedValue2 - 50) {
      pot3Locked = LOW;
    }
  }

  if (pot1Locked == LOW) {
    // KNOB 1 CONTROLS
    if (effectStep == 0) {
      frequency = analogRead(pot1);
      //Pot 2
      int resMap = map(analogRead(pot2), 0, 1023, 1, 150);
      resonance = resMap;
    }
    if (effectStep == 1) {
      int rateMapped = map(analogRead(pot2), 0, 1023, 0, 7);
      current_SampleRate = rateMapped;
      //Pot 2
      int crushMapped = map(analogRead(pot1), 0, 1023, 16, 0);
      current_CrushedBitz = crushMapped;
    }
    if (effectStep == 2) {
      float RevMaps = analogRead(pot1);
      float Revmapped = map(RevMaps, 0.0, 1023.0, 0.0, 1.0);
      seconds = Revmapped;
      //Pot 2
      float RFilter = analogRead(pot2);
      float revFilter = map(RFilter, 0.0, 1023.0, 0.0, 1.0);
      damp = revFilter;
    }
  }
}


void checkchannel() {
  LastchannelState = channelState;
  channelState = digitalRead(channelButton);
  if (LastchannelState == LOW and channelState == HIGH) {
    analogWrite(channelLeds[cChannelStep], 0);
    switchChannelstep();
    analogWrite(channelLeds[cChannelStep], 250);
  }
  if (LastchannelState == HIGH and channelState == LOW) {
    delay(3);
  }
}

void switchChannelstep() {

  lockedValue1 = analogRead(pot1);
  lockedValue2 = analogRead(pot2);

  pot1Locked = HIGH;
  pot2Locked = HIGH;
  pot3Locked = HIGH;


  if (cChannelStep == totalCSteps - 1) {
    cChannelStep = 0;
    effectStep = 0;
  } else {
    cChannelStep = cChannelStep + 1;
    effectStep = effectStep + 1;
  }
  // Saving the value of Effect Parameters
}


//----------------Effect Coding


// -1. BitCrusher (Working, maybe some tune ups)
void checkBit() {
  bitcrusher1.sampleRate(current_SampleRate);
  bitcrusher1.bits(current_CrushedBitz);
}

//-1. (Working-Show Mark and ask about inherat reverb)
void freeverb() {
  freeverb1.roomsize(seconds);
  freeverb1.damping(damp);
}


void biquadFilterCheck() {
  HighLowSwithch = digitalRead(switchHL);
  filterOnOff = digitalRead(switchOnOffFilter);
  volumePot1 = analogRead(pot5);
  volumePot2 = analogRead(pot4);
  HFdB = map(volumePot1, 0, 1023, -10, 10);
  LFdB = map(volumePot2, 0, 1023, -6, 5);

  //If Filter On
  if (filterOnOff == 0) {  // On
    biquad1.setLowShelf(0, 1000, LFdB, .4);
    biquad1.setHighShelf(1, 12000, HFdB, .4);
    delay(10);
  }
  if (filterOnOff == 1) {
    biquad1.setHighShelf(1, 4000, 0, 1);
    biquad1.setLowShelf(0, 7000, 0, 1);
    delay(10);
  }
}


void bypassBit() {
  byBitLastchannelState = byBitState;
  byBitState = digitalRead(byBitButton);
  if (byBitLastchannelState == LOW and byBitState == HIGH) {
    byBit = !byBit;
    delay(10);
  }
  if (byBitLastchannelState == HIGH and byBitState == LOW) {
    delay(10);
  }
}

void byBitStatestep() {
  //Turning Bypass Off
  if (byBit == HIGH) {
    digitalWrite(byPassLED1, LOW);
    mixer1.gain(0, 1);
    mixer1.gain(1, 0);
    mixer1.gain(2, 0);
    mixer1.gain(3, 0);
  }
  //Turning Bypass On
  if (byBit == LOW) {
    digitalWrite(byPassLED1, HIGH);
    mixer1.gain(0, 0);
    mixer1.gain(1, 1);
    mixer1.gain(2, 0);
    mixer1.gain(3, 0);
  }
}

void bypassVerb() {

  byVerbLastchannelState = byVerbState;
  byVerbState = digitalRead(byVerbButton);
  if (byVerbLastchannelState == LOW and byVerbState == HIGH) {
    byRev = !byRev;
  }
  if (byVerbLastchannelState == HIGH and byVerbState == LOW) {
    delay(3);
  }
}

  void byVerbStatestep() {
    //Turning Bypass Off
    if (byRev == HIGH) {
      digitalWrite(byPassLED2, LOW);
      mixer3.gain(0, 1);
      mixer3.gain(1, 0);
      mixer3.gain(2, 0);
      mixer3.gain(3, 0);
    }
    //Turning Bypass On
    if (byRev == LOW) {
      digitalWrite(byPassLED2, HIGH);
      mixer3.gain(0, 0);
      mixer3.gain(1, 1);
      mixer3.gain(2, 0);
      mixer3.gain(3, 0);
    }
  }

void vu() {
  if (notefreq1.available()) {
    float note = notefreq1.read();
    noteMap = map(note, 0, 700, 0, 255);
    noteMap2 = map(note, 500, 0, 255, 0);
  }

  peakMap = map(peak1.read(),0.0, 1.0 , 0, 8);
  for(int i = 0; i <= peakMap; i++ ){
    neopixel.setPixelColor(i ,noteMap, 100, noteMap2);
  delay(10);
   neopixel.show();
  }
  neopixel.clear();
  
}


  void checkCrossfade() {
    float mixPot = 0.0;
    float mixPot2 = 0.0;
    float mixerMapPot = analogRead(pot3);
    mixPot = map(mixerMapPot, 0, 1023, 0, 1);
    mixPot2 = map(mixerMapPot, 0, 1023, 1, 0);
    mixer2.gain(0, mixPot2);
    mixer2.gain(1, mixPot);
    mixer2.gain(2, 0);
    mixer2.gain(3, 0);
  }