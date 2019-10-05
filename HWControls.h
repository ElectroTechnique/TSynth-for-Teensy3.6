// This optional setting causes Encoder to use more optimized code,
// It must be defined before Encoder.h is included.
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <Bounce.h>

//Teensy 3.6 - Mux Pins
#define MUX_0 28
#define MUX_1 27
#define MUX_2 26
#define MUX_3 25
#define MUX1_S 16//A2 ADC0
#define MUX2_S 39//A20 ADC1
//Mux 1 Connections
#define MUX1_octaveA 0
#define MUX1_octaveB  1
#define MUX1_vcowaveformA 2
#define MUX1_vcowaveformB 3
#define MUX1_pwA 4
#define MUX1_pwB 5
#define MUX1_vcoLevelA 6
#define MUX1_vcoLevelB 7
#define MUX1_detune 8
#define MUX1_vcoLfoRate 9
#define MUX1_noiseLevel  10
#define MUX1_vcoLfoWaveform 11
#define MUX1_pwmRate 12
#define MUX1_vcolfoamt 13
#define MUX1_filterfreq 14
#define MUX1_filterres 15
//Mux 2 Connections
#define MUX2_vcflfoamt 0
#define MUX2_vcflforate 1
#define MUX2_filterenv 2
#define MUX2_vcflfowaveform 3
#define MUX2_filtermixer 4
#define MUX2_keytracking 5
#define MUX2_vcfattack 6
#define MUX2_vcfdecay 7
#define MUX2_vcfsustain 8
#define MUX2_vcfrelease 9
#define MUX2_vcaattack 10
#define MUX2_vcadecay 11
#define MUX2_vcasustain 12
#define MUX2_vcarelease 13
#define MUX2_glide 14
#define MUX2_volume 15

//Teensy 3.6 Pins
#define PWM_SOURCE_SW 33
#define RING_MOD_SW 35
#define PITCH_LFO_RETRIG_SW 34
#define VCF_LFO_RETRIG_SW 37
#define UNISON_SW 36
#define TEMPO_SW 38
#define ENTER_SW 17
#define SAVE_SW 30
#define RECALL_SW 24
#define BACK_SW 29
#define EFFECTAMT_POT A21
#define EFFECTMIX_POT A22

#define ENCODER_PINA 5
#define ENCODER_PINB 4

#define MUXCHANNELS 16
#define QUANTISE_FACTOR 7

#define DEBOUNCE 30

static byte muxInput = 0;
static int mux1ValuesPrev[MUXCHANNELS] = {};
static int mux2ValuesPrev[MUXCHANNELS] = {};

static int mux1Read = 0;
static int mux2Read = 0;
static int fxAmtRead = 0;
static int fxMixRead = 0;
static int fxAmtPrevious = 0;
static int fxMixPrevious = 0;
static long encPrevious = 0;

//Switches require debouncing
Bounce pwmSourceSwitch = Bounce(PWM_SOURCE_SW, DEBOUNCE);  
Bounce ringModSwitch = Bounce(RING_MOD_SW, DEBOUNCE); 
Bounce vcoLFORetrigSwitch = Bounce(PITCH_LFO_RETRIG_SW, DEBOUNCE); 
Bounce vcfLFORetrigSwitch = Bounce(VCF_LFO_RETRIG_SW, DEBOUNCE); 
Bounce unisonSwitch = Bounce(UNISON_SW, DEBOUNCE); 
Bounce tempoSwitch = Bounce(TEMPO_SW, DEBOUNCE); 

//These are pushbuttons and require debouncing
Bounce enterButton = Bounce(ENTER_SW, DEBOUNCE);  
Bounce saveButton = Bounce(SAVE_SW, DEBOUNCE);  
Bounce recallButton = Bounce(RECALL_SW, DEBOUNCE); 
Bounce backButton = Bounce(BACK_SW, DEBOUNCE);  
boolean reini = true;//Hack for recall button
Encoder encoder(ENCODER_PINB, ENCODER_PINA);

void setupHardware() {
  pinMode(MUX_0, OUTPUT);
  pinMode(MUX_1, OUTPUT);
  pinMode(MUX_2, OUTPUT);
  pinMode(MUX_3, OUTPUT);

  pinMode(PWM_SOURCE_SW, INPUT_PULLUP);
  pinMode(RING_MOD_SW, INPUT_PULLUP);
  pinMode(PITCH_LFO_RETRIG_SW, INPUT_PULLUP);
  pinMode(VCF_LFO_RETRIG_SW, INPUT_PULLUP);
  pinMode(UNISON_SW, INPUT_PULLUP);
  pinMode(TEMPO_SW, INPUT_PULLUP);

  pinMode(ENTER_SW, INPUT_PULLUP);//On encoder
  pinMode(SAVE_SW, INPUT_PULLUP);
  pinMode(RECALL_SW, INPUT_PULLUP);
  pinMode(BACK_SW, INPUT_PULLUP);
}

void incrementEnc() {
  Serial.println("incrementEnc");
}

void decrementEnc() {
  Serial.println("decrementEnc");
}
