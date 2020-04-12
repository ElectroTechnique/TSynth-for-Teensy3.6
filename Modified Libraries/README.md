These are several library files that replace the Teensyduino installed ones. These go in .....\Arduino\hardware\teensy\avr\libraries\Audio\  

- synth_waveform has a modification to call a new function sync() to reset the waveform start - this is to allow LFOs to retrigger when note on is received. Also the addition of a 'silent' waveform to more easily give the waveform control an 'off' setting. 
- filter_variable.h has modifications to cutoff freq and resonance ranges
- effect_combine has an extra combineMode enumeration to turn it off and allow audio to pass through.
- effect_ensemble.h and effect_ensemble.cpp are a chorus/ensemble type effect  
- A reference to effect_ensemble.h needs adding to Audio.h
