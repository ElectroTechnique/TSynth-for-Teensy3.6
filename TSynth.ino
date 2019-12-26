// Tools menu Settings:
// Board: "Teensy3.6"
// USB Type: "Serial + MIDI + Audio"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>
#include <SerialFlash.h>
#include <MIDI.h>
#include <USBHost_t36.h>
#include "MidiCC.h"
#include "AudioPatching.h"
#include "Constants.h"
#include "Parameters.h"
#define PARAMETER  0
#define RECALL  1
#define SAVE  2
#define REINITIALISE  3
#define PATCH  4
#define PATCHNAMING  5
#define DELETE  6

#define PULSE  1
#define VAR_TRI  2
#define FILTER_ENV  3
#define AMP_ENV  4

volatile unsigned int state = PARAMETER;
#include "PatchMgr.h"
#include "ST7735Display.h"
#include "HWControls.h"

//Teensy Audio Shield SD card
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
MIDI_CREATE_INSTANCE(HardwareSerial, Serial4, MIDI);//RX - Pin 31

struct VoiceAndNote {
  int note;
  long timeOn;
};

struct VoiceAndNote voices[NO_OF_VOICES] = {
  { -1, 0},
  { -1, 0},
  { -1, 0},
  { -1, 0},
  { -1, 0},
  { -1, 0}
};

int  prevNote = 48;//Initialised to middle value
float previousMillis = millis();//For MIDI Clk Sync

int count = 0;
byte midiChannel = MIDI_CHANNEL_OMNI;
int  patchNo = 1;
int voiceToReturn = -1;//Initialise to 'null'
long earliestTime = millis();//For voice allocation - initialise to now

void setup() {
  //ST7735 Display
  setupDisplay();
  setupHardware();

  AudioMemory(48);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);//Headphones
  sgtl5000_1.lineOutLevel(29);//Line out

  // Configure SPI
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  Sd2Card card;
  cardStatus = card.init(SPI_FULL_SPEED, SDCARD_CS_PIN);
  if (cardStatus) {
    Serial.println("SD card is connected");
    if (!SD.begin(SDCARD_CS_PIN)) {
      Serial.println("SD card failed!");
      showPatchPage("   SD card ", "    Failed");
    } else {
      //Get patch numbers and names from SD card
      getPatches(SD.open("/"));
      if (patches.size() == 0) {
        //save an initialialised patch to SD card
        savePatch("1", INITPATCH);
        getPatches(SD.open("/"));
      }
    }
  } else {
    Serial.println("SD card is not connected or unusable");
    reinitialiseToPanel();
    showPatchPage("SD card not", "conn'd or usable");
  }

  //Read MIDI Channel from EEPROM
//  midiChannel = EEPROM.read(0);
//  if (midiChannel == 0) midiChannel = MIDI_CHANNEL_OMNI;

  //USB HOST MIDI Class Compliant
  delay(200);//Wait to turn on USB Host
  myusb.begin();
  midi1.setHandleControlChange(myControlChange);
  midi1.setHandleNoteOff(myNoteOff);
  midi1.setHandleNoteOn(myNoteOn);
  midi1.setHandlePitchChange(myPitchBend);
  midi1.setHandleProgramChange(myProgramChange);
  midi1.setHandleClock(myMIDIClock);
  midi1.setHandleStart(myMIDIClockStart);
  Serial.println("USB HOST MIDI Class Compliant Listening");

  //USB Client MIDI
  usbMIDI.setHandleControlChange(myControlChange);
  usbMIDI.setHandleNoteOff(myNoteOff);
  usbMIDI.setHandleNoteOn(myNoteOn);
  usbMIDI.setHandlePitchChange(myPitchBend);
  usbMIDI.setHandleProgramChange(myProgramChange);
  usbMIDI.setHandleClock(myMIDIClock);
  usbMIDI.setHandleStart(myMIDIClockStart);
  Serial.println("USB Client MIDI Listening");

  //MIDI 5 Pin DIN
  MIDI.begin();
  MIDI.setHandleNoteOn(myNoteOn);
  MIDI.setHandleNoteOff(myNoteOff);
  MIDI.setHandlePitchBend(myPitchBend);
  MIDI.setHandleControlChange(myControlChange);
  MIDI.setHandleProgramChange(myProgramChange);
  //Doesn't like continuous Midi Clock signals from DAW, works with USB Midi fine
  //MIDI.setHandleClock(myMIDIClock);
  //MIDI.setHandleStart(myMIDIClockStart);
  Serial.println("MIDI In DIN Listening");

  constant1Dc.amplitude(CONSTANTONE);

  noiseMixer.gain(0, CONSTANTONE);
  noiseMixer.gain(1, CONSTANTONE);
  noiseMixer.gain(2, 0);
  noiseMixer.gain(3, 0);

  voiceMixer1.gain(0, VOICEMIXERLEVEL);
  voiceMixer1.gain(1, VOICEMIXERLEVEL);
  voiceMixer1.gain(2, VOICEMIXERLEVEL);
  voiceMixer1.gain(3, VOICEMIXERLEVEL);

  voiceMixer2.gain(0, VOICEMIXERLEVEL);
  voiceMixer2.gain(1, VOICEMIXERLEVEL);
  voiceMixer2.gain(2, 0);
  voiceMixer2.gain(3, 0);

  voiceMixerM.gain(0, VOICEMIXERLEVEL);
  voiceMixerM.gain(1, VOICEMIXERLEVEL);
  voiceMixerM.gain(2, 0);
  voiceMixerM.gain(3, 0);

  pwmLfo.amplitude(CONSTANTONE);
  pwmLfo.begin(PWMWAVEFORM);//

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

  waveformMod5a.amplitude(CONSTANTONE);
  waveformMod5a.frequency(440);
  waveformMod5a.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod5a.arbitraryWaveform(HARMONIC_WAVE, 172.0);
  waveformMod5a.begin(vcoWaveformA);
  waveformMod5b.amplitude(CONSTANTONE);
  waveformMod5b.frequency(440);
  waveformMod5b.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod5b.arbitraryWaveform(PARABOLIC_WAVE, 172.0);
  waveformMod5b.begin(vcoWaveformB);

  waveformMod6a.amplitude(CONSTANTONE);
  waveformMod6a.frequency(440);
  waveformMod6a.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod6a.arbitraryWaveform(HARMONIC_WAVE, 172.0);
  waveformMod6a.begin(vcoWaveformA);
  waveformMod6b.amplitude(CONSTANTONE);
  waveformMod6b.frequency(440);
  waveformMod6b.frequencyModulation(VCOLFOOCTAVERANGE);
  waveformMod6b.arbitraryWaveform(PARABOLIC_WAVE, 172.0);
  waveformMod6b.begin(vcoWaveformB);

  filter1.octaveControl(FILTEROCTAVERANGE);
  filter2.octaveControl(FILTEROCTAVERANGE);
  filter3.octaveControl(FILTEROCTAVERANGE);
  filter4.octaveControl(FILTEROCTAVERANGE);
  filter5.octaveControl(FILTEROCTAVERANGE);
  filter6.octaveControl(FILTEROCTAVERANGE);

  waveformMixer1.gain(2, CONSTANTONE);//Noise
  waveformMixer1.gain(3, 0);//Ring Mod
  waveformMixer2.gain(2, CONSTANTONE);//Noise
  waveformMixer2.gain(3, 0);//Ring Mod
  waveformMixer3.gain(2, CONSTANTONE);//Noise
  waveformMixer3.gain(3, 0);//Ring Mod
  waveformMixer4.gain(2, CONSTANTONE);//Noise
  waveformMixer4.gain(3, 0);//Ring Mod
  waveformMixer5.gain(2, CONSTANTONE);//Noise
  waveformMixer5.gain(3, 0);//Ring Mod
  waveformMixer6.gain(2, CONSTANTONE);//Noise
  waveformMixer6.gain(3, 0);//Ring Mod

  vcfModMixer1.gain(1, CONSTANTONE);//LFO
  vcfModMixer1.gain(3, 0);//Not used
  vcfModMixer2.gain(1, CONSTANTONE);//LFO
  vcfModMixer2.gain(3, 0);//Not used
  vcfModMixer3.gain(1, CONSTANTONE);//LFO
  vcfModMixer3.gain(3, 0);//Not used
  vcfModMixer4.gain(1, CONSTANTONE);//LFO
  vcfModMixer4.gain(3, 0); //Not used
  vcfModMixer5.gain(1, CONSTANTONE);//LFO
  vcfModMixer5.gain(3, 0);//Not used
  vcfModMixer6.gain(1, CONSTANTONE);//LFO
  vcfModMixer6.gain(3, 0); //Not used

  filterMixer1.gain(3, 0);//Not used
  filterMixer2.gain(3, 0);//Not used
  filterMixer3.gain(3, 0);//Not used
  filterMixer4.gain(3, 0);//Not used
  filterMixer5.gain(3, 0);//Not used
  filterMixer6.gain(3, 0);//Not used

  ensemble.lfoRate(fxAmt);

  effectMixerL.gain(2, 0);
  effectMixerL.gain(3, 0);
  effectMixerR.gain(2, 0);
  effectMixerR.gain(3, 0);

  //reinitialiseToPanel();
  recallPatch(String(patchNo));
}

void myNoteOn(byte channel, byte note, byte velocity) {
  if (midiChannel != MIDI_CHANNEL_OMNI && midiChannel != channel) return;
  //Check for out of range notes
  if (note + vcoOctaveA < 0 || note + vcoOctaveA > 127 || note + vcoOctaveB < 0 || note + vcoOctaveB > 127) return;

  AudioNoInterrupts();
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
        keytracking1.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
        voices[0].note = note;
        voices[0].timeOn = millis();
        updateVoice1();
        vcfEnvelope1.noteOn();
        vcaEnvelope1.noteOn();
        voiceOn[0] = true;
        break;
      case 2:
        //Serial.println("ON 2");
        keytracking2.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
        voices[1].note = note;
        voices[1].timeOn = millis();
        updateVoice2();
        vcfEnvelope2.noteOn();
        vcaEnvelope2.noteOn();
        voiceOn[1] = true;
        break;
      case 3:
        //Serial.println("ON 3");
        keytracking3.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
        voices[2].note = note;
        voices[2].timeOn = millis();
        updateVoice3();
        vcfEnvelope3.noteOn();
        vcaEnvelope3.noteOn();
        voiceOn[2] = true;
        break;
      case 4:
        //Serial.println("ON 4");
        keytracking4.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
        voices[3].note = note;
        voices[3].timeOn = millis();
        updateVoice4();
        vcfEnvelope4.noteOn();
        vcaEnvelope4.noteOn();
        voiceOn[3] = true;
        break;
      case 5:
        //Serial.println("ON 5");
        keytracking5.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
        voices[4].note = note;
        voices[4].timeOn = millis();
        updateVoice5();
        vcfEnvelope5.noteOn();
        vcaEnvelope5.noteOn();
        voiceOn[4] = true;
        break;
      case 6:
        //Serial.println("ON 6");
        keytracking6.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
        voices[5].note = note;
        voices[5].timeOn = millis();
        updateVoice6();
        vcfEnvelope6.noteOn();
        vcaEnvelope6.noteOn();
        voiceOn[5] = true;
        break;
    }
  } else {
    //UNISON MODE
    keytracking1.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
    keytracking2.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
    keytracking3.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
    keytracking4.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
    keytracking5.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
    keytracking6.amplitude(note * DIV127 * keytrackingAmount * KEYTRACKINGFACTOR);
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
    voices[4].note = note;
    voices[4].timeOn = millis();
    updateVoice5();
    voices[5].note = note;
    voices[5].timeOn = millis();
    updateVoice6();

    vcfEnvelope1.noteOn();
    vcfEnvelope2.noteOn();
    vcfEnvelope3.noteOn();
    vcfEnvelope4.noteOn();
    vcfEnvelope5.noteOn();
    vcfEnvelope6.noteOn();

    vcaEnvelope1.noteOn();
    vcaEnvelope2.noteOn();
    vcaEnvelope3.noteOn();
    vcaEnvelope4.noteOn();
    vcaEnvelope5.noteOn();
    vcaEnvelope6.noteOn();

    voiceOn[0] = true;
    voiceOn[1] = true;
    voiceOn[2] = true;
    voiceOn[3] = true;
    voiceOn[4] = true;
    voiceOn[5] = true;
  }
  AudioInterrupts();
}

void myNoteOff(byte channel, byte note, byte velocity) {
  if (midiChannel != MIDI_CHANNEL_OMNI && midiChannel != channel) return;
  AudioNoInterrupts();
  if (unison == 0) {
    switch (getVoiceNo(note)) {
      case 1:
        vcfEnvelope1.noteOff();
        vcaEnvelope1.noteOff();
        voices[0].note = -1;
        voiceOn[0] = false;
        break;
      case 2:
        vcfEnvelope2.noteOff();
        vcaEnvelope2.noteOff();
        voices[1].note = -1;
        voiceOn[1] = false;
        break;
      case 3:
        vcfEnvelope3.noteOff();
        vcaEnvelope3.noteOff();
        voices[2].note = -1;
        voiceOn[2] = false;
        break;
      case 4:
        vcfEnvelope4.noteOff();
        vcaEnvelope4.noteOff();
        voices[3].note = -1;
        voiceOn[3] = false;
        break;
      case 5:
        vcfEnvelope5.noteOff();
        vcaEnvelope5.noteOff();
        voices[4].note = -1;
        voiceOn[4] = false;
        break;
      case 6:
        vcfEnvelope6.noteOff();
        vcaEnvelope6.noteOff();
        voices[5].note = -1;
        voiceOn[5] = false;
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
  vcfEnvelope5.noteOff();
  vcaEnvelope5.noteOff();
  vcfEnvelope6.noteOff();
  vcaEnvelope6.noteOff();

  voices[0].note = -1;
  voices[1].note = -1;
  voices[2].note = -1;
  voices[3].note = -1;
  voices[4].note = -1;
  voices[5].note = -1;

  voiceOn[0] = false;
  voiceOn[1] = false;
  voiceOn[2] = false;
  voiceOn[3] = false;
  voiceOn[4] = false;
  voiceOn[5] = false;
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
    waveformMod1b.frequency(NOTEFREQS[voices[0].note + vcoOctaveB] * (detune + ((1 - detune) * 0.09)));
  } else {
    waveformMod1b.frequency(NOTEFREQS[voices[0].note + vcoOctaveB] * detune);
  }
}

void updateVoice2() {
  if (unison == 1) {
    waveformMod2a.frequency(NOTEFREQS[voices[1].note + vcoOctaveA] * (detune + ((1 - detune) * 0.18)));
    waveformMod2b.frequency(NOTEFREQS[voices[1].note + vcoOctaveB] * (detune + ((1 - detune) * 0.27)));
  } else {
    waveformMod2a.frequency(NOTEFREQS[voices[1].note + vcoOctaveA]);
    waveformMod2b.frequency(NOTEFREQS[voices[1].note + vcoOctaveB] * detune);
  }
}

void updateVoice3() {
  if (unison == 1) {
    waveformMod3a.frequency(NOTEFREQS[voices[2].note + vcoOctaveA] * (detune + ((1 - detune) * 0.36)));
    waveformMod3b.frequency(NOTEFREQS[voices[2].note + vcoOctaveB] * (detune + ((1 - detune) * 0.46)));
  } else {
    waveformMod3a.frequency(NOTEFREQS[voices[2].note + vcoOctaveA]);
    waveformMod3b.frequency(NOTEFREQS[voices[2].note + vcoOctaveB] * detune);
  }
}void updateVoice4() {
  if (unison == 1) {
    waveformMod4a.frequency(NOTEFREQS[voices[3].note + vcoOctaveA] * (detune + ((1 - detune) * 0.55)));
    waveformMod4b.frequency(NOTEFREQS[voices[3].note + vcoOctaveB] * (detune + ((1 - detune) * 0.64)));
  } else {
    waveformMod4a.frequency(NOTEFREQS[voices[3].note + vcoOctaveA]);
    waveformMod4b.frequency(NOTEFREQS[voices[3].note + vcoOctaveB] * detune);
  }
}

void updateVoice5() {
  if (unison == 1) {
    waveformMod5a.frequency(NOTEFREQS[voices[4].note + vcoOctaveA] * (detune + ((1 - detune) * 0.73)));
    waveformMod5b.frequency(NOTEFREQS[voices[4].note + vcoOctaveB] * (detune + ((1 - detune) * 0.82)));
  } else {
    waveformMod5a.frequency(NOTEFREQS[voices[4].note + vcoOctaveA]);
    waveformMod5b.frequency(NOTEFREQS[voices[4].note + vcoOctaveB] * detune);
  }
}

void updateVoice6() {
  if (unison == 1) {
    waveformMod6a.frequency(NOTEFREQS[voices[5].note + vcoOctaveA] * (detune + ((1 - detune) * 0.90)));
  } else {
    waveformMod6a.frequency(NOTEFREQS[voices[5].note + vcoOctaveA]);
  }
  waveformMod6b.frequency(NOTEFREQS[voices[5].note + vcoOctaveB] * detune);
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

String getWaveformStr(int value) {
  switch (value) {
    case WAVEFORM_SAMPLE_HOLD:
      return "Sample & Hold";
    case WAVEFORM_SINE:
      return "Sine";
    case WAVEFORM_SQUARE:
      return "Square";
    case WAVEFORM_TRIANGLE:
      return "Triangle";
    case WAVEFORM_SAWTOOTH:
      return "Sawtooth";
    case WAVEFORM_SAWTOOTH_REVERSE:
      return "Ramp";
    case WAVEFORM_PULSE:
      return "Var. Pulse";
    case WAVEFORM_TRIANGLE_VARIABLE:
      return "Var. Triangle";
    case WAVEFORM_ARBITRARY:
      return "User";
    default:
      return "ERR_WAVE";
  }
}

float getLFOTempoRate(int value) {
  lfoTempoValue = LFOTEMPO[value];
  return lfoSyncFreq * LFOTEMPO[value];
}

int getVCOWaveformA(int value, int currentValue) {
  //Overlapping zones in between values to avoid instability
  if (value >= 0 && value < 20) {
    return WAVEFORM_SINE;
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
int getVCOWaveformB(int value, int currentValue) {
  //Overlapping zones in between values to avoid instability
  if (value >= 0 && value < 20) {
    return WAVEFORM_SAMPLE_HOLD;
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

int getVCOOctave(int value, int currentValue) {
  //Overlapping zones in between values to avoid instability
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
  pwMixer5a.gain(0, value);
  pwMixer6a.gain(0, value);
  showCurrentParameterPage("1. PWM LFO", String(value));
}
void setPwmMixerBLFO(float value) {
  pwMixer1b.gain(0, value);
  pwMixer2b.gain(0, value);
  pwMixer3b.gain(0, value);
  pwMixer4b.gain(0, value);
  pwMixer5b.gain(0, value);
  pwMixer6b.gain(0, value);
  showCurrentParameterPage("2. PWM LFO", String(value));
}

void setPwmMixerAPW(float value) {
  pwMixer1a.gain(1, value);
  pwMixer2a.gain(1, value);
  pwMixer3a.gain(1, value);
  pwMixer4a.gain(1, value);
  pwMixer5a.gain(1, value);
  pwMixer6a.gain(1, value);
}

void setPwmMixerBPW(float value) {
  pwMixer1b.gain(1, value);
  pwMixer2b.gain(1, value);
  pwMixer3b.gain(1, value);
  pwMixer4b.gain(1, value);
  pwMixer5b.gain(1, value);
  pwMixer6b.gain(1, value);
}

void setPwmMixerAFEnv(float value) {
  pwMixer1a.gain(2, value);
  pwMixer2a.gain(2, value);
  pwMixer3a.gain(2, value);
  pwMixer4a.gain(2, value);
  pwMixer5a.gain(2, value);
  pwMixer6a.gain(2, value);
  showCurrentParameterPage("1. PWM F Env", String(value));
}

void setPwmMixerBFEnv(float value) {
  pwMixer1b.gain(2, value);
  pwMixer2b.gain(2, value);
  pwMixer3b.gain(2, value);
  pwMixer4b.gain(2, value);
  pwMixer5b.gain(2, value);
  pwMixer6b.gain(2, value);
  showCurrentParameterPage("2. PWM F Env", String(value));
}

void updateUnison() {
  if (unison == 0) {
    allNotesOff();
    showCurrentParameterPage("Unison", "Off");
  } else {
    showCurrentParameterPage("Unison", "On");
  }
}

void updateVolume(float vol) {
  showCurrentParameterPage("Volume", vol);
}

void updateGlide() {
  showCurrentParameterPage("Glide", String(int(glideSpeed * GLIDEFACTOR)) + " ms");
}

void updateWaveformA() {
  waveformMod1a.begin(vcoWaveformA);
  waveformMod2a.begin(vcoWaveformA);
  waveformMod3a.begin(vcoWaveformA);
  waveformMod4a.begin(vcoWaveformA);
  waveformMod5a.begin(vcoWaveformA);
  waveformMod6a.begin(vcoWaveformA);
  showCurrentParameterPage("1. Waveform", getWaveformStr(vcoWaveformA));
}

void updateWaveformB() {
  waveformMod1b.begin(vcoWaveformB);
  waveformMod2b.begin(vcoWaveformB);
  waveformMod3b.begin(vcoWaveformB);
  waveformMod4b.begin(vcoWaveformB);
  waveformMod5b.begin(vcoWaveformB);
  waveformMod6b.begin(vcoWaveformB);
  showCurrentParameterPage("2. Waveform", getWaveformStr(vcoWaveformB));
}

void updateOctaveA() {
  //update waveforms with new frequencies if notes are on
  if (voices[0].note != -1 ||  voices[1].note != -1 || voices[2].note != -1 || voices[3].note != -1 || voices[4].note != -1 || voices[5].note != -1) {
    updateVoice1();
    updateVoice2();
    updateVoice3();
    updateVoice4();
    updateVoice5();
    updateVoice6();
  }
  showCurrentParameterPage("1. Octave", (vcoOctaveA > 0 ? "+" : "") + String(vcoOctaveA));
}

void updateOctaveB() {
  //update waveforms with new frequencies if notes are on
  if (voices[0].note != -1 ||  voices[1].note != -1 || voices[2].note != -1 || voices[3].note != -1 || voices[4].note != -1 || voices[5].note != -1) {
    updateVoice1();
    updateVoice3();
    updateVoice2();
    updateVoice4();
    updateVoice5();
    updateVoice6();
  }
  showCurrentParameterPage("2. Octave", (vcoOctaveB > 0 ? "+" : "") + String(vcoOctaveB));
}

void updateDetune() {
  //update waveforms with new frequencies if notes are on
  if (voices[0].note != -1 ||  voices[1].note != -1 || voices[2].note != -1 || voices[3].note != -1 || voices[4].note != -1 || voices[5].note != -1) {
    updateVoice1();
    updateVoice2();
    updateVoice3();
    updateVoice4();
    updateVoice5();
    updateVoice6();
  }
  showCurrentParameterPage("Detune", String((1 - detune) * 100) + " %");
}

void updatePWMSource() {
  if (pwmSource == PWMSOURCELFO) {
    setPwmMixerAFEnv(0);
    setPwmMixerBFEnv(0);
    if (pwmRate >= 0.01) {
      setPwmMixerALFO(pwmAmtA);
      setPwmMixerBLFO(pwmAmtB);
    }
    showCurrentParameterPage("PWM Source", "LFO");
  } else {
    setPwmMixerALFO(0);
    setPwmMixerBLFO(0);
    if (pwmRate >= 0.01) {
      setPwmMixerAFEnv(pwmAmtA);
      setPwmMixerBFEnv(pwmAmtB);
    }
    showCurrentParameterPage("PWM Source", "Filter Env");
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
    showCurrentParameterPage("PW Mode", "On");
  } else {
    setPwmMixerAPW(0);
    setPwmMixerBPW(0);
    if (pwmSource == PWMSOURCELFO) {
      setPwmMixerAFEnv(0);
      setPwmMixerBFEnv(0);
      setPwmMixerALFO(pwmAmtA);
      setPwmMixerBLFO(pwmAmtB);
      showCurrentParameterPage("PWM Rate", String(pwmRate) + " Hz");
    } else {
      //Filter env mod - pwmRate does nothing
      setPwmMixerALFO(0);
      setPwmMixerBLFO(0);
      setPwmMixerAFEnv(pwmAmtA);
      setPwmMixerBFEnv(pwmAmtB);
      showCurrentParameterPage("PWM Source", "Filter Env");
    }
  }
}

void updatePWMAmount() {
  //MIDI only
  pwA = 0;
  pwB = 0;
  setPwmMixerALFO(pwmAmtA);
  setPwmMixerBLFO(pwmAmtB);
  showCurrentParameterPage("PWM Amt", String(pwmAmtA) + " " + String(pwmAmtB));
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
    if (vcoWaveformA == WAVEFORM_TRIANGLE_VARIABLE) {
      showCurrentParameterPage("1. PW Amt", pwA, VAR_TRI);
    } else {
      showCurrentParameterPage("1. PW Amt", pwA, PULSE);
    }
  } else {
    setPwmMixerAPW(0);
    setPwmMixerBPW(0);
    if (pwmSource == PWMSOURCELFO) {
      //PW alters PWM LFO amount for waveform A
      setPwmMixerALFO(pwmAmtA);
      showCurrentParameterPage("1. PWM Amt", "LFO " + String(pwmAmtA));
    } else {
      //PW alters PWM FEnv amount for waveform A
      setPwmMixerAFEnv(pwmAmtA);
      showCurrentParameterPage("1. PWM Amt", "F. Env " + String(pwmAmtA));
    }
  }
  pwa.amplitude(pwA);
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
    if (vcoWaveformB == WAVEFORM_TRIANGLE_VARIABLE) {
      showCurrentParameterPage("2. PW Amt", pwB, VAR_TRI);
    } else {
      showCurrentParameterPage("2. PW Amt", pwB, PULSE);
    }
  } else {
    setPwmMixerAPW(0);
    setPwmMixerBPW(0);
    if (pwmSource == PWMSOURCELFO) {
      //PW alters PWM LFO amount for waveform B
      setPwmMixerBLFO(pwmAmtB);
      showCurrentParameterPage("2. PWM Amt", "LFO " + String(pwmAmtB));
    } else {
      //PW alters PWM FEnv amount for waveform B
      setPwmMixerBFEnv(pwmAmtB);
      showCurrentParameterPage("2. PWM Amt", "F. Env " + String(pwmAmtB));
    }
  }
  pwb.amplitude(pwB);
}

void updateVcoLevelA() {
  waveformMixer1.gain(0, VCOALevel);
  waveformMixer2.gain(0, VCOALevel);
  waveformMixer3.gain(0, VCOALevel);
  waveformMixer4.gain(0, VCOALevel);
  waveformMixer5.gain(0, VCOALevel);
  waveformMixer6.gain(0, VCOALevel);
  if (ringMod == 1) {
    waveformMixer1.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
    waveformMixer2.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
    waveformMixer3.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
    waveformMixer4.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
    waveformMixer5.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
    waveformMixer6.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
  }
  showCurrentParameterPage("1. Level", VCOALevel);
}


void updateVcoLevelB() {
  waveformMixer1.gain(1, VCOBLevel);
  waveformMixer2.gain(1, VCOBLevel);
  waveformMixer3.gain(1, VCOBLevel);
  waveformMixer4.gain(1, VCOBLevel);
  waveformMixer5.gain(1, VCOBLevel);
  waveformMixer6.gain(1, VCOBLevel);
  if (ringMod == 1) {
    waveformMixer1.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
    waveformMixer2.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
    waveformMixer3.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
    waveformMixer4.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
    waveformMixer5.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
    waveformMixer6.gain(3, (VCOALevel + VCOBLevel) / 2.0f);//Ring Mod
  }
  showCurrentParameterPage("2. Level", VCOBLevel);
}

void updateNoiseLevel() {
  if (noiseLevel > 0) {
    pink.amplitude(noiseLevel);
    white.amplitude(0.0f);
    showCurrentParameterPage("Noise Level", "Pink " + String(noiseLevel));
  } else if (noiseLevel < 0) {
    pink.amplitude(0.0f);
    white.amplitude(abs(noiseLevel));
    showCurrentParameterPage("Noise Level", "White " + String(abs(noiseLevel)));
  } else {
    pink.amplitude(noiseLevel);
    white.amplitude(noiseLevel);
    showCurrentParameterPage("Noise Level", "Off");
  }

}

void updateFilterFreq() {
  filter1.frequency(filterFreq);
  filter2.frequency(filterFreq);
  filter3.frequency(filterFreq);
  filter4.frequency(filterFreq);
  filter5.frequency(filterFreq);
  filter6.frequency(filterFreq);
  showCurrentParameterPage("Cutoff", String(int(filterFreq)) + " Hz");
}


void updateFilterRes() {
  filter1.resonance(filterRes);
  filter2.resonance(filterRes);
  filter3.resonance(filterRes);
  filter4.resonance(filterRes);
  filter5.resonance(filterRes);
  filter6.resonance(filterRes);
  showCurrentParameterPage("Resonance", filterRes);
}

void updateFilterMixer() {
  float LP = 1.0f;
  float BP = 0;
  float HP = 0;
  String filterStr;
  if (filterMix == LINEAR_FILTERMIXER[127]) {
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
  filterMixer5.gain(0, LP);
  filterMixer5.gain(1, BP);
  filterMixer5.gain(2, HP);
  filterMixer6.gain(0, LP);
  filterMixer6.gain(1, BP);
  filterMixer6.gain(2, HP);
  showCurrentParameterPage("Filter Type", filterStr);
}

void updateFilterEnv() {
  vcfModMixer1.gain(0, filterEnv);
  vcfModMixer2.gain(0, filterEnv);
  vcfModMixer3.gain(0, filterEnv);
  vcfModMixer4.gain(0, filterEnv);
  vcfModMixer5.gain(0, filterEnv);
  vcfModMixer6.gain(0, filterEnv);
  showCurrentParameterPage("Filter Env.", String(filterEnv));
}

void updatePitchEnv() {
  vcoModMixer1a.gain(1, pitchEnv);
  vcoModMixer1b.gain(1, pitchEnv);
  vcoModMixer2a.gain(1, pitchEnv);
  vcoModMixer2b.gain(1, pitchEnv);
  vcoModMixer3a.gain(1, pitchEnv);
  vcoModMixer3b.gain(1, pitchEnv);
  vcoModMixer4a.gain(1, pitchEnv);
  vcoModMixer4b.gain(1, pitchEnv);
  vcoModMixer5a.gain(1, pitchEnv);
  vcoModMixer5b.gain(1, pitchEnv);
  vcoModMixer6a.gain(1, pitchEnv);
  vcoModMixer6b.gain(1, pitchEnv);
  showCurrentParameterPage("Pitch Env Amt", String(pitchEnv));
}

void updateKeyTracking() {
  vcfModMixer1.gain(2, keytrackingAmount);
  vcfModMixer2.gain(2, keytrackingAmount);
  vcfModMixer3.gain(2, keytrackingAmount);
  vcfModMixer4.gain(2, keytrackingAmount);
  vcfModMixer5.gain(2, keytrackingAmount);
  vcfModMixer6.gain(2, keytrackingAmount);
  showCurrentParameterPage("Key Tracking", String(keytrackingAmount));
}

void updateVcoLFOAmt() {
  vcoLfo.amplitude(vcoLfoAmt);
  showCurrentParameterPage("LFO Amount", String(vcoLfoAmt));
}

void updateModWheel() {
  vcoLfo.amplitude(vcoLfoAmt);
}

void updateVcoLFORate() {
  vcoLfo.frequency(vcoLfoRate);
  showCurrentParameterPage("LFO Rate", String(vcoLfoRate) + " Hz");
}

void updateVcoLFOWaveform() {
  vcoLfo.begin(vcoLFOWaveform);
  showCurrentParameterPage("Pitch LFO", getWaveformStr(vcoLFOWaveform));
}

//MIDI CC only
void updateVcoLFOMidiClkSync() {
  showCurrentParameterPage("P. LFO Sync", vcoLFOMidiClkSync == 1 ? "On" : "Off");
}

void updateVcfLfoRate() {
  vcfLfo.frequency(vcfLfoRate);
  if (vcfLFOMidiClkSync) {
    showCurrentParameterPage("LFO Time Div", vcfLFOTimeDivStr);
  } else {
    showCurrentParameterPage("F. LFO Rate", String(vcfLfoRate) + " Hz");
  }
}

void updateVcfLfoAmt() {
  vcfLfo.amplitude(vcfLfoAmt);
  showCurrentParameterPage("F. LFO Amt", String(vcfLfoAmt));
}

void updateVcfLFOWaveform() {
  vcfLfo.begin(vcfLfoWaveform);
  showCurrentParameterPage("Filter LFO", getWaveformStr(vcfLfoWaveform));
}

void updateVcoLFORetrig() {
  showCurrentParameterPage("P. LFO Retrig", vcoLfoRetrig == 1 ? "On" : "Off");
}

void updateVcfLFORetrig() {
  showCurrentParameterPage("F. LFO Retrig", vcfLfoRetrig == 1 ? "On" : "Off");
}

void updateVcfLFOMidiClkSync() {
  showCurrentParameterPage("Tempo Sync", vcfLFOMidiClkSync == 1 ? "On" : "Off");
}

void updateVcfAttack() {
  vcfEnvelope1.attack(vcfAttack);
  vcfEnvelope2.attack(vcfAttack);
  vcfEnvelope3.attack(vcfAttack);
  vcfEnvelope4.attack(vcfAttack);
  vcfEnvelope5.attack(vcfAttack);
  vcfEnvelope6.attack(vcfAttack);
  if (vcfAttack < 1000) {
    showCurrentParameterPage("Filter Attack", String(int(vcfAttack)) + " ms", FILTER_ENV);
  } else {
    showCurrentParameterPage("Filter Attack", String(vcfAttack * 0.001) + " s", FILTER_ENV);
  }
}

void updateVcfDecay() {
  vcfEnvelope1.decay(vcfDecay);
  vcfEnvelope2.decay(vcfDecay);
  vcfEnvelope3.decay(vcfDecay);
  vcfEnvelope4.decay(vcfDecay);
  vcfEnvelope5.decay(vcfDecay);
  vcfEnvelope6.decay(vcfDecay);
  if (vcfDecay < 1000) {
    showCurrentParameterPage("Filter Decay", String(int(vcfDecay)) + " ms", FILTER_ENV);
  } else {
    showCurrentParameterPage("Filter Decay", String(vcfDecay * 0.001) + " s", FILTER_ENV);
  }
}

void updateVcfSustain() {
  vcfEnvelope1.sustain(vcfSustain);
  vcfEnvelope2.sustain(vcfSustain);
  vcfEnvelope3.sustain(vcfSustain);
  vcfEnvelope4.sustain(vcfSustain);
  vcfEnvelope5.sustain(vcfSustain);
  vcfEnvelope6.sustain(vcfSustain);
  showCurrentParameterPage("Filter Sustain", String(vcfSustain), FILTER_ENV);
}

void updateVcfRelease() {
  vcfEnvelope1.release(vcfRelease);
  vcfEnvelope2.release(vcfRelease);
  vcfEnvelope3.release(vcfRelease);
  vcfEnvelope4.release(vcfRelease);
  vcfEnvelope5.release(vcfRelease);
  vcfEnvelope6.release(vcfRelease);
  if (vcfRelease < 1000) {
    showCurrentParameterPage("Filter Release", String(int(vcfRelease)) + " ms", FILTER_ENV);
  } else {
    showCurrentParameterPage("Filter Release", String(vcfRelease * 0.001) + " s", FILTER_ENV);
  }
}

void updateVcaAttack() {
  vcaEnvelope1.attack(vcaAttack);
  vcaEnvelope2.attack(vcaAttack);
  vcaEnvelope3.attack(vcaAttack);
  vcaEnvelope4.attack(vcaAttack);
  vcaEnvelope5.attack(vcaAttack);
  vcaEnvelope6.attack(vcaAttack);
  if (vcaAttack < 1000) {
    showCurrentParameterPage("Attack", String(int(vcaAttack)) + " ms", AMP_ENV);
  } else {
    showCurrentParameterPage("Attack", String(vcaAttack * 0.001) + " s", AMP_ENV);
  }
}

void updateVcaDecay() {
  vcaEnvelope1.decay(vcaDecay);
  vcaEnvelope2.decay(vcaDecay);
  vcaEnvelope3.decay(vcaDecay);
  vcaEnvelope4.decay(vcaDecay);
  vcaEnvelope5.decay(vcaDecay);
  vcaEnvelope6.decay(vcaDecay);
  if (vcaDecay < 1000) {
    showCurrentParameterPage("Decay", String(int(vcaDecay)) + " ms", AMP_ENV);
  } else {
    showCurrentParameterPage("Decay", String(vcaDecay * 0.001) + " s", AMP_ENV);
  }
}

void updateVcaSustain() {
  vcaEnvelope1.sustain(vcaSustain);
  vcaEnvelope2.sustain(vcaSustain);
  vcaEnvelope3.sustain(vcaSustain);
  vcaEnvelope4.sustain(vcaSustain);
  vcaEnvelope5.sustain(vcaSustain);
  vcaEnvelope6.sustain(vcaSustain);
  showCurrentParameterPage("Sustain", String(vcaSustain), AMP_ENV);
}

void updateVcaRelease() {
  vcaEnvelope1.release(vcaRelease);
  vcaEnvelope2.release(vcaRelease);
  vcaEnvelope3.release(vcaRelease);
  vcaEnvelope4.release(vcaRelease);
  vcaEnvelope5.release(vcaRelease);
  vcaEnvelope6.release(vcaRelease);
  if (vcaRelease < 1000) {
    showCurrentParameterPage("Release", String(int(vcaRelease)) + " ms", AMP_ENV);
  } else {
    showCurrentParameterPage("Release", String(vcaRelease * 0.001) + " s", AMP_ENV);
  }
}

void updateRingMod() {
  if (ringMod == 1) {
    ringMod1.setCombineMode(AudioEffectDigitalCombine::XOR);
    ringMod2.setCombineMode(AudioEffectDigitalCombine::XOR);
    ringMod3.setCombineMode(AudioEffectDigitalCombine::XOR);
    ringMod4.setCombineMode(AudioEffectDigitalCombine::XOR);
    ringMod5.setCombineMode(AudioEffectDigitalCombine::XOR);
    ringMod6.setCombineMode(AudioEffectDigitalCombine::XOR);
    waveformMixer1.gain(3, (VCOALevel + VCOBLevel) / 2.0);//Ring Mod
    waveformMixer2.gain(3, (VCOALevel + VCOBLevel) / 2.0);//Ring Mod
    waveformMixer3.gain(3, (VCOALevel + VCOBLevel) / 2.0);//Ring Mod
    waveformMixer4.gain(3, (VCOALevel + VCOBLevel) / 2.0);//Ring Mod
    waveformMixer5.gain(3, (VCOALevel + VCOBLevel) / 2.0);//Ring Mod
    waveformMixer6.gain(3, (VCOALevel + VCOBLevel) / 2.0);//Ring Mod
    showCurrentParameterPage("Ring Mod", "On");
  } else {
    ringMod1.setCombineMode(AudioEffectDigitalCombine::OFF);
    ringMod2.setCombineMode(AudioEffectDigitalCombine::OFF);
    ringMod3.setCombineMode(AudioEffectDigitalCombine::OFF);
    ringMod4.setCombineMode(AudioEffectDigitalCombine::OFF);
    ringMod5.setCombineMode(AudioEffectDigitalCombine::OFF);
    ringMod6.setCombineMode(AudioEffectDigitalCombine::OFF);
    waveformMixer1.gain(3, 0);//Ring Mod
    waveformMixer2.gain(3, 0);//Ring Mod
    waveformMixer3.gain(3, 0);//Ring Mod
    waveformMixer4.gain(3, 0);//Ring Mod
    waveformMixer5.gain(3, 0);//Ring Mod
    waveformMixer6.gain(3, 0);//Ring Mod
    showCurrentParameterPage("Ring Mod", "Off");
  }
}

void updateFXAmt() {
  ensemble.lfoRate(fxAmt);
  showCurrentParameterPage("Effect Amt", String(fxAmt) + " Hz");
}

void updateFXMix() {
  effectMixerL.gain(0, 1.0 - fxMix); //Dry
  effectMixerL.gain(1, fxMix);//Wet
  effectMixerR.gain(0, 1.0 - fxMix); //Dry
  effectMixerR.gain(1, fxMix);//Wet
  showCurrentParameterPage("Effect Mix", String(fxMix));
}

void updatePatchname() {
  showPatchPage(String(patchNo), patchName);
}

void myPitchBend(byte channel, int bend) {
  if (midiChannel != MIDI_CHANNEL_OMNI && midiChannel != channel) return;
  pitchBend.amplitude(bend * 0.5 * DIV8192);//)0.5 to give 1oct - spread of mod is 2oct
}

void myControlChange(byte channel, byte control, byte value) {
  if (midiChannel != MIDI_CHANNEL_OMNI && midiChannel != channel) return;
  AudioNoInterrupts();
  switch (control) {
    case CCvolume:
      sgtl5000_1.volume(0.8 * LINEAR[value]);//Headphones
      //sgtl5000_1.lineOutLevel(31 - (18 * LINEAR[value])); //Line out
      updateVolume(LINEAR[value]);
      break;

    case CCunison:
      value > 0 ? unison = 1 : unison = 0;
      updateUnison();
      break;

    case CCglide:
      glideSpeed = LINEAR[value];
      updateGlide();
      break;

    case CCpitchenv:
      //The pitch envelope amount has a large 'zero zone' in the centre
      //to make it easier to ensure that it is off when adjusted.
      pitchEnv = LINEARCENTREZERO[value] * VCOMODMIXERMAX;
      updatePitchEnv();
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
      detune = 1 -  (MAXDETUNE * LINEAR[value]);
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
      //NO FRONT PANEL CONTROL - MIDI CC ONLY
      //Total PWM amount for both oscillators
      pwmAmtA =  LINEAR[value];
      pwmAmtB =  LINEAR[value];
      updatePWMAmount();
      break;

    case CCpwA:
      pwA = (2.0 * LINEAR[value]) - 1.0;//Bipolar
      pwmAmtA =   LINEAR[value];
      updatePWA();
      break;

    case CCpwB:
      pwB =  (2.0 * LINEAR[value]) - 1.0;//Bipolar
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
      noiseLevel = LINEARCENTREZERO[value];
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
      vcoLfoAmt = POWER[value] * MODWHEELFACTOR;//Less LFO amount from mod wheel
      updateModWheel();
      break;

    case CCvcolfoamt:
      vcoLfoAmt = POWER[value];
      updateVcoLFOAmt();
      break;

    case CCvcoLfoRate:
      if (vcoLFOMidiClkSync == 1) {
        vcoLfoRate = getLFOTempoRate(value);
        vcoLFOTimeDivStr = LFOTEMPOSTR[value];
      } else {
        vcoLfoRate = LFOMAXRATE * POWER[value];
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
        vcfLFOTimeDivStr = LFOTEMPOSTR[value];
      } else {
        vcfLfoRate = LFOMAXRATE * POWER[value];
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
      updateVcfLFORetrig();
      break;

    //MIDI Only
    case CCvcoLFOMidiClkSync:
      value > 0 ? vcoLFOMidiClkSync = 1 : vcoLFOMidiClkSync = 0;
      updateVcoLFOMidiClkSync();
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
      fxAmt = ENSEMBLE_LFO[value];
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
  if (midiChannel != MIDI_CHANNEL_OMNI && midiChannel != channel) return;
  state = PATCH;
  Serial.print("MIDI Pgm Change:");
  Serial.println(String(program + 1));
  patchNo = program + 1;
  recallPatch(String(program + 1));
  state = PARAMETER;
}

void myMIDIClockStart() {
  //Resync LFOs when MIDI Clock starts.
  //When there's a jump to a different
  //part of a track, such as in a DAW, the DAW must have same
  //rhythmic quantisation as Tempo Div.
  if (vcoLFOMidiClkSync == 1) {
    vcoLfo.sync();
  }
  if (vcfLFOMidiClkSync == 1 ) {
    vcfLfo.sync();
  }
}

void myMIDIClock() {
  //This recalculates theLFO frequencies if the tempo changes (MIDI cLock is 24ppq)
  if ((vcoLFOMidiClkSync == 1 || vcfLFOMidiClkSync == 1) && count > 23) {
    float timeNow = millis();
    midiClkTimeInterval = (timeNow - previousMillis);
    lfoSyncFreq = 1000.0 / midiClkTimeInterval;
    previousMillis = timeNow;
    if (vcoLFOMidiClkSync == 1) vcoLfo.frequency(lfoSyncFreq * lfoTempoValue); //MIDI CC only
    if (vcfLFOMidiClkSync == 1) vcfLfo.frequency(lfoSyncFreq * lfoTempoValue);
    count = 0;
  }
  if (count < 24)count++;//prevent eventual overflow
}

void recallPatch(String patchNo) {
  File patchFile = SD.open(patchNo);
  if (!patchFile) {
    Serial.println("File not found");
  } else {
    String data[NO_OF_PARAMS];    //Array of data read in
    recallPatchData(patchFile, data);
    setCurrentPatchData(data);
    patchFile.close();
  }
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
  vcoLFOMidiClkSync = data[30].toFloat();//MIDI CC Only
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
  pitchEnv = data[46].toFloat();

  updatePatchname();
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
  updatePitchEnv();
  Serial.print("Set Patch: ");
  Serial.println(patchName);
}

String getCurrentPatchData() {
  return patchName + "," + String(VCOALevel)  + "," + String(VCOBLevel)  + "," + String(noiseLevel)  + "," + String(unison) + "," + String(ringMod) + "," + String(detune) + "," + String(lfoSyncFreq) + "," + String(midiClkTimeInterval) + "," + String(lfoTempoValue)  + "," + String(keytrackingAmount)  + "," + String(glideSpeed)  + "," + String(vcoOctaveA)  + "," + String(vcoOctaveB)  + "," + String(vcoWaveformA)  + "," + String(vcoWaveformB)  + "," +
         String(pwmSource)  + "," + String(pwmAmtA)  + "," + String(pwmAmtB)  + "," + String(pwmRate)  + "," + String(pwA)  + "," + String(pwB)  + "," + String(filterRes)  + "," + String(filterFreq)  + "," + String(filterMix)  + "," + String(filterEnv)  + "," + String(vcoLfoAmt)  + "," + String(vcoLfoRate)  + "," + String(vcoLFOWaveform)  + "," + String(vcoLfoRetrig)  + "," + String(vcoLFOMidiClkSync)  + "," + String(vcfLfoRate)  + "," +
         vcfLfoRetrig  + "," + vcfLFOMidiClkSync  + "," + vcfLfoAmt  + "," + vcfLfoWaveform  + "," + vcfAttack  + "," + vcfDecay  + "," + vcfSustain  + "," + vcfRelease  + "," + vcaAttack  + "," + vcaDecay  + "," + vcaSustain  + "," + vcaRelease  + "," +
         String(fxAmt)  + "," + String(fxMix) + "," + String(pitchEnv);
}

void checkMux() {
  mux1Read = analogRead(MUX1_S);
  mux2Read = analogRead(MUX2_S);
  if (mux1Read > (mux1ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux1Read < (mux1ValuesPrev[muxInput] - QUANTISE_FACTOR)) {
    mux1ValuesPrev[muxInput] = mux1Read;
    mux1Read = (mux1Read >> 3); //Change range to 0-127

    switch (muxInput) {
      case MUX1_octaveA:
        myControlChange(midiChannel, CCoctaveA, mux1Read);
        break;
      case MUX1_octaveB:
        myControlChange(midiChannel, CCoctaveB, mux1Read);
        break;
      case MUX1_vcowaveformA:
        myControlChange(midiChannel, CCvcowaveformA, mux1Read);
        break;
      case MUX1_vcowaveformB:
        myControlChange(midiChannel, CCvcowaveformB, mux1Read);
        break;
      case MUX1_vcoLevelA:
        myControlChange(midiChannel, CCvcoLevelA, mux1Read);
        break;
      case MUX1_vcoLevelB:
        myControlChange(midiChannel, CCvcoLevelB, mux1Read);
        break;
      case MUX1_pwA:
        myControlChange(midiChannel, CCpwA, mux1Read);
        break;
      case MUX1_pwB:
        myControlChange(midiChannel, CCpwB, mux1Read);
        break;
      case MUX1_pwmRate:
        myControlChange(midiChannel, CCpwmRate, mux1Read);
        break;
      case MUX1_vcoLfoRate:
        myControlChange(midiChannel, CCvcoLfoRate, mux1Read);
        break;
      case MUX1_noiseLevel:
        myControlChange(midiChannel, CCnoiseLevel, mux1Read);
        break;
      case MUX1_vcoLfoWaveform:
        myControlChange(midiChannel, CCvcoLfoWaveform, mux1Read);
        break;
      case MUX1_vcolfoamt:
        myControlChange(midiChannel, CCvcolfoamt, mux1Read);
        break;
      case MUX1_detune:
        myControlChange(midiChannel, CCdetune, mux1Read);
        break;
      case MUX1_filterfreq:
        myControlChange(midiChannel, CCfilterfreq, mux1Read);
        break;
      case MUX1_filterres:
        myControlChange(midiChannel, CCfilterres, mux1Read);
        break;
    }
  }

  if (mux2Read > (mux2ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux2Read < (mux2ValuesPrev[muxInput] - QUANTISE_FACTOR)) {
    mux2ValuesPrev[muxInput] = mux2Read;
    mux2Read = (mux2Read >> 3); //Change range to 0-127

    switch (muxInput) {
      case MUX2_vcflfoamt:
        myControlChange(midiChannel, CCvcflfoamt, mux2Read);
        break;
      case MUX2_vcflforate:
        myControlChange(midiChannel, CCvcflforate, mux2Read);
        break;
      case MUX2_filterenv:
        myControlChange(midiChannel, CCfilterenv, mux2Read);
        break;
      case MUX2_vcflfowaveform:
        myControlChange(midiChannel, CCvcflfowaveform, mux2Read);
        break;
      case MUX2_filtermixer:
        myControlChange(midiChannel, CCfiltermixer, mux2Read);
        break;
      case MUX2_keytracking:
        myControlChange(midiChannel, CCkeytracking, mux2Read);
        break;
      case MUX2_vcfattack:
        myControlChange(midiChannel, CCvcfattack, mux2Read);
        break;
      case MUX2_vcfdecay:
        myControlChange(midiChannel, CCvcfdecay, mux2Read);
        break;
      case MUX2_vcfsustain:
        myControlChange(midiChannel, CCvcfsustain, mux2Read);
        break;
      case MUX2_vcfrelease:
        myControlChange(midiChannel, CCvcfrelease, mux2Read);
        break;
      case MUX2_vcaattack:
        myControlChange(midiChannel, CCvcaattack, mux2Read);
        break;
      case MUX2_vcadecay:
        myControlChange(midiChannel, CCvcadecay, mux2Read);
        break;
      case MUX2_vcasustain:
        myControlChange(midiChannel, CCvcasustain, mux2Read);
        break;
      case MUX2_vcarelease:
        myControlChange(midiChannel, CCvcarelease, mux2Read);
        break;
      case MUX2_glide:
        myControlChange(midiChannel, CCglide, mux2Read);
        break;
      case MUX2_volume:
        myControlChange(midiChannel, CCvolume, mux2Read);
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
    myControlChange(midiChannel, CCfxamt, fxAmtRead);
  }
  fxMixRead = analogRead(EFFECTMIX_POT);
  if (fxMixRead > (fxMixPrevious + QUANTISE_FACTOR) || fxMixRead < (fxMixPrevious - QUANTISE_FACTOR)) {
    fxMixPrevious = fxMixRead;
    fxMixRead = (fxMixRead >> 3); //Change range to 0-127
    myControlChange(midiChannel, CCfxmix, fxMixRead);
  }
}

void checkSwitches() {
  pwmSourceSwitch.update();
  if (pwmSourceSwitch.risingEdge()) {
    pwmSource = PWMSOURCEFENV;
    myControlChange(midiChannel, CCpwmSource, pwmSource);
  } else if (pwmSourceSwitch.fallingEdge()) {
    pwmSource = PWMSOURCELFO;
    myControlChange(midiChannel, CCpwmSource, pwmSource);
  }

  unisonSwitch.update();
  if (unisonSwitch.risingEdge()) {
    unison = 1;
    myControlChange(midiChannel, CCunison, unison);
  } else if (unisonSwitch.fallingEdge()) {
    unison = 0;
    myControlChange(midiChannel, CCunison, unison);
  }

  ringModSwitch.update();
  if (ringModSwitch.risingEdge()) {
    ringMod = 1;
    myControlChange(midiChannel, CCringmod, ringMod);
    Serial.println("RING_MOD_SW: " + String(ringMod));
  } else if (ringModSwitch.fallingEdge()) {
    ringMod = 0;
    myControlChange(midiChannel, CCringmod, ringMod);
  }

  vcoLFORetrigSwitch.update();
  if (vcoLFORetrigSwitch.risingEdge()) {
    vcoLfoRetrig = 1;
    myControlChange(midiChannel, CCvcolforetrig, vcoLfoRetrig);
  } else if (vcoLFORetrigSwitch.fallingEdge()) {
    vcoLfoRetrig = 0;
    myControlChange(midiChannel, CCvcolforetrig, vcoLfoRetrig);
  }

  vcfLFORetrigSwitch.update();
  if (vcfLFORetrigSwitch.risingEdge()) {
    vcfLfoRetrig = 1;
    myControlChange(midiChannel, CCvcflforetrig, vcfLfoRetrig);
  } else if (vcfLFORetrigSwitch.fallingEdge()) {
    vcfLfoRetrig = 0;
    myControlChange(midiChannel, CCvcflforetrig, vcfLfoRetrig);
  }

  tempoSwitch.update();
  if (tempoSwitch.risingEdge()) {
    vcfLFOMidiClkSync = 1;
    myControlChange(midiChannel, CCvcfLFOMidiClkSync, vcfLFOMidiClkSync);
  } else  if (tempoSwitch.fallingEdge()) {
    vcfLFOMidiClkSync = 0;
    myControlChange(midiChannel, CCvcfLFOMidiClkSync, vcfLFOMidiClkSync);
  }

  saveButton.update();
  if (saveButton.read() == LOW && saveButton.duration() > INITIALISE_DURATION) {
    state = DELETE;
    saveButton.write(HIGH);//Come out of this state
    del = true;//Hack
  } else if (saveButton.risingEdge()) {
    if (!del) {
      switch (state) {
        case PARAMETER:
          if (patches.size() < PATCHES_LIMIT) {
            getPatches(SD.open("/"));//Reload patches from SD
            patches.push({patches.size() + 1, INITPATCHNAME});
            state = SAVE;
          }
          break;
        case SAVE:
          //Save as new patch with INITIALPATCH or overwrite existing keeping name - bypassing patch renaming
          patchName = patches.last().patchName;
          savePatch(String(patches.last().patchNo), getCurrentPatchData());
          showPatchPage(patches.last().patchNo, patches.last().patchName);
          getPatches(SD.open("/"));//Get rid of pushed patchNo if it wasn't saved TODO - Also resets circularbuffer
          renamedPatch = "";
          state = PARAMETER;
          break;
        case PATCHNAMING:
          //Save renamed patch
          //sort patches so new patch is saved at end
          patchName = renamedPatch;
          savePatch(String(patches.last().patchNo), getCurrentPatchData());
          showPatchPage(patches.last().patchNo, patchName);
          getPatches(SD.open("/"));//Get rid of pushed patchNo if it wasn't saved TODO - Also resets circularbuffer
          renamedPatch = "";
          state = PARAMETER;
          break;
      }
    } else {
      del = false;
    }
  }


  recallButton.update();
  if (recallButton.read() == LOW && recallButton.duration() > INITIALISE_DURATION) {
    //If recall held for 1.5s, set current patch to match current hardware state
    //Reinitialise all hardware values to force them to be re-read if different
    state = REINITIALISE;
    reinitialiseToPanel();
    recallButton.write(HIGH);//Come out of this state
    reini = true;//Hack
  } else if (recallButton.risingEdge()) {//cannot be fallingEdge because holding button won't work
    if (!reini) {
      switch (state) {
        case PARAMETER:
          state = RECALL;
          break;
        case RECALL:
          state = PATCH;
          //Recall the current patch
          patchNo = patches.first().patchNo;
          recallPatch(String(patchNo));
          state = PARAMETER;
          break;
      }
    } else {
      reini = false;
    }
  }

  backButton.update();
  if (backButton.read() == LOW && backButton.duration() > INITIALISE_DURATION) {
    //If Back button held for 1.5s, Panic - all notes off
    allNotesOff();
    backButton.write(HIGH);//Come out of this state
    panic = true;//Hack
  } else if (backButton.risingEdge()) {//cannot be fallingEdge because holding button won't work
    if (!panic) {
      switch (state) {
        case RECALL:
          state = PARAMETER;
          break;
        case SAVE:
          renamedPatch = "";
          state = PARAMETER;
          getPatches(SD.open("/"));
          break;
        case PATCHNAMING:
          renamedPatch = "";
          state = SAVE;
          break;
        case DELETE:
          state = PARAMETER;
          break;
      }
    } else {
      panic = false;
    }
  }

  enterButton.update();
  if (enterButton.fallingEdge()) {
    switch (state) {
      case PARAMETER:
      case RECALL:
        state = PATCH;
        patchNo = patches.first().patchNo;
        recallPatch(String(patchNo));
        state = PARAMETER;
        break;
      case SAVE:
        showRenamingPage(patches.last().patchName);
        state = PATCHNAMING;
        break;
      case PATCHNAMING:
        if (renamedPatch.length() < 16) {
          renamedPatch.concat(String(currentCharacter));
          charIndex = 0;
          currentCharacter = CHARACTERS[charIndex];
          showRenamingPage(renamedPatch);
        }
        break;
      case DELETE:
        state = PATCH;
        if (patches.size() > 0) {
          deletePatch(String(patches.first().patchNo));
          getPatches(SD.open("/"));
          patchNo = patches.first().patchNo;
          recallPatch(String(patchNo));
          resortPatches();//Make patch Nos consecutive on SD
          getPatches(SD.open("/"));
        }
        state = PARAMETER;
        break;
    }
  }
}

void reinitialiseToPanel() {
  //This reinialises the previous hardware values to force a re-read
  muxInput = 0;
  for (int i = 0; i < MUXCHANNELS; i++) {
    mux1ValuesPrev[i] = -9999;
    mux2ValuesPrev[i] = -9999;
  }
  fxAmtPrevious = -9999;
  fxMixPrevious = -9999;
  //Read switch state
  pwmSource = pwmSourceSwitch.read();
  updatePWMSource();
  ringMod = ringModSwitch.read();
  updateRingMod();
  vcoLfoRetrig = vcoLFORetrigSwitch.read();
  vcfLfoRetrig = vcfLFORetrigSwitch.read();
  unison = unisonSwitch.read();
  vcfLFOMidiClkSync = tempoSwitch.read();
  patchName = INITPATCHNAME;
  showPatchPage("Initial", "Panel Settings");
}

void checkEncoder() {
  //Encoder works with relative inc and dec values
  //Detent encoder goes up in 4 steps, hence +/-3

  long encRead = encoder.read();
  if (encRead > encPrevious + 3) {
    switch (state) {
      case PARAMETER:
        state = PATCH;
        patches.push(patches.shift());
        patchNo = patches.first().patchNo;
        recallPatch(String(patchNo));
        state = PARAMETER;
        break;
      case RECALL:
        patches.push(patches.shift());
        break;
      case SAVE:
        patches.push(patches.shift());
        break;
      case PATCHNAMING:
        if (charIndex == 63)charIndex = 0;
        currentCharacter = CHARACTERS[charIndex++];
        showRenamingPage(renamedPatch + currentCharacter);
        break;
      case DELETE:
        patches.push(patches.shift());
        break;
    }
    encPrevious = encRead;
  } else if (encRead < encPrevious - 3) {
    switch (state) {
      case PARAMETER:
        state = PATCH;
        patches.unshift(patches.pop());
        patchNo = patches.first().patchNo;
        recallPatch(String(patchNo));
        state = PARAMETER;
        break;
      case RECALL:
        patches.unshift(patches.pop());
        break;
      case SAVE:
        patches.unshift(patches.pop());
        break;
      case PATCHNAMING:
        if (charIndex == -1)charIndex = 62;
        currentCharacter = CHARACTERS[charIndex--];
        showRenamingPage(renamedPatch + currentCharacter);
        break;
      case DELETE:
        patches.unshift(patches.pop());
        break;
    }
    encPrevious = encRead;
  }
}

void storeMidiChannel(byte channel) {
  midiChannel = channel;
  EEPROM.write(0, channel);
}

void loop() {
  myusb.Task();
  midi1.read();//USB HOST MIDI Class Compliant
  usbMIDI.read();//USB Client MIDI
  MIDI.read();//MIDI 5 Pin DIN

  checkMux();
  checkFxPots();
  checkSwitches();
  checkEncoder();

  //Monitor MIDI In DIN
  //  if (Serial4.available() > 0) {
  //    Serial.print("UART received: ");
  //    Serial.println(Serial4.read(), HEX);
  //  }

//          Serial.print("CPU:");
//          Serial.print(AudioProcessorUsageMax());
//          Serial.print("  MEM:");
//          Serial.println(AudioMemoryUsageMax());
}
