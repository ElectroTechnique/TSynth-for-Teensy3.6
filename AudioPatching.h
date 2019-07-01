// GUItool: begin automatically generated code
AudioSynthWaveformDc     constant1Dc;    //xy=69.5,437
AudioSynthNoisePink      pink;           //xy=268,349
AudioSynthWaveform       pwmLfo;         //xy=92,208
AudioSynthWaveformDc     pwa;            //xy=107,256
AudioSynthWaveformDc     pwb;            //xy=110,301
AudioEffectEnvelope      vcfEnvelope4;   //xy=617,1107
AudioEffectEnvelope      vcfEnvelope1;   //xy=628,300
AudioEffectEnvelope      vcfEnvelope3;   //xy=636,857
AudioEffectEnvelope      vcfEnvelope2;   //xy=640,604
AudioMixer4              pwMixer1a;       //xy=281,169
AudioMixer4              pwMixer1b;       //xy=287,272
AudioMixer4              pwMixer4b;         //xy=313.75,954.75
AudioMixer4              pwMixer4a;         //xy=314.75,887.75
AudioMixer4              pwMixer2a;         //xy=323.75,472.75
AudioMixer4              pwMixer2b;         //xy=323.75,553.75
AudioMixer4              pwMixer3a;         //xy=353.75,688.75
AudioMixer4              pwMixer3b;         //xy=358.75,763.75
AudioSynthWaveform       vcoLfo;         //xy=122,111
AudioSynthWaveformDc     glide;          //xy=124,152
AudioSynthWaveformDc     pitchBend;      //xy=128,74
AudioSynthWaveform       vcfLfo;         //xy=166,632
AudioSynthWaveformDc     keytracking;    //xy=176,745
AudioMixer4              vcoModMixer;    //xy=296,95
AudioSynthWaveformModulated waveformMod1b;  //xy=503,170
AudioSynthWaveformModulated waveformMod1a;  //xy=507,114
AudioSynthWaveformModulated waveformMod2b;  //xy=513,550
AudioSynthWaveformModulated waveformMod4a;  //xy=519,899
AudioSynthWaveformModulated waveformMod2a;  //xy=521,458
AudioSynthWaveformModulated waveformMod4b;  //xy=525,940
AudioSynthWaveformModulated waveformMod3a;  //xy=532,678
AudioSynthWaveformModulated waveformMod3b;  //xy=535,744
AudioEffectDigitalCombine ringMod1;       //xy=663.8888549804688,223.888916015625
AudioEffectDigitalCombine ringMod2;       //xy=684,542
AudioEffectDigitalCombine ringMod4;       //xy=686,1046
AudioEffectDigitalCombine ringMod3;       //xy=687,803
AudioMixer4              waveformMixer1; //xy=824,170
AudioMixer4              waveformMixer2; //xy=829,476
AudioMixer4              waveformMixer3; //xy=846,774
AudioMixer4              waveformMixer4; //xy=854,1023
AudioMixer4              vcfModMixer1;   //xy=845,292
AudioMixer4              vcfModMixer2;   //xy=848,637
AudioMixer4              vcfModMixer3;   //xy=852,937
AudioMixer4              vcfModMixer4;   //xy=855,1099
AudioFilterStateVariable filter2;        //xy=994,498
AudioFilterStateVariable filter1;        //xy=1000,210
AudioFilterStateVariable filter3;        //xy=1002,822
AudioFilterStateVariable filter4;        //xy=1022,1047
AudioMixer4              filterMixer2;   //xy=1144,504
AudioMixer4              filterMixer3;   //xy=1144,825
AudioMixer4              filterMixer1;   //xy=1151,214
AudioMixer4              filterMixer4;   //xy=1155,1050
AudioEffectEnvelope      vcaEnvelope2;   //xy=1315,503
AudioEffectEnvelope      vcaEnvelope3;   //xy=1315,823
AudioEffectEnvelope      vcaEnvelope4;   //xy=1321,1045
AudioEffectEnvelope      vcaEnvelope1;   //xy=1327,211
AudioMixer4              voiceMixer;     //xy=1524,570
AudioEffectChorus        chorusR;        //xy=1704,606
AudioEffectChorus        chorusL;        //xy=1706,558
AudioMixer4              chorusMixerR;         //xy=1848,625
AudioMixer4              chorusMixerL;         //xy=1857,539
AudioOutputUSB           usbAudio;       //xy=2356.11083984375,593.5556030273438
AudioOutputI2S           i2s;            //xy=2364.22216796875,547.2222290039062
AudioConnection          patchCord1(constant1Dc, vcfEnvelope2);
AudioConnection          patchCord2(constant1Dc, vcfEnvelope3);
AudioConnection          patchCord3(constant1Dc, vcfEnvelope4);
AudioConnection          patchCord4(constant1Dc, vcfEnvelope1);
AudioConnection          patchCord5(pwmLfo, 0, pwMixer1a, 0);
AudioConnection          patchCord6(pwmLfo, 0, pwMixer1b, 0);
AudioConnection          patchCord7(pwmLfo, 0, pwMixer2a, 0);
AudioConnection          patchCord8(pwmLfo, 0, pwMixer2b, 0);
AudioConnection          patchCord9(pwmLfo, 0, pwMixer3a, 0);
AudioConnection          patchCord10(pwmLfo, 0, pwMixer3b, 0);
AudioConnection          patchCord11(pwmLfo, 0, pwMixer4a, 0);
AudioConnection          patchCord12(pwmLfo, 0, pwMixer4b, 0);
AudioConnection          patchCord13(pwa, 0, pwMixer1a, 1);
AudioConnection          patchCord14(pwa, 0, pwMixer2a, 1);
AudioConnection          patchCord15(pwa, 0, pwMixer3a, 1);
AudioConnection          patchCord16(pwa, 0, pwMixer4a, 1);
AudioConnection          patchCord17(pwb, 0, pwMixer1b, 1);
AudioConnection          patchCord18(pwb, 0, pwMixer2b, 1);
AudioConnection          patchCord19(pwb, 0, pwMixer3b, 1);
AudioConnection          patchCord20(pwb, 0, pwMixer4b, 1);
AudioConnection          patchCord21(vcoLfo, 0, vcoModMixer, 1);
AudioConnection          patchCord22(glide, 0, vcoModMixer, 2);
AudioConnection          patchCord23(pitchBend, 0, vcoModMixer, 0);
AudioConnection          patchCord24(vcfLfo, 0, vcfModMixer1, 1);
AudioConnection          patchCord25(vcfLfo, 0, vcfModMixer2, 1);
AudioConnection          patchCord26(vcfLfo, 0, vcfModMixer3, 1);
AudioConnection          patchCord27(vcfLfo, 0, vcfModMixer4, 1);
AudioConnection          patchCord28(keytracking, 0, vcfModMixer1, 2);
AudioConnection          patchCord29(keytracking, 0, vcfModMixer2, 2);
AudioConnection          patchCord30(keytracking, 0, vcfModMixer3, 2);
AudioConnection          patchCord31(keytracking, 0, vcfModMixer4, 2);
AudioConnection          patchCord32(pink, 0, waveformMixer1, 2);
AudioConnection          patchCord33(pink, 0, waveformMixer2, 2);
AudioConnection          patchCord34(pink, 0, waveformMixer3, 2);
AudioConnection          patchCord35(pink, 0, waveformMixer4, 2);
AudioConnection          patchCord36(pwMixer1a, 0, waveformMod1a, 1);
AudioConnection          patchCord37(pwMixer1b, 0, waveformMod1b, 1);
AudioConnection          patchCord38(pwMixer1b, 0, waveformMod3b, 1);
AudioConnection          patchCord39(vcoModMixer, 0, waveformMod1a, 0);
AudioConnection          patchCord40(vcoModMixer, 0, waveformMod1b, 0);
AudioConnection          patchCord41(vcoModMixer, 0, waveformMod2a, 0);
AudioConnection          patchCord42(vcoModMixer, 0, waveformMod2b, 0);
AudioConnection          patchCord43(vcoModMixer, 0, waveformMod3a, 0);
AudioConnection          patchCord44(vcoModMixer, 0, waveformMod3b, 0);
AudioConnection          patchCord45(vcoModMixer, 0, waveformMod4a, 0);
AudioConnection          patchCord46(vcoModMixer, 0, waveformMod4b, 0);
AudioConnection          patchCord47(pwMixer4b, 0, waveformMod4b, 1);
AudioConnection          patchCord48(pwMixer4a, 0, waveformMod4a, 1);
AudioConnection          patchCord49(pwMixer2a, 0, waveformMod2a, 1);
AudioConnection          patchCord50(pwMixer2b, 0, waveformMod2b, 1);
AudioConnection          patchCord51(pwMixer3a, 0, waveformMod3a, 1);
AudioConnection          patchCord52(waveformMod1b, 0, waveformMixer1, 1);
AudioConnection          patchCord53(waveformMod1b, 0, ringMod1, 1);
AudioConnection          patchCord54(waveformMod1a, 0, waveformMixer1, 0);
AudioConnection          patchCord55(waveformMod1a, 0, ringMod1, 0);
AudioConnection          patchCord56(waveformMod2b, 0, waveformMixer2, 1);
AudioConnection          patchCord57(waveformMod2b, 0, ringMod2, 1);
AudioConnection          patchCord58(waveformMod4a, 0, waveformMixer4, 0);
AudioConnection          patchCord59(waveformMod4a, 0, ringMod4, 0);
AudioConnection          patchCord60(waveformMod2a, 0, waveformMixer2, 0);
AudioConnection          patchCord61(waveformMod2a, 0, ringMod2, 0);
AudioConnection          patchCord62(waveformMod4b, 0, waveformMixer4, 1);
AudioConnection          patchCord63(waveformMod4b, 0, ringMod4, 1);
AudioConnection          patchCord64(waveformMod3a, 0, ringMod3, 0);
AudioConnection          patchCord65(waveformMod3a, 0, waveformMixer3, 0);
AudioConnection          patchCord66(waveformMod3b, 0, ringMod3, 1);
AudioConnection          patchCord67(waveformMod3b, 0, waveformMixer3, 1);
AudioConnection          patchCord68(vcfEnvelope4, 0, vcfModMixer4, 0);
AudioConnection          patchCord69(vcfEnvelope4, 0, pwMixer4a, 2);
AudioConnection          patchCord70(vcfEnvelope4, 0, pwMixer4b, 2);
AudioConnection          patchCord71(vcfEnvelope1, 0, vcfModMixer1, 0);
AudioConnection          patchCord72(vcfEnvelope1, 0, pwMixer1a, 2);
AudioConnection          patchCord73(vcfEnvelope1, 0, pwMixer1b, 2);
AudioConnection          patchCord74(vcfEnvelope3, 0, vcfModMixer3, 0);
AudioConnection          patchCord75(vcfEnvelope3, 0, pwMixer3a, 2);
AudioConnection          patchCord76(vcfEnvelope3, 0, pwMixer3b, 2);
AudioConnection          patchCord77(vcfEnvelope2, 0, vcfModMixer2, 0);
AudioConnection          patchCord78(vcfEnvelope2, 0, pwMixer2a, 2);
AudioConnection          patchCord79(vcfEnvelope2, 0, pwMixer2b, 2);
AudioConnection          patchCord80(ringMod1, 0, waveformMixer1, 3);
AudioConnection          patchCord81(ringMod2, 0, waveformMixer2, 3);
AudioConnection          patchCord82(ringMod4, 0, waveformMixer4, 3);
AudioConnection          patchCord83(ringMod3, 0, waveformMixer3, 3);
AudioConnection          patchCord84(waveformMixer1, 0, filter1, 0);
AudioConnection          patchCord85(waveformMixer2, 0, filter2, 0);
AudioConnection          patchCord86(vcfModMixer1, 0, filter1, 1);
AudioConnection          patchCord87(waveformMixer3, 0, filter3, 0);
AudioConnection          patchCord88(vcfModMixer2, 0, filter2, 1);
AudioConnection          patchCord89(vcfModMixer3, 0, filter3, 1);
AudioConnection          patchCord90(waveformMixer4, 0, filter4, 0);
AudioConnection          patchCord91(vcfModMixer4, 0, filter4, 1);
AudioConnection          patchCord92(filter2, 0, filterMixer2, 0);
AudioConnection          patchCord93(filter2, 1, filterMixer2, 1);
AudioConnection          patchCord94(filter2, 2, filterMixer2, 2);
AudioConnection          patchCord95(filter1, 0, filterMixer1, 0);
AudioConnection          patchCord96(filter1, 1, filterMixer1, 1);
AudioConnection          patchCord97(filter1, 2, filterMixer1, 2);
AudioConnection          patchCord98(filter3, 0, filterMixer3, 0);
AudioConnection          patchCord99(filter3, 1, filterMixer3, 1);
AudioConnection          patchCord100(filter3, 2, filterMixer3, 2);
AudioConnection          patchCord101(filter4, 0, filterMixer4, 0);
AudioConnection          patchCord102(filter4, 1, filterMixer4, 1);
AudioConnection          patchCord103(filter4, 2, filterMixer4, 2);
AudioConnection          patchCord104(filterMixer2, vcaEnvelope2);
AudioConnection          patchCord105(filterMixer3, vcaEnvelope3);
AudioConnection          patchCord106(filterMixer1, vcaEnvelope1);
AudioConnection          patchCord107(filterMixer4, vcaEnvelope4);
AudioConnection          patchCord108(vcaEnvelope2, 0, voiceMixer, 1);
AudioConnection          patchCord109(vcaEnvelope3, 0, voiceMixer, 2);
AudioConnection          patchCord110(vcaEnvelope4, 0, voiceMixer, 3);
AudioConnection          patchCord111(vcaEnvelope1, 0, voiceMixer, 0);
AudioConnection          patchCord112(voiceMixer, chorusL);
AudioConnection          patchCord113(voiceMixer, chorusR);
AudioConnection          patchCord114(voiceMixer, 0, chorusMixerL, 0);
AudioConnection          patchCord115(voiceMixer, 0, chorusMixerR, 0);
AudioConnection          patchCord116(chorusR, 0, chorusMixerR, 1);
AudioConnection          patchCord117(chorusL, 0, chorusMixerL, 1);
AudioConnection          patchCord118(chorusMixerR, 0, usbAudio, 1);
AudioConnection          patchCord119(chorusMixerR, 0, i2s, 1);
AudioConnection          patchCord120(chorusMixerL, 0, i2s, 0);
AudioConnection          patchCord121(chorusMixerL, 0, usbAudio, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=2353.8890380859375,505.5556640625
// GUItool: end automatically generated code
