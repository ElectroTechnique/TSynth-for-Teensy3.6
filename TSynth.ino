// Tools menu Settings:
// Board: "Teensy3.6"
// USB Type: "Serial + MIDI + Audio"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <MIDI.h>
#include <USBHost_t36.h>
#include "MidiCC.h"
#include "AudioPatching.h"
#include "Constants.h"
#include "Parameters.h"
#include "ST7735Display.h"
#include "HWControls.h"
#include "PatchHandler.h"

//Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
boolean cardStatus = false;

//USB HOST MIDI Class Compliant
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
MIDIDevice midi1(myusb);

//MIDI 5 Pin DIN
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

struct VoiceAndNote {
  int note;
  long timeOn;
};

struct VoiceAndNote voices[NO_OF_VOICES] = {
  { -1, 0},
  { -1, 0},
  { -1, 0},
  { -1, 0}
};

int  prevNote = 48;//Initialised to middle value
float previousMillis = millis();//For MIDI Clk Sync
File patchFile;
int count = 0;
byte channel = MIDI_CHANNEL_OMNI;
byte programNo = 1;
int voiceToReturn = -1;//Initialise to 'null'
long earliestTime = millis();//For voice allocation - initialise to now

void setup() {
  Serial.begin(115200);
  //ST7735 Display
  setupDisplay();
  setupHardware();

  AudioMemory(30);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.4);

  // Configure SPI
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  Sd2Card card;
  cardStatus = card.init(SPI_FULL_SPEED, SDCARD_CS_PIN);
  if (cardStatus) {
    Serial.println("SD card is connected");
  } else {
    Serial.println("SD card is not connected or unusable");
  }

  //USB HOST MIDI Class Compliant
  delay(200);//Wait to turn on USB Host
  myusb.begin();
  midi1.setHandleControlChange(myControlChange);
  midi1.setHandleNoteOff(myNoteOff);
  midi1.setHandleNoteOn(myNoteOn);
  midi1.setHandlePitchChange(myPitchBend);
  midi1.setHandleProgramChange(myProgramChange);
  // midi1.setHandleClock(myMIDIClock);
  Serial.println("USB HOST MIDI Class Compliant");

  //USB Client MIDI
  usbMIDI.setHandleControlChange(myControlChange);
  usbMIDI.setHandleNoteOff(myNoteOff);
  usbMIDI.setHandleNoteOn(myNoteOn);
  usbMIDI.setHandlePitchChange(myPitchBend);
  usbMIDI.setHandleProgramChange(myProgramChange);
  usbMIDI.setHandleClock(myMIDIClock);
  Serial.println("USB Client MIDI");

  //MIDI 5 Pin DIN
  //  MIDI.begin();
  //  MIDI.setHandleNoteOn(myNoteOn);
  //  MIDI.setHandleNoteOff(myNoteOff);
  //  MIDI.setHandlePitchBend(myPitchBend);
  //  MIDI.setHandleControlChange(myControlChange);
  //  MIDI.setHandleProgramChange(myProgramChange);
  // MIDI.setHandleClock(myMIDIClock);
  //  Serial.println("MIDI DIN");

  constant1Dc.amplitude(CONSTANTONE);

  voiceMixer.gain(0, VOICEMIXERLEVEL);
  voiceMixer.gain(1, VOICEMIXERLEVEL);
  voiceMixer.gain(2, VOICEMIXERLEVEL);
  voiceMixer.gain(3, VOICEMIXERLEVEL);

  pwmLfo.amplitude(CONSTANTONE);
  pwmLfo.begin(WAVEFORM_TRIANGLE);// set waveform as Triangle

  waveformMod1a.amplitude(CONSTANTONE);
  waveformMod1a.frequency(440);
  waveformMod1a.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod1a.arbitraryWaveform(HARMONIC_WAVE, 172.0);
  waveformMod1a.begin(vcoWaveformA);
  waveformMod1b.amplitude(CONSTANTONE);
  waveformMod1b.frequency(440);
  waveformMod1b.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod1b.arbitraryWaveform(PARABOLIC_WAVE, 172.0);
  waveformMod1b.begin(vcoWaveformB);

  waveformMod2a.amplitude(CONSTANTONE);
  waveformMod2a.frequency(440);
  waveformMod2a.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod2a.arbitraryWaveform(HARMONIC_WAVE, 172.0);
  waveformMod2a.begin(vcoWaveformA);
  waveformMod2b.amplitude(CONSTANTONE);
  waveformMod2b.frequency(440);
  waveformMod2b.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod2b.arbitraryWaveform(PARABOLIC_WAVE, 172.0);
  waveformMod2b.begin(vcoWaveformB);

  waveformMod3a.amplitude(CONSTANTONE);
  waveformMod3a.frequency(440);
  waveformMod3a.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod3a.arbitraryWaveform(HARMONIC_WAVE, 172.0);
  waveformMod3a.begin(vcoWaveformA);
  waveformMod3b.amplitude(CONSTANTONE);
  waveformMod3b.frequency(440);
  waveformMod3b.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod3b.arbitraryWaveform(PARABOLIC_WAVE, 172.0);
  waveformMod3b.begin(vcoWaveformB);

  waveformMod4a.amplitude(CONSTANTONE);
  waveformMod4a.frequency(440);
  waveformMod4a.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod4a.arbitraryWaveform(HARMONIC_WAVE, 172.0);
  waveformMod4a.begin(vcoWaveformA);
  waveformMod4b.amplitude(CONSTANTONE);
  waveformMod4b.frequency(440);
  waveformMod4b.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod4b.arbitraryWaveform(PARABOLIC_WAVE, 172.0);
  waveformMod4b.begin(vcoWaveformB);

  filter1.octaveControl(FILTEROCTAVERANGE);
  filter2.octaveControl(FILTEROCTAVERANGE);
  filter3.octaveControl(FILTEROCTAVERANGE);
  filter4.octaveControl(FILTEROCTAVERANGE);

  waveformMixer1.gain(2, CONSTANTONE);//Noise
  waveformMixer1.gain(3, 0);//Ring Mod
  waveformMixer2.gain(2, CONSTANTONE);//Noise
  waveformMixer2.gain(3, 0);//Ring Mod
  waveformMixer3.gain(2, CONSTANTONE);//Noise
  waveformMixer3.gain(3, 0);//Ring Mod
  waveformMixer4.gain(2, CONSTANTONE);//Noise
  waveformMixer4.gain(3, 0);//Ring Mod

  vcfModMixer1.gain(1, CONSTANTONE);//LFO
  vcfModMixer1.gain(3, 0);//Not used
  vcfModMixer2.gain(1, CONSTANTONE);//LFO
  vcfModMixer2.gain(3, 0);//Not used
  vcfModMixer3.gain(1, CONSTANTONE);//LFO
  vcfModMixer3.gain(3, 0);//Not used
  vcfModMixer4.gain(1, CONSTANTONE);//LFO
  vcfModMixer4.gain(3, 0); //Not used

  filterMixer1.gain(3, 0);//Not used
  filterMixer2.gain(3, 0);//Not used
  filterMixer3.gain(3, 0);//Not used
  filterMixer4.gain(3, 0);//Not used

  chorusL.begin(delayLineL, CHORUS_DELAY_LENGTH_L, fxAmt);
  chorusR.begin(delayLineR, CHORUS_DELAY_LENGTH_R, fxAmt);

  chorusMixerL.gain(2, 0);
  chorusMixerL.gain(3, 0);
  chorusMixerR.gain(2, 0);
  chorusMixerR.gain(3, 0);

  reinitialiseHW();
  renderCurrentPatchPage(programNo, patchName);
}

void myNoteOn(byte channel, byte note, byte velocity) {
  //Check for out of range notes
  if (note + vcoOctaveA < 0 || note + vcoOctaveA > 127 || note + vcoOctaveB < 0 || note + vcoOctaveB > 127) return;

  AudioNoInterrupts();
  //TODO NEED SEPARATE KEYTRACKING FOR EACH VOICE
  keytracking.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
  if (glideSpeed > 0) {
    glide.amplitude((prevNote - note) * DIV12);//Set glide to previous note frequency
    glide.amplitude(0, glideSpeed * GLIDEFACTOR);//Glide to current note
  }

  if (vcoLfoRetrig == 1) {
    vcoLfo.sync();
  }
  if (vcfLfoRetrig == 1) {
    vcfLfo.sync();
  }

  prevNote = note;
  if (unison == 0) {
    switch (getVoiceNo(-1)) {
      case 1:
        //Serial.println("ON 1");
        voices[0].note = note;
        voices[0].timeOn = millis();
        updateVoice1();
        vcfEnvelope1.noteOn();
        vcaEnvelope1.noteOn();
        break;
      case 2:
        //Serial.println("ON 2");
        voices[1].note = note;
        voices[1].timeOn = millis();
        updateVoice2();
        vcfEnvelope2.noteOn();
        vcaEnvelope2.noteOn();
        break;
      case 3:
        //Serial.println("ON 3");
        voices[2].note = note;
        voices[2].timeOn = millis();
        updateVoice3();
        vcfEnvelope3.noteOn();
        vcaEnvelope3.noteOn();
        break;
      case 4:
        //Serial.println("ON 4");
        voices[3].note = note;
        voices[3].timeOn = millis();
        updateVoice4();
        vcfEnvelope4.noteOn();
        vcaEnvelope4.noteOn();
        break;
    }
  } else {
    //UNISON MODE
    voices[0].note = note;
    voices[0].timeOn = millis();
    updateVoice1();
    voices[1].note = note;
    voices[1].timeOn = millis();
    updateVoice2();
    voices[2].note = note;
    voices[2].timeOn = millis();
    updateVoice3();
    voices[3].note = note;
    voices[3].timeOn = millis();
    updateVoice4();

    vcfEnvelope1.noteOn();
    vcfEnvelope2.noteOn();
    vcfEnvelope3.noteOn();
    vcfEnvelope4.noteOn();
    vcaEnvelope1.noteOn();
    vcaEnvelope2.noteOn();
    vcaEnvelope3.noteOn();
    vcaEnvelope4.noteOn();
  }
  AudioInterrupts();
}

void myNoteOff(byte channel, byte note, byte velocity) {
  AudioNoInterrupts();
  if (unison == 0) {
    switch (getVoiceNo(note)) {
      case 1:
        //Serial.println("OFF 1");
        vcfEnvelope1.noteOff();
        vcaEnvelope1.noteOff();
        voices[0].note = -1;
        break;
      case 2:
        //Serial.println("OFF 2");
        vcfEnvelope2.noteOff();
        vcaEnvelope2.noteOff();
        voices[1].note = -1;
        break;
      case 3:
        //Serial.println("OFF 3");
        vcfEnvelope3.noteOff();
        vcaEnvelope3.noteOff();
        voices[2].note = -1;
        break;
      case 4:
        // Serial.println("OFF 4");
        vcfEnvelope4.noteOff();
        vcaEnvelope4.noteOff();
        voices[3].note = -1;
        break;
    }
  } else {
    //UNISON MODE
    allNotesOff();
  }
  AudioInterrupts();
}

void allNotesOff() {
  vcfEnvelope1.noteOff();
  vcaEnvelope1.noteOff();
  vcfEnvelope2.noteOff();
  vcaEnvelope2.noteOff();
  vcfEnvelope3.noteOff();
  vcaEnvelope3.noteOff();
  vcfEnvelope4.noteOff();
  vcaEnvelope4.noteOff();

  voices[0].note = -1;
  voices[1].note = -1;
  voices[2].note = -1;
  voices[3].note = -1;
}

int getVoiceNo(int note) {
  voiceToReturn = -1;//Initialise to 'null'
  earliestTime = millis();//Initialise to now
  if (note == -1) {
    //NoteOn() - Get the oldest free voice (recent voices may be still on release stage)
    for (int i = 0; i <  NO_OF_VOICES; i++) {
      if (voices[i].note == -1) {
        if (voices[i].timeOn < earliestTime) {
          earliestTime = voices[i].timeOn;
          voiceToReturn = i;
        }
      }
    }
    if (voiceToReturn == -1) {
      //No free voices, need to steal oldest sounding voice
      earliestTime = millis();//Reinitialise
      for (int i = 0; i <  NO_OF_VOICES; i++) {
        if (voices[i].timeOn < earliestTime) {
          earliestTime = voices[i].timeOn;
          voiceToReturn = i;
        }
      }
    }
    return voiceToReturn + 1;
  } else {
    //NoteOff() - Get voice number from note
    for (int i = 0; i <  NO_OF_VOICES; i++) {
      if (voices[i].note == note) {
        return i + 1;
      }
    }
  }
  //Shouldn't get here, return voice 1
  return 1;
}

void updateVoice1() {
  waveformMod1a.frequency(NOTEFREQS[voices[0].note + vcoOctaveA]);
  if (unison == 1) {
    waveformMod1b.frequency(NOTEFREQS[voices[0].note + vcoOctaveB] * (detune + ((1 - detune) * 0.14)));
  } else {
    waveformMod1b.frequency(NOTEFREQS[voices[0].note + vcoOctaveB] * detune);
  }
}

void updateVoice2() {
  if (unison == 1) {
    waveformMod2a.frequency(NOTEFREQS[voices[1].note + vcoOctaveA] * (detune + ((1 - detune) * 0.28)));
    waveformMod2b.frequency(NOTEFREQS[voices[1].note + vcoOctaveB] * (detune + ((1 - detune) * 0.42)));
  } else {
    waveformMod2a.frequency(NOTEFREQS[voices[1].note + vcoOctaveA]);
    waveformMod2b.frequency(NOTEFREQS[voices[1].note + vcoOctaveB] * detune);
  }
}

void updateVoice3() {
  if (unison == 1) {
    waveformMod3a.frequency(NOTEFREQS[voices[2].note + vcoOctaveA] * (detune + ((1 - detune) * 0.56)));
    waveformMod3b.frequency(NOTEFREQS[voices[2].note + vcoOctaveB] * (detune + ((1 - detune) * 0.71)));
  } else {
    waveformMod3a.frequency(NOTEFREQS[voices[2].note + vcoOctaveA]);
    waveformMod3b.frequency(NOTEFREQS[voices[2].note + vcoOctaveB] * detune);
  }
}

void updateVoice4() {
  if (unison == 1) {
    waveformMod4a.frequency(NOTEFREQS[voices[3].note + vcoOctaveA] * (detune + ((1 - detune) * 0.86)));
  } else {
    waveformMod4a.frequency(NOTEFREQS[voices[3].note + vcoOctaveA]);
  }
  waveformMod4b.frequency(NOTEFREQS[voices[3].note + vcoOctaveB] * detune);
}

int getLFOWaveform(int value, int currentValue) {
  if (value >= 0 && value < 20) {
    return WAVEFORM_SINE;
  } else if (value >= 21 && value < 41) {
    return WAVEFORM_TRIANGLE;
  } else if (value >= 42 && value < 63) {
    return WAVEFORM_SAWTOOTH_REVERSE;
  } else if (value >= 64 && value < 84) {
    return WAVEFORM_SAWTOOTH;
  } else if (value >= 85 && value < 105) {
    return WAVEFORM_SQUARE;
  } else if (value >= 106) {
    return WAVEFORM_SAMPLE_HOLD;
  } else {
    return currentValue;
  }
}

String getLFOWaveformStr(int value) {
  switch (value) {
    case WAVEFORM_SINE:
      return "Sine";
    case WAVEFORM_TRIANGLE:
      return "Triangle";
    case WAVEFORM_SAWTOOTH_REVERSE:
      return "Sawtooth";
    case WAVEFORM_SAWTOOTH:
      return "Ramp";
    case WAVEFORM_SQUARE:
      return "Square";
    case WAVEFORM_SAMPLE_HOLD:
      return "S & H";
    default:
      return "ERRLFOWAVE";
  }
}

String getWaveformAStr(int value) {
  switch (value) {
    case WAVEFORM_SINE:
      return "Sine";
    case WAVEFORM_SQUARE:
      return "Square";
    case WAVEFORM_SAWTOOTH_REVERSE:
      return "Sawtooth";
    case WAVEFORM_PULSE:
      return "Pulse";
    case WAVEFORM_TRIANGLE_VARIABLE:
      return "Var Triangle";
    case WAVEFORM_ARBITRARY:
      return "USER";
    default:
      return "ERRWAVEA";
  }
}

String getWaveformBStr(int value) {
  switch (value) {
    case WAVEFORM_SAMPLE_HOLD:
      return "S & H";
    case WAVEFORM_SQUARE:
      return "Square";
    case WAVEFORM_SAWTOOTH:
      return "Ramp";
    case WAVEFORM_PULSE:
      return "Pulse";
    case WAVEFORM_TRIANGLE_VARIABLE:
      return "Var Triangle";
    case WAVEFORM_ARBITRARY:
      return "USER";
    default:
      return "ERRWAVEB";
  }
}

float getLFOTempoRate(int value) {
  lfoTempoValue = LFOTEMPO[value];
  return lfoSyncFreq * LFOTEMPO[value];
}

int getVCOWaveformA(int value, int currentValue) {
  //Dead zones in between values to avoid instability
  if (value >= 0 && value < 20) {
    return WAVEFORM_SINE;
  } else if (value >= 21 && value < 41) {
    return WAVEFORM_SQUARE;
  } else if (value >= 42 && value < 62) {
    return WAVEFORM_SAWTOOTH;
  } else if (value >= 63 && value < 84) {
    return WAVEFORM_PULSE;
  } else if (value >= 85 && value < 106) {
    return WAVEFORM_TRIANGLE_VARIABLE;
  } else if (value >= 107) {
    return WAVEFORM_ARBITRARY;
  } else {
    return currentValue;
  }
}
int getVCOWaveformB(int value, int currentValue) {
  //Dead zones in between values to avoid instability
  if (value >= 0 && value < 20) {
    return WAVEFORM_SAMPLE_HOLD;
  } else if (value >= 21 && value < 41) {
    return WAVEFORM_SQUARE;
  } else if (value >= 42 && value < 62) {
    return WAVEFORM_SAWTOOTH_REVERSE;
  } else if (value >= 63 && value < 84) {
    return WAVEFORM_PULSE;
  } else if (value >= 85 && value < 106) {
    return WAVEFORM_TRIANGLE_VARIABLE;
  } else if (value >= 107) {
    return WAVEFORM_ARBITRARY;
  } else {
    return currentValue;
  }
}

int getVCOOctave(int value, int currentValue) {
  //Dead zones in between values to avoid instability
  if (value >= 0 && value < 24) {
    return -24;
  } else if (value >= 25 && value < 50) {
    return -12;
  } else if (value >= 51 && value < 76) {
    return 0;
  } else if (value >= 77 && value < 102) {
    return 12;
  } else if (value >= 103) {
    return 24;
  } else {
    return currentValue;
  }
}

void setPwmMixerALFO(float value) {
  pwMixer1a.gain(0, value);
  pwMixer2a.gain(0, value);
  pwMixer3a.gain(0, value);
  pwMixer4a.gain(0, value);
  renderCurrentParameterPage("PWM1 LFO", String(value));
}
void setPwmMixerBLFO(float value) {
  pwMixer1b.gain(0, value);
  pwMixer2b.gain(0, value);
  pwMixer3b.gain(0, value);
  pwMixer4b.gain(0, value);
  renderCurrentParameterPage("PWM2 LFO", String(value));
}

void setPwmMixerAPW(float value) {
  pwMixer1a.gain(1, value);
  pwMixer2a.gain(1, value);
  pwMixer3a.gain(1, value);
  pwMixer4a.gain(1, value);
  renderCurrentParameterPage("PW1", String(value));
}

void setPwmMixerBPW(float value) {
  pwMixer1b.gain(1, value);
  pwMixer2b.gain(1, value);
  pwMixer3b.gain(1, value);
  pwMixer4b.gain(1, value);
  renderCurrentParameterPage("PW2", String(value));
}

void setPwmMixerAFEnv(float value) {
  pwMixer1a.gain(2, value);
  pwMixer2a.gain(2, value);
  pwMixer3a.gain(2, value);
  pwMixer4a.gain(2, value);
  renderCurrentParameterPage("PWM1 F Env", String(value));
}

void setPwmMixerBFEnv(float value) {
  pwMixer1b.gain(2, value);
  pwMixer2b.gain(2, value);
  pwMixer3b.gain(2, value);
  pwMixer4b.gain(2, value);
  renderCurrentParameterPage("PWM2 F Env", String(value));
}

void updateUnison() {
  if (unison == 0) {
    allNotesOff();
    renderCurrentParameterPage("Unison", "Off");
  }
  renderCurrentParameterPage("Unison", "On");
}


void updateGlide() {
  renderCurrentParameterPage("Glide", String(glideSpeed) + "ms");
}

void updateWaveformA() {
  waveformMod1a.begin(vcoWaveformA);
  waveformMod2a.begin(vcoWaveformA);
  waveformMod3a.begin(vcoWaveformA);
  waveformMod4a.begin(vcoWaveformA);
  renderCurrentParameterPage("Osc1 Wave", getWaveformAStr(vcoWaveformA));
}

void updateWaveformB() {
  waveformMod1b.begin(vcoWaveformB);
  waveformMod2b.begin(vcoWaveformB);
  waveformMod3b.begin(vcoWaveformB);
  waveformMod4b.begin(vcoWaveformB);
  renderCurrentParameterPage("Osc2 Wave", getWaveformBStr(vcoWaveformB));
}

void updateOctaveA() {
  //update waveforms with new frequencies if notes are on
  if (voices[0].note != -1 ||  voices[1].note != -1 || voices[2].note != -1 || voices[3].note != -1) {
    updateVoice1();
    updateVoice2();
    updateVoice3();
    updateVoice4();
  }
  renderCurrentParameterPage("Osc1 Octave", String(vcoOctaveA));
}

void updateOctaveB() {
  //update waveforms with new frequencies if notes are on
  if (voices[0].note != -1 ||  voices[1].note != -1 || voices[2].note != -1 || voices[3].note != -1) {
    updateVoice1();
    updateVoice2();
    updateVoice3();
    updateVoice4();
  }
  renderCurrentParameterPage("Osc2 Octave", String(vcoOctaveB));
}


void updateDetune() {
  //update waveforms with new frequencies if notes are on
  if (voices[0].note != -1 ||  voices[1].note != -1 || voices[2].note != -1 || voices[3].note != -1) {
    updateVoice1();
    updateVoice2();
    updateVoice3();
    updateVoice4();
  }
  renderCurrentParameterPage("Detune", String(detune));
}

void updatePWMSource() {
  if (pwmSource == PWMSOURCELFO) {
    setPwmMixerAFEnv(0);
    setPwmMixerBFEnv(0);
    if (pwmRate >= 0.01) {
      setPwmMixerALFO(pwmAmtA);
      setPwmMixerBLFO(pwmAmtB);
    }
    renderCurrentParameterPage("PWM Source", "LFO");
  } else {
    setPwmMixerALFO(0);
    setPwmMixerBLFO(0);
    if (pwmRate >= 0.01) {
      setPwmMixerAFEnv(pwmAmtA);
      setPwmMixerBFEnv(pwmAmtB);
    }
    renderCurrentParameterPage("PWM Source", "Filter Env");
  }
}

void updatePWMRate() {
  pwmLfo.frequency(pwmRate);
  if (pwmRate < 0.01) {
    //Set to fixed PW mode
    setPwmMixerALFO(0);
    setPwmMixerBLFO(0);
    setPwmMixerAFEnv(0);
    setPwmMixerBFEnv(0);
    setPwmMixerAPW(1);
    setPwmMixerBPW(1);
  } else {
    setPwmMixerAPW(0);
    setPwmMixerBPW(0);
    if (pwmSource == PWMSOURCELFO) {
      setPwmMixerAFEnv(0);
      setPwmMixerBFEnv(0);
      setPwmMixerALFO(pwmAmtA);
      setPwmMixerBLFO(pwmAmtB);
    } else {
      //Filter env mod - pwmRate does nothing
      setPwmMixerALFO(0);
      setPwmMixerBLFO(0);
      setPwmMixerAFEnv(pwmAmtA);
      setPwmMixerBFEnv(pwmAmtB);
    }
  }
  renderCurrentParameterPage("PWM Rate", String(pwmRate) + "Hz");
}

void updatePWMAmount() {
  //MIDI only
  pwA = 0;
  pwB = 0;
  setPwmMixerALFO(pwmAmtA);
  setPwmMixerBLFO(pwmAmtB);
  renderCurrentParameterPage("PWM Amt", String(pwmAmtA) + " " + String(pwmAmtB));
}

void updatePWA() {
  if (pwmRate < 0.01) {
    //if PWM amount is around zero, fixed PW is enabled
    setPwmMixerALFO(0);
    setPwmMixerBLFO(0);
    setPwmMixerAFEnv(0);
    setPwmMixerBFEnv(0);
    setPwmMixerAPW(1);
    setPwmMixerBPW(1);
  } else {
    setPwmMixerAPW(0);
    setPwmMixerBPW(0);
    if (pwmSource == PWMSOURCELFO) {
      //PW alters PWM LFO amount for waveform A
      setPwmMixerALFO(pwmAmtA);
    } else {
      //PW alters PWM FEnv amount for waveform A
      setPwmMixerAFEnv(pwmAmtA);
    }
  }
  pwa.amplitude(pwA);
  renderCurrentParameterPage("Osc1 PW Amt", pwA);
}

void updatePWB() {
  if (pwmRate < 0.01) {
    //if PWM amount is around zero, fixed PW is enabled
    setPwmMixerALFO(0);
    setPwmMixerBLFO(0);
    setPwmMixerAFEnv(0);
    setPwmMixerBFEnv(0);
    setPwmMixerAPW(1);
    setPwmMixerBPW(1);
  } else {
    setPwmMixerAPW(0);
    setPwmMixerBPW(0);
    if (pwmSource == PWMSOURCELFO) {
      //PW alters PWM LFO amount for waveform B
      setPwmMixerBLFO(pwmAmtB);
    } else {
      //PW alters PWM FEnv amount for waveform B
      setPwmMixerBFEnv(pwmAmtB);
    }
  }
  pwb.amplitude(pwB);
  renderCurrentParameterPage("Osc2 PW Amt", pwB);
}

void updateVcoLevelA() {
  waveformMixer1.gain(0, VCOALevel);
  waveformMixer2.gain(0, VCOALevel);
  waveformMixer3.gain(0, VCOALevel);
  waveformMixer4.gain(0, VCOALevel);
  renderCurrentParameterPage("Osc1 Level", VCOALevel);
}


void updateVcoLevelB() {
  waveformMixer1.gain(1, VCOBLevel);
  waveformMixer2.gain(1, VCOBLevel);
  waveformMixer3.gain(1, VCOBLevel);
  waveformMixer4.gain(1, VCOBLevel);
  renderCurrentParameterPage("Osc2 Level", VCOBLevel);
}

void updateNoiseLevel() {
  pink.amplitude(noiseLevel);
  renderCurrentParameterPage("Noise Level", noiseLevel);
}

void updateFilterFreq() {
  filter1.frequency(filterFreq);
  filter2.frequency(filterFreq);
  filter3.frequency(filterFreq);
  filter4.frequency(filterFreq);
  renderCurrentParameterPage("Cutoff", String(filterFreq) + "Hz");
}


void updateFilterRes() {
  filter1.resonance(filterRes);
  filter2.resonance(filterRes);
  filter3.resonance(filterRes);
  filter4.resonance(filterRes);
  renderCurrentParameterPage("Resonance", filterRes);
}

void updateFilterMixer() {
  float LP = 1.0f;
  float BP = 0;
  float HP = 0;
  String filterStr;
  if (filterMix > LINEAR_FILTERMIXER[125]) {
    //BP mode
    LP = 0;
    BP = 1.0f;
    HP = 0;
    filterStr = "Band Pass";
  } else {
    //LP-HP mix mode
    LP = 1.0f - filterMix;
    BP = 0;
    HP = filterMix;
    if (filterMix == LINEAR_FILTERMIXER[0]) {
      filterStr = "Low Pass";
    } else if (filterMix == LINEAR_FILTERMIXER[125]) {
      filterStr = "High Pass";
    } else {
      filterStr = "LP " + String(100 - filterMixStr) + ":" + String(filterMixStr) + " HP" ;
    }
  }
  filterMixer1.gain(0, LP);
  filterMixer1.gain(1, BP);
  filterMixer1.gain(2, HP);
  filterMixer2.gain(0, LP);
  filterMixer2.gain(1, BP);
  filterMixer2.gain(2, HP);
  filterMixer3.gain(0, LP);
  filterMixer3.gain(1, BP);
  filterMixer3.gain(2, HP);
  filterMixer4.gain(0, LP);
  filterMixer4.gain(1, BP);
  filterMixer4.gain(2, HP);
  renderCurrentParameterPage("Filter Type", filterStr);
}

void updateFilterEnv() {
  vcfModMixer1.gain(0, filterEnv);
  vcfModMixer2.gain(0, filterEnv);
  vcfModMixer3.gain(0, filterEnv);
  vcfModMixer4.gain(0, filterEnv);
  renderCurrentParameterPage("Filter Env", String(filterEnv));
}

void updateKeyTracking() {
  vcfModMixer1.gain(2, keytrackingAmount);
  vcfModMixer2.gain(2, keytrackingAmount);
  vcfModMixer3.gain(2, keytrackingAmount);
  vcfModMixer4.gain(2, keytrackingAmount);
  renderCurrentParameterPage("Key Track", String(keytrackingAmount));
}

void updateVcoLFOAmt() {
  vcoLfo.amplitude(vcoLfoAmt);
  renderCurrentParameterPage("LFO Amt", String(vcoLfoAmt));
}

void updateVcoLFORate() {
  vcoLfo.frequency(vcoLfoRate);
  renderCurrentParameterPage("LFO Rate", String(vcoLfoRate) + "Hz");
}

void updateVcoLFOWaveform() {
  vcoLfo.begin(vcoLFOWaveform);
  renderCurrentParameterPage("Pitch LFO", getLFOWaveformStr(vcoLFOWaveform));
}

void updateVcfLfoRate() {
  vcfLfo.frequency(vcfLfoRate);
  renderCurrentParameterPage("LFO Rate", String(vcfLfoRate) + "ms");
}

void updateVcfLfoAmt() {
  vcfLfo.amplitude(vcfLfoAmt);
  renderCurrentParameterPage("LFO Amt", String(vcfLfoAmt));
}

void updateVcfLFOWaveform() {
  vcfLfo.begin(vcfLfoWaveform);
  renderCurrentParameterPage("Filter LFO", getLFOWaveformStr(vcfLfoWaveform));
}

void updateVcoLFORetrig() {
  renderCurrentParameterPage("Pit LFO Retrig", vcoLfoRetrig == 1 ? "On" : "Off");
}

void updateVcfLFOMidiClkSync() {
  renderCurrentParameterPage("Fil LFO Clk Sync", vcfLFOMidiClkSync == 1 ? "On" : "Off");
}

void updateVcfAttack() {
  vcfEnvelope1.attack(vcfAttack);
  vcfEnvelope2.attack(vcfAttack);
  vcfEnvelope3.attack(vcfAttack);
  vcfEnvelope4.attack(vcfAttack);
  renderCurrentParameterPage("Filter Att", String(vcfAttack) + "ms");
}

void updateVcfDecay() {
  vcfEnvelope1.decay(vcfDecay);
  vcfEnvelope2.decay(vcfDecay);
  vcfEnvelope3.decay(vcfDecay);
  vcfEnvelope4.decay(vcfDecay);
  renderCurrentParameterPage("Filter Dec", String(vcfDecay) + "ms");
}

void updateVcfSustain() {
  vcfEnvelope1.sustain(vcfSustain);
  vcfEnvelope2.sustain(vcfSustain);
  vcfEnvelope3.sustain(vcfSustain);
  vcfEnvelope4.sustain(vcfSustain);
  renderCurrentParameterPage("Filter Sus", String(vcfSustain));
}

void updateVcfRelease() {
  vcfEnvelope1.release(vcfRelease);
  vcfEnvelope2.release(vcfRelease);
  vcfEnvelope3.release(vcfRelease);
  vcfEnvelope4.release(vcfRelease);
  renderCurrentParameterPage("Filter Rel", String(vcfRelease) + "ms");
}

void updateVcaAttack() {
  vcaEnvelope1.attack(vcaAttack);
  vcaEnvelope2.attack(vcaAttack);
  vcaEnvelope3.attack(vcaAttack);
  vcaEnvelope4.attack(vcaAttack);
  renderCurrentParameterPage("Attack", String(vcaAttack) + "ms");
}

void updateVcaDecay() {
  vcaEnvelope1.decay(vcaDecay);
  vcaEnvelope2.decay(vcaDecay);
  vcaEnvelope3.decay(vcaDecay);
  vcaEnvelope4.decay(vcaDecay);
  renderCurrentParameterPage("Decay", String(vcaDecay) + "ms");
}

void updateVcaSustain() {
  vcaEnvelope1.sustain(vcaSustain);
  vcaEnvelope2.sustain(vcaSustain);
  vcaEnvelope3.sustain(vcaSustain);
  vcaEnvelope4.sustain(vcaSustain);
  renderCurrentParameterPage("Sustain", String(vcaSustain));
}

void updateVcaRelease() {
  vcaEnvelope1.release(vcaRelease);
  vcaEnvelope2.release(vcaRelease);
  vcaEnvelope3.release(vcaRelease);
  vcaEnvelope4.release(vcaRelease);
  renderCurrentParameterPage("Release", String(vcaRelease) + "ms");
}

void updateRingMod() {
  if (ringMod == 1) {
    ringMod1.setCombineMode(AudioEffectDigitalCombine::XOR);
    ringMod2.setCombineMode(AudioEffectDigitalCombine::XOR);
    ringMod3.setCombineMode(AudioEffectDigitalCombine::XOR);
    ringMod4.setCombineMode(AudioEffectDigitalCombine::XOR);
    waveformMixer1.gain(3, RINGMODLEVEL);//Ring Mod
    waveformMixer2.gain(3, RINGMODLEVEL);//Ring Mod
    waveformMixer3.gain(3, RINGMODLEVEL);//Ring Mod
    waveformMixer4.gain(3, RINGMODLEVEL);//Ring Mod
    renderCurrentParameterPage("Ring Mod", "On");
  } else {
    ringMod1.setCombineMode(AudioEffectDigitalCombine::OFF);
    ringMod2.setCombineMode(AudioEffectDigitalCombine::OFF);
    ringMod3.setCombineMode(AudioEffectDigitalCombine::OFF);
    ringMod4.setCombineMode(AudioEffectDigitalCombine::OFF);
    waveformMixer1.gain(3, 0);//Ring Mod
    waveformMixer2.gain(3, 0);//Ring Mod
    waveformMixer3.gain(3, 0);//Ring Mod
    waveformMixer4.gain(3, 0);//Ring Mod
    renderCurrentParameterPage("Ring Mod", "Off");
  }
}

void updateFXAmt() {
  chorusL.voices(fxAmt);
  chorusR.voices(fxAmt);
  renderCurrentParameterPage("Effect Amt", String(fxAmt));
}

void updateFXMix() {
  chorusMixerL.gain(0, 1.0 - fxMix); //Dry
  chorusMixerL.gain(1, fxMix);//Wet
  chorusMixerR.gain(0, 1.0 - fxMix); //Dry
  chorusMixerR.gain(1, fxMix);//Wet
  renderCurrentParameterPage("Effect Mix", String(fxMix));
}

void updatePatchname() {
  renderCurrentPatchPage(programNo, patchName);
}

void myPitchBend(byte channel, int bend) {
  pitchBend.amplitude(bend * 0.5 * DIV8192);//)0.5 to give 1oct - spread of mod is 2oct
}

void myControlChange(byte channel, byte control, byte value) {
  Serial.println("MIDI: " + String(control) + ":" + String(value));
  AudioNoInterrupts();
  switch (control) {
    case CCvolume:
      sgtl5000_1.volume(0.8 * LINEAR[value]);
      break;

    case CCunison:
      value > 0 ? unison = 1 : unison = 0;
      updateUnison();
      break;

    case CCglide:
      glideSpeed = LINEAR[value];
      updateGlide();
      break;

    case CCvcowaveformA:
      vcoWaveformA = getVCOWaveformA(value, vcoWaveformA);
      updateWaveformA();
      break;

    case CCvcowaveformB:
      vcoWaveformB = getVCOWaveformB(value, vcoWaveformB);
      updateWaveformB();
      break;

    case CCoctaveA:
      vcoOctaveA = getVCOOctave(value, vcoOctaveA);
      updateOctaveA();
      break;

    case CCoctaveB:
      vcoOctaveB = getVCOOctave(value, vcoOctaveB);
      updateOctaveB();
      break;

    case CCdetune:
      detune = 1 -  (0.05 * LINEAR[value]);//up to 5% freq
      updateDetune();
      break;

    case CCpwmSource:
      value > 0  ? pwmSource = PWMSOURCEFENV : pwmSource = PWMSOURCELFO;
      updatePWMSource();
      break;

    case CCpwmRate:
      //Uses combination of PWMRate, PWa and PWb
      pwmRate = PWMMAXRATE * POWER[value];
      updatePWMRate();
      break;

    case CCpwmAmt:
      //NO FRONT PANEL CONTROL - MIDI ONLY
      //Uses combination of PWMRate, PWa and PWb
      pwmAmtA =  LINEAR[value];
      pwmAmtB =  LINEAR[value];
      updatePWMAmount();
      break;

    case CCpwA:
      pwA = (1.90 * LINEAR[value]) - 0.95;
      pwmAmtA =  LINEAR[value];
      updatePWA();
      break;

    case CCpwB:
      pwB =  (1.90 * LINEAR[value]) - 0.95;
      pwmAmtB = LINEAR[value];
      updatePWB();
      break;

    case CCvcoLevelA:
      VCOALevel = LINEAR[value];
      updateVcoLevelA();
      break;

    case CCvcoLevelB:
      VCOBLevel = LINEAR[value];
      updateVcoLevelB();
      break;

    case CCnoiseLevel:
      noiseLevel = LINEAR[value];
      updateNoiseLevel();
      break;

    case CCfilterfreq:
      filterFreq = FILTERFREQS[value];
      updateFilterFreq();
      break;

    case CCfilterres:
      filterRes = (3.9f * LINEAR[value]) + 1.1f;//If <1.1 there is noise at high cutoff freq
      updateFilterRes();
      break;

    case CCfiltermixer:
      filterMix = LINEAR_FILTERMIXER[value];
      filterMixStr = LINEAR_FILTERMIXERSTR[value];
      updateFilterMixer();
      break;

    case CCfilterenv:
      filterEnv = ((2 * LINEAR[value]) - 1) * VCFMODMIXERMAX;
      updateFilterEnv();
      break;

    case CCkeytracking:
      keytrackingAmount = LINEAR[value] * VCFMODMIXERMAX;
      updateKeyTracking();
      break;

    case CCmodwheel:
      vcoLfoAmt = POWER[value] * MODWHEELFACTOR;//Less from mod wheel
      updateVcoLFOAmt();
      break;

    case CCvcolfoamt:
      vcoLfoAmt = POWER[value];
      updateVcoLFOAmt();
      break;

    case CCvcoLfoRate:
      if (vcoLFOMidiClkSync == 1) {
        vcoLfoRate = getLFOTempoRate(value);
      } else {
        vcoLfoRate = 20 * POWER[value];
      }
      updateVcoLFORate();
      break;

    case CCvcoLfoWaveform:
      vcoLFOWaveform = getLFOWaveform(value, vcoLFOWaveform);
      updateVcoLFOWaveform();
      break;

    case CCvcolforetrig:
      value > 0 ? vcoLfoRetrig = 1 : vcoLfoRetrig = 0;
      updateVcoLFORetrig();
      break;

    case CCvcfLFOMidiClkSync:
      value > 0 ? vcfLFOMidiClkSync = 1 : vcfLFOMidiClkSync = 0;
      updateVcfLFOMidiClkSync();
      break;

    case CCvcflforate:
      if (vcfLFOMidiClkSync == 1) {
        vcfLfoRate = getLFOTempoRate(value);
      } else {
        vcfLfoRate = 30 * POWER[value];
      }
      updateVcfLfoRate();
      break;

    case CCvcflfoamt:
      vcfLfoAmt = LINEAR[value] * VCFMODMIXERMAX;
      updateVcfLfoAmt();
      break;

    case CCvcflfowaveform:
      vcfLfoWaveform = getLFOWaveform(value, vcfLfoWaveform);
      updateVcfLFOWaveform();
      break;

    case CCvcflforetrig:
      value > 0 ? vcfLfoRetrig = 1 : vcfLfoRetrig = 0;
      break;

    case CCvcoLFOMidiClkSync:
      value > 0 ? vcoLFOMidiClkSync = 1 : vcoLFOMidiClkSync = 0;
      break;

    case CCvcfattack:
      vcfAttack = ENVTIMES[value];
      updateVcfAttack();
      break;

    case CCvcfdecay:
      vcfDecay = ENVTIMES[value];
      updateVcfDecay();
      break;

    case CCvcfsustain:
      vcfSustain = LINEAR[value];
      updateVcfSustain();
      break;

    case CCvcfrelease:
      vcfRelease = ENVTIMES[value];
      updateVcfRelease();
      break;

    case CCvcaattack:
      vcaAttack = ENVTIMES[value];
      updateVcaAttack();
      break;

    case CCvcadecay:
      vcaDecay = ENVTIMES[value];
      updateVcaDecay();
      break;

    case CCvcasustain:
      vcaSustain = LINEAR[value];
      updateVcaSustain();
      break;

    case CCvcarelease:
      vcaRelease = ENVTIMES[value];
      updateVcaRelease();
      break;

    case CCringmod:
      value > 0 ? ringMod = 1 : ringMod = 0;
      updateRingMod();
      break;

    case CCfxamt:
      fxAmt = CHORUS_VOICES[value];
      updateFXAmt();
      break;

    case CCfxmix:
      fxMix = LINEAR[value];
      updateFXMix();
      break;

    case CCallnotesoff:
      allNotesOff();
      break;
  }
  AudioInterrupts();
}

void myProgramChange(byte channel, byte program) {
  Serial.print("Pgm:");
  Serial.println(program);
  recallPatch(program);
}

void myMIDIClock() {
  if ((vcoLFOMidiClkSync == 1 || vcfLFOMidiClkSync == 1) && count > 23) {
    float timeNow = millis();
    midiClkTimeInterval = (timeNow - previousMillis);
    lfoSyncFreq = 1000.0 / midiClkTimeInterval;
    previousMillis = timeNow;
    if (vcoLFOMidiClkSync == 1) vcoLfo.frequency(lfoSyncFreq * lfoTempoValue); //MIDI only
    if (vcfLFOMidiClkSync == 1) vcfLfo.frequency(lfoSyncFreq * lfoTempoValue);
    count = 0;
  }
  if (count < 24)count++;//prevent eventual overflow
}

void printPatches(File dir) {
  while (true) {
    File patch =  dir.openNextFile();
    if (! patch) {
      // no more files
      break;
    }
    Serial.print(patch.name());
    if (patch.isDirectory()) {
      Serial.println("/");
      printPatches(patch);
    } else {
      String data[NO_OF_PARAMS];    //Array of data read in
      recallPatchData(patch, data);
      Serial.println(" " + data[0]);
    }
    patch.close();
  }
}

int recallAvailablePatches() {
  if (!SD.begin(SDCARD_CS_PIN)) {
    Serial.println("SD card failed!");
    return;
  }
  printPatches(SD.open("/"));
}

void recallPatch(byte program) {
  if (!SD.begin(SDCARD_CS_PIN)) {
    Serial.println("SD card failed!");
    return;
  }
  patchFile = SD.open(program);
  if (!patchFile) {
    Serial.println("File not found");
  } else {
    String data[NO_OF_PARAMS];    //Array of data read in
    recallPatchData(patchFile, data);
    setCurrentPatchData(data);
    patchFile.close();
  }
}

void savePatch(byte programNo) {
  if (!SD.begin(SDCARD_CS_PIN)) {
    Serial.println("SD card failed!");
    return;
  }
  if (SD.exists(programNo)) {
    SD.remove(programNo);
  }
  patchFile = SD.open(programNo, FILE_WRITE);
  if (patchFile) {
    Serial.print("Writing Patch:");
    Serial.println(programNo);
    renderCurrentParameterPage("Writing Patch", String(programNo));
    patchFile.println(getCurrentPatchData());
    patchFile.close();
  }
}

void recallPatchData(File patchFile, String data[]) {
  //Read patch data from file and set current patch parameters

  size_t n;      // Length of returned field with delimiter.
  char str[20];  // Must hold longest field with delimiter and zero byte.
  int i = 0;
  while (patchFile.available() && i < NO_OF_PARAMS) {
    n = readField(&patchFile, str, sizeof(str), ",\n");
    // done if Error or at EOF.
    if (n == 0) break;

    // Print the type of delimiter.
    if (str[n - 1] == ',' || str[n - 1] == '\n') {
      //        Serial.print(str[n - 1] == ',' ? F("comma: ") : F("endl:  "));
      // Remove the delimiter.
      str[n - 1] = 0;
    } else {
      // At eof, too long, or read error.  Too long is error.
      //        Serial.print(patchFile.available() ? F("error: ") : F("eof:   "));
    }
    // Print the field.
    //      Serial.print(i);
    //      Serial.print(" - ");
    //      Serial.println(str);
    data[i++] = String(str);
  }
}

size_t readField(File * file, char* str, size_t size, char* delim) {
  char ch;
  size_t n = 0;
  while ((n + 1) < size && file->read(&ch, 1) == 1) {
    // Delete CR.
    if (ch == '\r') {
      continue;
    }
    str[n++] = ch;
    if (strchr(delim, ch)) {
      break;
    }
  }
  str[n] = '\0';
  return n;
}

void setCurrentPatchData(String data[]) {
  patchName = data[0];
  VCOALevel = data[1].toFloat();
  VCOBLevel = data[2].toFloat();
  noiseLevel = data[3].toFloat();
  unison = data[4].toInt();
  ringMod = data[5].toInt();
  detune = data[6].toFloat();
  lfoSyncFreq = data[7].toInt();
  midiClkTimeInterval = data[8].toInt();
  lfoTempoValue = data[9].toFloat();
  keytrackingAmount = data[10].toFloat();
  glideSpeed = data[11].toFloat();
  vcoOctaveA = data[12].toFloat();
  vcoOctaveB = data[13].toFloat();
  vcoWaveformA = data[14].toFloat();
  vcoWaveformB = data[15].toFloat();
  pwmSource = data[16].toInt();
  pwmAmtA = data[17].toFloat();
  pwmAmtB = data[18].toFloat();
  pwmRate = data[19].toFloat();
  pwA = data[20].toFloat();
  pwB = data[21].toFloat();
  filterRes = data[22].toFloat();
  filterFreq = data[23].toFloat();
  filterMix = data[24].toFloat();
  filterEnv = data[25].toFloat();
  vcoLfoAmt = data[26].toFloat();
  vcoLfoRate = data[27].toFloat();
  vcoLFOWaveform = data[28].toFloat();
  vcoLfoRetrig = data[29].toInt();
  vcoLFOMidiClkSync = data[30].toFloat();//MIDI Only
  vcfLfoRate = data[31].toFloat();
  vcfLfoRetrig = data[32].toInt();
  vcfLFOMidiClkSync = data[33].toFloat();
  vcfLfoAmt = data[34].toFloat();
  vcfLfoWaveform = data[35].toFloat();
  vcfAttack = data[36].toFloat();
  vcfDecay = data[37].toFloat();
  vcfSustain = data[38].toFloat();
  vcfRelease = data[39].toFloat();
  vcaAttack = data[40].toFloat();
  vcaDecay = data[41].toFloat();
  vcaSustain = data[42].toFloat();
  vcaRelease = data[43].toFloat();
  fxAmt = data[44].toFloat();
  fxMix = data[45].toFloat();

  updatePatchname();
  updateDisplay = false;//prevent rapid update of display
  updateUnison();
  updateWaveformA();
  updateWaveformB();
  updateOctaveA();
  updateOctaveB();
  updateDetune();
  updatePWMSource();
  updatePWMAmount();
  updatePWA();
  updatePWB();
  updatePWMRate();
  updateVcoLevelA();
  updateVcoLevelB();
  updateNoiseLevel();
  updateFilterFreq();
  updateFilterRes();
  updateFilterMixer();
  updateFilterEnv();
  updateKeyTracking();
  updateVcoLFOAmt();
  updateVcoLFORate();
  updateVcoLFOWaveform();
  updateVcfLfoRate();
  updateVcfLfoAmt();
  updateVcfLFOWaveform();
  updateVcfAttack();
  updateVcfDecay();
  updateVcfSustain();
  updateVcfRelease();
  updateVcaAttack();
  updateVcaDecay();
  updateVcaSustain();
  updateVcaRelease();
  updateRingMod();
  updateFXAmt();
  updateFXMix();
  updateDisplay = true;
  Serial.print("Set Patch: ");
  Serial.println(patchName);
}

String getCurrentPatchData() {
  return patchName + "," + String(VCOALevel)  + "," + String(VCOBLevel)  + "," + String(noiseLevel)  + "," + String(unison) + "," + String(ringMod) + "," + String(detune) + "," + String(lfoSyncFreq) + "," + String(midiClkTimeInterval) + "," + String(lfoTempoValue)  + "," + String(keytrackingAmount)  + "," + String(glideSpeed)  + "," + String(vcoOctaveA)  + "," + String(vcoOctaveB)  + "," + String(vcoWaveformA)  + "," + String(vcoWaveformB)  + "," +
         String(pwmSource)  + "," + String(pwmAmtA)  + "," + String(pwmAmtB)  + "," + String(pwmRate)  + "," + String(pwA)  + "," + String(pwB)  + "," + String(filterRes)  + "," + String(filterFreq)  + "," + String(filterMix)  + "," + String(filterEnv)  + "," + String(vcoLfoAmt)  + "," + String(vcoLfoRate)  + "," + String(vcoLFOWaveform)  + "," + String(vcoLfoRetrig)  + "," + String(vcoLFOMidiClkSync)  + "," + String(vcfLfoRate)  + "," +
         vcfLfoRetrig  + "," + vcfLFOMidiClkSync  + "," + vcfLfoAmt  + "," + vcfLfoWaveform  + "," + vcfAttack  + "," + vcfDecay  + "," + vcfSustain  + "," + vcfRelease  + "," + vcaAttack  + "," + vcaDecay  + "," + vcaSustain  + "," + vcaRelease  + "," +
         String(fxAmt)  + "," + String(fxMix);
}

void checkMux() {
  mux1Read = analogRead(MUX1_S);
  mux2Read = analogRead(MUX2_S);
  if (mux1Read > (mux1ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux1Read < (mux1ValuesPrev[muxInput] - QUANTISE_FACTOR)) {
    mux1ValuesPrev[muxInput] = mux1Read;
    mux1Read = (mux1Read >> 3); //Change range to 0-127

    Serial.print("MUX1:");
    Serial.print(muxInput);
    Serial.print(" :");
    Serial.println(mux1Read);

    switch (muxInput) {
      case MUX1_octaveA:
        myControlChange(channel, CCoctaveA, mux1Read);
        break;
      case MUX1_octaveB:
        myControlChange(channel, CCoctaveB, mux1Read);
        break;
      case MUX1_vcowaveformA:
        myControlChange(channel, CCvcowaveformA, mux1Read);
        break;
      case MUX1_vcowaveformB:
        myControlChange(channel, CCvcowaveformB, mux1Read);
        break;
      case MUX1_vcoLevelA:
        myControlChange(channel, CCvcoLevelA, mux1Read);
        break;
      case MUX1_vcoLevelB:
        myControlChange(channel, CCvcoLevelB, mux1Read);
        break;
      case MUX1_pwA:
        myControlChange(channel, CCpwA, mux1Read);
        break;
      case MUX1_pwB:
        myControlChange(channel, CCpwB, mux1Read);
        break;
      case MUX1_pwmRate:
        myControlChange(channel, CCpwmRate, mux1Read);
        break;
      case MUX1_vcoLfoRate:
        myControlChange(channel, CCvcoLfoRate, mux1Read);
        break;
      case MUX1_noiseLevel:
        myControlChange(channel, CCnoiseLevel, mux1Read);
        break;
      case MUX1_vcoLfoWaveform:
        myControlChange(channel, CCvcoLfoWaveform, mux1Read);
        break;
      case MUX1_vcolfoamt:
        myControlChange(channel, CCvcolfoamt, mux1Read);
        break;
      case MUX1_detune:
        myControlChange(channel, CCdetune, mux1Read);
        break;
      case MUX1_filterfreq:
        myControlChange(channel, CCfilterfreq, mux1Read);
        break;
      case MUX1_filterres:
        myControlChange(channel, CCfilterres, mux1Read);
        break;
    }
  }

  if (mux2Read > (mux2ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux2Read < (mux2ValuesPrev[muxInput] - QUANTISE_FACTOR)) {
    mux2ValuesPrev[muxInput] = mux2Read;
    mux2Read = (mux2Read >> 3); //Change range to 0-127
    Serial.print("MUX2:");
    Serial.print(muxInput);
    Serial.print(" :");
    Serial.println(mux2Read);

    switch (muxInput) {
      case MUX2_vcflfoamt:
        myControlChange(channel, CCvcflfoamt, mux2Read);
        break;
      case MUX2_vcflforate:
        myControlChange(channel, CCvcflforate, mux2Read);
        break;
      case MUX2_filterenv:
        myControlChange(channel, CCfilterenv, mux2Read);
        break;
      case MUX2_vcflfowaveform:
        myControlChange(channel, CCvcflfowaveform, mux2Read);
        break;
      case MUX2_filtermixer:
        myControlChange(channel, CCfiltermixer, mux2Read);
        break;
      case MUX2_keytracking:
        myControlChange(channel, CCkeytracking, mux2Read);
        break;
      case MUX2_vcfattack:
        myControlChange(channel, CCvcfattack, mux2Read);
        break;
      case MUX2_vcfdecay:
        myControlChange(channel, CCvcfdecay, mux2Read);
        break;
      case MUX2_vcfsustain:
        myControlChange(channel, CCvcfsustain, mux2Read);
        break;
      case MUX2_vcfrelease:
        myControlChange(channel, CCvcfrelease, mux2Read);
        break;
      case MUX2_vcaattack:
        myControlChange(channel, CCvcaattack, mux2Read);
        break;
      case MUX2_vcadecay:
        myControlChange(channel, CCvcadecay, mux2Read);
        break;
      case MUX2_vcasustain:
        myControlChange(channel, CCvcasustain, mux2Read);
        break;
      case MUX2_vcarelease:
        myControlChange(channel, CCvcarelease, mux2Read);
        break;
      case MUX2_glide:
        myControlChange(channel, CCglide, mux2Read);
        break;
      case MUX2_volume:
        myControlChange(channel, CCvolume, mux2Read);
        break;
    }
  }
  muxInput++;
  if (muxInput >= MUXCHANNELS)  muxInput = 0;

  digitalWrite(MUX_0, muxInput & B0001);
  digitalWrite(MUX_1, muxInput & B0010);
  digitalWrite(MUX_2, muxInput & B0100);
  digitalWrite(MUX_3, muxInput & B1000);
}

void checkFxPots() {
  fxAmtRead = analogRead(EFFECTAMT_POT);
  if (fxAmtRead > (fxAmtPrevious + QUANTISE_FACTOR) || fxAmtRead < (fxAmtPrevious - QUANTISE_FACTOR)) {
    fxAmtPrevious = fxAmtRead;
    fxAmtRead = (fxAmtRead >> 3); //Change range to 0-127
    myControlChange(channel, CCfxamt, fxAmtRead);
    Serial.println("EFFECTAMT_POT");
  }
  fxMixRead = analogRead(EFFECTMIX_POT);
  if (fxMixRead > (fxMixPrevious + QUANTISE_FACTOR) || fxMixRead < (fxMixPrevious - QUANTISE_FACTOR)) {
    fxMixPrevious = fxMixRead;
    fxMixRead = (fxMixRead >> 3); //Change range to 0-127
    myControlChange(channel, CCfxmix, fxMixRead);
    Serial.println("EFFECTMIX_POT");
  }
}

void checkSwitches() {
  pwmSourceSwitch.update();
  if (pwmSourceSwitch.risingEdge()) {
    pwmSource = PWMSOURCEFENV;
    myControlChange(channel, CCpwmSource, pwmSource);
    Serial.println("PWM_SOURCE_SW: " + String(pwmSource));
  } else if (pwmSourceSwitch.fallingEdge()) {
    pwmSource = PWMSOURCELFO;
    myControlChange(channel, CCpwmSource, pwmSource);
    Serial.println("PWM_SOURCE_SW: " + String(pwmSource));
  }

  unisonSwitch.update();
  if (unisonSwitch.risingEdge()) {
    unison = 1;
    myControlChange(channel, CCunison, unison);
    Serial.println("UNISON_SW: " + String(unison));
  } else if (unisonSwitch.fallingEdge()) {
    unison = 0;
    myControlChange(channel, CCunison, unison);
    Serial.println("UNISON_SW: " + String(unison));
  }

  ringModSwitch.update();
  if (ringModSwitch.risingEdge()) {
    ringMod = 1;
    myControlChange(channel, CCringmod, ringMod);
    Serial.println("RING_MOD_SW: " + String(ringMod));
  } else if (ringModSwitch.fallingEdge()) {
    ringMod = 0;
    myControlChange(channel, CCringmod, ringMod);
    Serial.println("RING_MOD_SW: " + String(ringMod));
  }

  vcoLFORetrigSwitch.update();
  if (vcoLFORetrigSwitch.risingEdge()) {
    vcoLfoRetrig = 1;
    myControlChange(channel, CCvcolforetrig, vcoLfoRetrig);
    Serial.println("PITCH_LFO_RETRIG_SW: " + String(vcoLfoRetrig));
  } else if (vcoLFORetrigSwitch.fallingEdge()) {
    vcoLfoRetrig = 0;
    myControlChange(channel, CCvcolforetrig, vcoLfoRetrig);
    Serial.println("PITCH_LFO_RETRIG_SW: " + String(vcoLfoRetrig));
  }

  vcfLFORetrigSwitch.update();
  if (vcfLFORetrigSwitch.risingEdge()) {
    vcfLfoRetrig = 1;
    myControlChange(channel, CCvcflforetrig, vcfLfoRetrig);
    Serial.println("VCF_LFO_RETRIG_SW: " + String(vcfLfoRetrig));
  } else if (vcfLFORetrigSwitch.fallingEdge()) {
    vcfLfoRetrig = 0;
    myControlChange(channel, CCvcflforetrig, vcfLfoRetrig);
    Serial.println("VCF_LFO_RETRIG_SW: " + String(vcfLfoRetrig));
  }

  tempoSwitch.update();
  if (tempoSwitch.risingEdge()) {
    vcfLFOMidiClkSync = 1;
    myControlChange(channel, CCvcfLFOMidiClkSync, vcfLFOMidiClkSync);
    Serial.println("TEMPO_SW: " + String(vcfLFOMidiClkSync));
  } else  if (tempoSwitch.fallingEdge()) {
    vcfLFOMidiClkSync = 0;
    myControlChange(channel, CCvcfLFOMidiClkSync, vcfLFOMidiClkSync);
    Serial.println("TEMPO_SW: " + String(vcfLFOMidiClkSync));
  }

  saveButton.update();
  if (saveButton.fallingEdge()) {
    Serial.println("SAVE_SW");
    
    savePatch(programNo);
  }

  recallButton.update();
  if (recallButton.read() == LOW && recallButton.duration() > INITIALISE_DURATION) {
    //If recall held for 1.5s, set current patch to match current hardware state
    Serial.println("Recall Held");
    //Reinitialise all hardware values to force them to be re-read if different
    reinitialiseHW();
    recallButton.write(HIGH);//Come out of this state
    reini = true;//Hack
  } else if (recallButton.risingEdge()) {
    if (!reini) {
      recallAvailablePatches();
      recallPatch(programNo);
    } else {
      reini = false;
    }
  }

  backButton.update();
  if (backButton.fallingEdge()) {
    Serial.println("BACK_SW");
  }

  enterButton.update();
  if (enterButton.fallingEdge()) {
    Serial.println("ENTER_SW");
  }
}

void checkDisplay() {
  if (timeOut()) {
    renderCurrentPatchPage(programNo, patchName);
  }
}

void reinitialiseHW() {
  //This reinialises the previous hardware values to force a re-read
  muxInput = 0;
  for (int i = 0; i < MUXCHANNELS; i++) {
    mux1ValuesPrev[i] = -99;
    mux2ValuesPrev[i] = -99;
  }
  fxAmtPrevious = -99;
  fxMixPrevious = -99;
  //Read switch state
  pwmSource = pwmSourceSwitch.read();
  ringMod = ringModSwitch.read();
  vcoLfoRetrig = vcoLFORetrigSwitch.read();
  vcfLfoRetrig = vcfLFORetrigSwitch.read();
  unison = unisonSwitch.read();
  vcfLFOMidiClkSync = tempoSwitch.read();
  renderCurrentParameterPage(programNo, INITPATCHNAME);
  Serial.println("Reinitialise HW");
}

void checkEncoder() {
  //Encoder works relatively inc and dec values
  //Detent encoder goes up in 3, hence +/-2
  long encRead = encoder.read();
  if (encRead > encPrevious + 2) {
    incrementEnc();
    encPrevious = encRead;
  } else if (encRead < encPrevious - 2) {
    decrementEnc();
    encPrevious = encRead;
  }
}

void loop() {
  myusb.Task();
  midi1.read();//USB HOST MIDI Class Compliant
  usbMIDI.read();//USB Client MIDI
  //MIDI.read();//MIDI 5 Pin DIN

  checkMux();
  checkFxPots();
  checkSwitches();
  checkEncoder();
  checkDisplay();
  //  Serial.print("CPU:");
  //  Serial.print(AudioProcessorUsageMax());
  //  Serial.print("  MEM:");
  //  Serial.println(AudioMemoryUsageMax());
}
