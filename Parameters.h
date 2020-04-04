//Values below are just for initialising and will be changed when synth is initialised to current panel controls & EEPROM settings
byte midiChannel = MIDI_CHANNEL_OMNI;//(EEPROM)
String patchName = INITPATCHNAME;
float VCOALevel = 1;
float VCOBLevel = 1;
float noiseLevel = 0;
int unison = 0;
int oscFX = 0;
float detune = 0.98;
float lfoSyncFreq = 1.0;
float midiClkTimeInterval = 0.0;
float lfoTempoValue = 1.0;
int pitchBendRange = 12;
float modWheelDepth = 0.2f;
float keytrackingAmount = 0.5;//MIDI CC & Menu option (EEPROM)
float glideSpeed = 0;
int vcoOctaveA = 0;
int vcoOctaveB = 12;
float pitchEnv = 0;
int vcoWaveformA = WAVEFORM_SQUARE;
int vcoWaveformB = WAVEFORM_SQUARE;
float pwmAmtA = 1;
float pwmAmtB = 1;
float pwmRate = 0.5;
float pwA = 0;
float pwB = 0;
int pwmSource = PWMSOURCELFO;

float filterRes = 1.1;
float filterFreq = 12000;
float filterMix = 0;
int filterMixStr = 0;//For display
float filterEnv = 0;
float vcoLfoAmt = 0;
float vcoLfoRate = 4;
int vcoLFOWaveform = WAVEFORM_SINE;
int vcoLfoRetrig = 0;
int vcoLFOMidiClkSync = 0;//MIDI Only
String vcoLFOTimeDivStr = "";//For display
float vcfLfoRate = 2;
int vcfLfoRetrig = 0;
int vcfLFOMidiClkSync = 0;
String vcfLFOTimeDivStr = "";//For display
float vcfLfoAmt = 0;
int vcfLfoWaveform = WAVEFORM_SINE;

float vcfAttack = 100;
float vcfDecay = 350;
float vcfSustain = 0.7;
float vcfRelease = 300;

float vcaAttack = 10;
float vcaDecay = 35;
float vcaSustain = 1;
float vcaRelease = 300;

float fxAmt = 1;
float fxMix = 0;
