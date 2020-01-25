# TSynth
Teensy 3.6 based synthesizer using PJRC Audio Board and Audio Lib

More details at PJRC Forum https://forum.pjrc.com/threads/57418-TSynth-Two-Oscillator-Polysynth

![](https://forum.pjrc.com/attachment.php?attachmentid=17379&d=1567073211)

Specifications

Oscillators
- Six voice polyphony (last note priority), two oscillators per voice, detunable with +/- 2 octaves range, Sine/Sample & Hold (like tuned noise)/Square/Sawtooth/Ramp/PWM/Var Triangle/User waveforms and level
- Pulse Width/Var Triangle can be set for each oscillator with PWM by dedicated LFO or from the filter envelope
- Pink or white noise level
- Dedicated LFO for pitch mod (can be retriggered by note on) , Sine/Triangle/Sawtooth/Ramp/Square/S&H waveforms
- Pitch can be modulated by filter envelope (+/-)
- XOR ‘Ring Mod’ (creates lots of harmonics with certain waveforms)
- Unison with all eight oscillators detunable from each other

Filter
- State variable 12dB filter (SVF) with continuous mix between LP and HP (provides notch filter) and BP
- Cutoff freq and resonance
- Cutoff can be modulated by dedicated ADSR envelope (+/-), dedicated LFO and key tracking
- LFO has same waveforms as pitch LFO (can be retriggered by note on)  and rate can be set to match MIDI clock (tempo) with variable time division delay (1,3/4,1/2,1/4,1/8...)

Amplifier
- Dedicated ADSR envelope
- Glide (up to 1 octave range) with variable time
- Volume for DAC output
- Effect amount and mix  - currently for stereo ensemble chorus rate and mix but could be set up to allow choices in programmer

Programmer
- 160x80 IPS colour display
- Encoder with button for data entry, Back button for menu navigation
- Save and Recall buttons for storing patches, holding Recall initialises the current patch to match the panel controls. Holding the Save button takes you into a patch deletion page.
- The programmer makes the synth very flexible with future possibilities for changing User waveforms, alternative filters, alternative effects with further parameter settings.

MIDI
- USB HOST MIDI Class Compliant (direct connection to MIDI controller, no PC needed)
- USB Client MIDI In from PC
- MIDI In 5 pin DIN

Audio
- SGTL5000 Audio Shield 16 bit, 44.1 kHz  Stereo out
- USB Audio in/out—appears as 16 bit, 44.1 kHz  audio interface on PC

Hardware
- Teensy 3.6 with SGTL5000 Audio Shield. Two 4067 multiplexers providing 32 channels from the pots into two ADCs. The rest of the pots and switches use remaining pins on Teensy.
- Enclosure is laser cut acrylic with etched labels filled with yellow acrylic paint (this technique could be improved with experimentation), end cheeks are 3D printed.


USE

TSynth patch saving and recall works like an analogue polysynth from the late 70s (Prophet 5). When you recall a patch, all the front panel controls will be different values from those saved in the patch. Moving them will cause a jump to the current value.

- Back button cancels current mode such as save, recall, delete and rename patches.

- Recall shows list of patches. Use encoder to move through list. Enter button on encoder chooses highlighted patch or press Recall again. Recall also recalls the current patch settings if the panel controls have been altered. Holding Recall for 1.5s will initialise the synth with all the current panel control settings - the synth sounds the same as the controls are set.

- Save will save the current settings to a new patch at the end of the list or you can use the encoder to overwrite an existing patch. Press Save again to save it. If you want to name/rename the patch, press the encoder enter button and use the encoder and enter button to choose an alphanumeric name. Holding Save for 1.5s will go into a patch deletion mode. Use encoder and enter button to choose and delete patch. Patch numbers will be changed on the SD card to be consecutive again.

KNOWN ISSUES
- Plugging in a MIDI controller may alter current patch settings. Arturia Minilab sends its current panel control settings when plugged in, causing MIDI CC messages to be received by TSynth.
- MIDI In DIN doesn't like MIDI Clock signals which get mixed up with note on/off and CC messages, so it's set not to listen to MIDI Clock. MIDI Host and client USB is fine.
