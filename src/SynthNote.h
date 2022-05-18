//
//  SynthNote.h
//  MySynth Plugin Header File - for individual notes
//
//  Used to declare objects and data structures used by the plugin.
//

#include "SynthPlugin.h"

#pragma once

//================================================================================
// MyNote - object representing a single note (within the synthesiser - see above)
//================================================================================

class MyNote : public APDI::Synth::Note
{
public:
    MyNote(MySynth* synthesiser) : Note(synthesiser), fFrequency(440.f), fLevel(1.f) { }
    
    MySynth* getSynthesiser() { return (MySynth*)synthesiser; }
    
    void onStartNote (int pitch, float velocity);
    bool onStopNote (float velocity);
    void onPitchWheel (int value);
    void onControlChange (int controller, int value);
    
    bool process (float** outputBuffer, int numChannels, int numSamples);
    
private:
    // Declare note class variables here
    float fFrequency;
    float fLevel;
    
    float fFreq_Ah [5] = { 936, 1551, 2815, 4299, 4568 }; // configurations for first formant "Ah"
    float fQ_Ah [4] = { 30, 22, 30, 3};
    float fGain_Ah [4] = { 1.67, 0.83, 0.96, 0.5};
    
    float fFreg_Ee [5] = { 341, 2495, 3121, 4078, 5000 }; // configurations for second formant "Ee"
    float fQ_Ee [4] = { 25, 23, 20, 11 };
    float fGain_Ee [4] = { 1.63, 0.83, 1.54, 0.32 };
    
    float fFreg_Oh [5] = { 514, 944, 2801, 3475, 1500 }; // configurations for second formant "Oh"
    float fQ_Oh [4] = { 30, 30, 20, 11} ;
    float fGain_Oh [4] = { 1.76, 1.2, 0.43, 0.1 };

    int iPitch;
    
    Envelope pitchEG;
    Envelope amplitudeEG;
    Envelope crossfade;
    Envelope portamento;
    
    Saw formant[2];
    Sine vibrato;

    float fFirstFiltered[4];
    Formant firstFormant;
    
    float fSecondFiltered[4];
    Formant secondFormant;
};
