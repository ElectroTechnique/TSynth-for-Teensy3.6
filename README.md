# TSynth
Teensy 3.6 based synthesizer using PJRC Audio Board and Audio Lib

More details at PJRC Forum https://www.youtube.com/redirect?q=https%3A%2F%2Fforum.pjrc.com%2Fthreads%2F57418-TSynth-Two-Oscillator-Polysynth&event=video_description&v=jmnZkLp9bjI&redir_token=YrY5PhZMRgDk-zFijMkkrkQBzFF8MTU3NTQ0NTg2M0AxNTc1MzU5NDYz

Specifications

Oscillators
- Four voice polyphony (last note priority), two oscillators per voice, detunable with +/- 2 octaves range, Sine/Sample & Hold (like tuned noise)/Square/Sawtooth/Ramp/PWM/Var Triangle/User waveforms and level
- Pulse Width/Var Triangle can be set for each oscillator with PWM by dedicated LFO or from the filter envelope
- Pink noise with level
- Dedicated LFO for pitch mod (can be retriggered by note on) , Sine/Triangle/Sawtooth/Ramp/Square/S&H waveforms
- XOR ‘Ring Mod’ (creates lots of harmonics with certain waveforms)
- Unison with all eight oscillators detunable from each other

Filter
- State variable 12dB filter (SVF) with continuous mix between LP and HP (provides notch filter) and BP
- Cutoff freq and resonance
- Cutoff can be modulated by dedicated ADSR envelope, dedicated LFO and key tracking
- LFO has same waveforms as pitch LFO (can be retriggered by note on)  and rate can be set to match MIDI clock  (tempo) with variable delay per bar

Amplifier
- Dedicated ADSR envelope
- Glide (up to 1 octave range) with variable time
- Volume for DAC output
- Effect amount and mix  - currently for stereo chorus but could be set up to allow choices in programmer

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
- Teensy 3.6 with SGTL5000 Audio Shield. Two 4061 multiplexers providing 32 channels from the pots into two ADCs. The rest of the pots and switches use remaining pins on Teensy.
- Enclosure is laser cut acrylic with etched labels filled with yellow acrylic paint (this technique could be improved with experimentation), end cheeks are 3D printed.

