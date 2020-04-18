![](https://electrotechnique.github.io/etlogo.png)

# Teensy 3.6 based synthesizer using PJRC Audio Board and Audio Lib

Currently under development. The pcb and front panel as seen below will be available around May 2020 from Tindie.com with SMD 4067 multiplexers, 6N138 opto-isolator, capacitors and resistors fitted. The entire cost of parts to build TSynth will be around $99 if you buy components from the cheaper suppliers and the build time around two hours to solder. Plans for a 3D printed/laser cut enclosure will be available.
Questions: info@electrotechnique.cc

# Latest News
**19th April 2020** - The code is now mostly finished with improvements in various areas. Glide is now polyphonic. The schematic and a number of files for making the enclosure have been added. The PCB and front panel designs are finalised.

**6th April 2020** - The main PCB has had a few minor revisons including 0.1uF capacitors across all the potentiometers to reduce noise. I'm considering adding a footprint to allow a 3.5mm jack for TRS MIDI as an option instead of the 5 pin DIN, which would allow the synth to be less high. The code has had some improvements - sawtooth and square waves are now band limited _to some extent_ by using wavetables for sets of notes and loading these as arbitrary waves. The Settings menu allows you to set MIDI channel, pitchbend range, key tracking and mod wheel depth. This can be extended to allow other functionality including velocity sensitivity.

![](fp.jpg)

![](pcb.jpg)

# Specifications

Oscillators
- Six voice polyphony (last note priority), two oscillators per voice, detunable with +/- 2 octaves range, Sine/Sample & Hold (like tuned noise)/Square/Sawtooth/Ramp/PWM/Var Triangle/User waveforms and level
- Pulse Width/Var Triangle can be set for each oscillator with PWM by dedicated LFO or from the filter envelope
- Pink or white noise level
- Dedicated LFO for pitch mod (can be retriggered by note on), Sine/Triangle/Sawtooth/Ramp/Square/S&H waveforms
- Pitch can be modulated by filter envelope (+/-)
- XOR ‘Ring Mod’ (creates lots of harmonics with certain waveforms)
- Unison with all twelve oscillators detunable from each other
- Polyphonic Glide with variable time

Filter
- State variable 12dB filter (SVF) with continuous mix between LP and HP (provides notch filter) and BP
- Cutoff freq and resonance
- Cutoff can be modulated by dedicated ADSR envelope (+/-), dedicated LFO
- LFO has same waveforms as pitch LFO (can be retriggered by note on)  and rate can be set to match MIDI clock (tempo) with variable time division (1,3/4,1/2,1/4,1/8...)

Amplifier
- Dedicated ADSR envelope
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
- Teensy 3.6 with SGTL5000 Audio Shield. Two 4067 multiplexers providing 32 channels from the pots into two ADCs. The rest of the pots and switches use remaining pins on Teensy
- Enclosure is laser cut acrylic with PCB-base front panel and 3D printed end cheeks


USE

TSynth patch saving and recall works like an analogue polysynth from the late 70s (Prophet 5). When you recall a patch, all the front panel controls will be different values from those saved in the patch. Moving them will cause a jump to the current value.

- Back button cancels current mode such as save, recall, delete and rename patches. Holding this for 1s is 'Panic', all notes off.

- Recall shows list of patches. Use encoder to move through list. Enter button on encoder chooses highlighted patch or press Recall again. Recall also recalls the current patch settings if the panel controls have been altered. 

- Save will save the current settings to a new patch at the end of the list or you can use the encoder to overwrite an existing patch. Press Save again to save it. If you want to name/rename the patch, press the encoder enter button and use the encoder and enter button to choose an alphanumeric name. Holding Save for 1s will go into a patch deletion mode. Use encoder and enter button to choose and delete patch. Patch numbers will be changed on the SD card to be consecutive again.

- Settings is a menu for things not on the front panel such as pitch bend range, mod wheeel range, MIDI channel and can be extended to other global functions. Holding this for 1s will initialise the synth with all the current panel control settings - the synth sounds the same as the controls are set.

KNOWN ISSUES
- Occasional digital noises from audio over USB, possibly attributable to the 44117Hz sample rate T3.6 uses. T4 uses 44100Hz and will probably be better. Audio from Audio Board is fine.
- Plugging in a MIDI controller may alter current patch settings. Arturia Minilab for example, sends its current panel control settings when plugged in, causing MIDI CC messages to be received by TSynth.
- Low cost (sub $10) USB to MIDI converters can have problems handling Clock signals and SysEx, which get mixed up with note on/off and CC messages. Use quality converters made by a known brand name.
