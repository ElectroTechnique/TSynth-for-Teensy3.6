This is the current firmware for TSynth - use the latest, earlier ones will be left here. It is a hex file that is flashed to Teensy using [Teensy Loader from PJRC](https://www.pjrc.com/teensy/loader.html) You will probably need to press the button on a the Teensy for the firmware to flash.


V1.17 - Velocity sensitivity added. There are four velocity curves (see [electrotechnique.cc](https://electrotechnique.cc) post about this) that change the volume level with the velocity value. Also a minor fix to PWM source changing that failed to switch the filter envelope on when coming from fixed PW.
