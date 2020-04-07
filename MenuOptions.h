#define MENUOPTIONSNO 4
#define MENUVALUESNO 18//Maximum number of menu option values needed
int menuValueIndex = 0;//currently selected menu option value index

struct MenuOption
{
  char * option;//Menu option string
  char *value[MENUVALUESNO];//Array of strings of menu option values
  int  handler;//Function to handle the values for this menu option
  int  currentIndex;//Function to array index of current value for this menu option
};

void menuMIDICh(char * value);
void menuKeyTracking(char * value);
void menuPitchBend(char * value);
void menuModWheelDepth(char * value);
void menuHandler(char * s, void (*f)(char*));

int currentIndexMIDICh();
int currentIndexKeyTracking();
int currentIndexPitchBend();
int currentIndexModWheelDepth();
int getCurrentIndex(int (*f)());


void menuMIDICh(char * value) {
  if (strcmp(value, "ALL") == 0) {
    midiChannel = MIDI_CHANNEL_OMNI;
  } else {
    midiChannel = atoi(value);
  }
  storeMidiChannel(midiChannel);
}
void menuKeyTracking(char * value) {
  if (strcmp(value, "None") == 0) keytrackingAmount = 0;
  if (strcmp(value, "Half") == 0)  keytrackingAmount =  0.5;
  if (strcmp(value, "Full") == 0) keytrackingAmount =  1.0;
  storeKeyTracking(keytrackingAmount);
}

void menuPitchBend(char * value) {
  pitchBendRange = atoi(value);
  storePitchBendRange(pitchBendRange);
}

void menuModWheelDepth(char * value) {
  modWheelDepth = atoi(value)/10.0f;
  storeModWheelDepth(modWheelDepth);
}

//Takes a pointer to a specific method for the menu option and invokes it.
void menuHandler(char * s, void (*f)(char*) ) {
  f(s);
}

int currentIndexMIDICh() {
  return getMIDIChannel();
}

int currentIndexKeyTracking() {
  float value = getKeyTracking();
  if (value == 0) return 0;
  if (value == 0.5)  return 1;
  if (value == 1.0) return 2;
  return 0;
}

int currentIndexPitchBend() {
  return  getPitchBendRange() - 1;
}

int currentIndexModWheelDepth() {
  return (getModWheelDepth() * 10) - 1;
}

//Takes a pointer to a specific method for the current menu option value and invokes it.
int getCurrentIndex(int (*f)() ) {
  return f();
}

CircularBuffer<MenuOption, MENUOPTIONSNO>  menuOptions;

// add menus to the circular buffer
void setUpMenus() {
  menuOptions.push(MenuOption{"MIDI Ch.", {"All", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", '\0'}, menuMIDICh, currentIndexMIDICh});
  menuOptions.push(MenuOption{"Key Tracking", {"None", "Half", "Full", '\0'}, menuKeyTracking, currentIndexKeyTracking});
  menuOptions.push(MenuOption{"Pitch Bend", {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", '\0'}, menuPitchBend, currentIndexPitchBend});
  menuOptions.push(MenuOption{"MW Depth", {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", '\0'}, menuModWheelDepth, currentIndexModWheelDepth});
}
