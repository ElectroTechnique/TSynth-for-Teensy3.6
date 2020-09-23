This is the current firmware for TSynth - use the latest, earlier ones will be left here. It is a hex file that is flashed to Teensy using [Teensy Loader from PJRC](https://www.pjrc.com/teensy/loader.html) You will probably need to press the button on a the Teensy for the firmware to flash.

- V1.27 - The volume control affects digital and analogue audio together. This can be loud on the headphone socket.

- V1.26 - Improve ADC reads to decrease noise and increase sensitivity to pot changes. Increase AudioMemory to 48.

- V1.24 - Improved filter response at higher frequencies.

- V1.23 - Added MIDI Out - panel controls will send MIDI CC data over a channel set in Settings through USB Host and Client MIDI ports. Added VU Meter that shows peak amplitude on display (enabled in Settings.) Not a true VU meter, however.

- V1.22 - Fixed glide behaviour when notes were held.

- V1.21 - Fixed odd glide behaviour. Now glides correctly from previous note.

- V1.20 - Added oscilloscope display option in Settings. This is an 'experimental feature' that needs more work.

- V1.19 - Added 'Pick-up' option which allows certain controls (both front panel and MIDI) to only change the value when the current patch value is 'picked-up'. For example, if Freq Cutoff is set to 312Hz, adjusting Filter Cutoff will pick-up the value when the control is near 312Hz and then change it. This prevents sudden jumps in sound when playing and adjusting. Filter Cuttoff, Resonance, Filter Type, Pitch LFO Amount, Pitch LFO Rate, Filter LFO Amount, Filter LFO Rate, FX Amount and FX Mix have pick-up as these are most likely to be adjusted during performance. This feature is somewhat experimental but good enough to include. An indicator, 'PK' shows when pick-up is enabled on the parameter value page of the display. Also added 'Bass Enhance' to the Settings menu. This enhances bass frequencies using the SGTL5000 audio codec chip (not the Teensy) and will only be heard through the headphone jack not USB digital audio.

- V1.17 - Velocity sensitivity added. There are four velocity curves (see [electrotechnique.cc](https://electrotechnique.cc) post about this) that change the volume level with the velocity value. Also a minor fix to PWM source changing that failed to switch the filter envelope on when coming from fixed PW.

- V1.16 - Envelopes closed when a patch changes or the Panic button is pressed, to prevent next patch sounding.
