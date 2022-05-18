//
//  SynthNote.cpp
//  MySynth Plugin Source Code - for individual notes
//
//  Used to define the bodies of functions used by the plugin, as declared in SynthPlugin.h.
//

#include "SynthNote.h"
#define VibDepth parameters[2]
#define Gain2 parameters[4]
#define NoteLength parameters[5]
#define Position parameters[7]

//================================================================================
// MyNote - object representing a single note (within the synthesiser - see above)
//================================================================================

// Triggered when a note is started (use to initialise / prepare note processing)
void MyNote::onStartNote (int pitch, float velocity)
{
    iPitch = pitch;
    // convert note number to fundamental frequency (Hz)
    fFrequency = 440.f * pow (2.f, (pitch - 69.f) / 12.f);
    
    int iOldPitch = getSynthesiser()->getLastNote();
    float fOldFrequency = 440.f * pow (2.f, (iOldPitch - 69.f) / 12.f);
    
    portamento.set(Envelope::Points (0, fOldFrequency) (0.05, fFrequency) );
     
    fLevel = velocity;
    
    getSynthesiser()->rememberNote(pitch);
    
    pitchEG.set(Envelope::Points (0, 0.9) (0.09, 1) );
    
    amplitudeEG.set(Envelope::Points (0, 0) (0.1, 1) );
    amplitudeEG.setLoop(1, 1);

    crossfade.set(Envelope::Points (0, 0) (1, 1));
    
    formant[0].reset();
    formant[1].reset();
    
    vibrato.reset();
    vibrato.setFrequency(5);

    firstFormant.setFilters(fFreq_Ah, fQ_Ah); // Frequencies, Qs and Gains for "Ah"
    firstFormant.setGain(fGain_Ah);

    if (iPitch <= 60) {
        secondFormant.setFilters(fFreg_Oh, fQ_Oh); // Frequencies, Qs and Gains for "Oh"
        secondFormant.setGain(fGain_Oh);
    }
    else {
        secondFormant.setFilters(fFreg_Ee, fQ_Ee); // Frequencies, Qs and Gains for "Ee"
        secondFormant.setGain(fGain_Ee);
    }
}
    
// Triggered when a note is stopped (return false to keep the note alive)
bool MyNote::onStopNote (float velocity)
{
    if (Position < 0.2) {
        amplitudeEG.release(0.1); // faster release in first section -> shorter notes
    }
    else {
        amplitudeEG.release(0.3);
    }
    return false;
}

void MyNote::onPitchWheel (int value){

}
 
void MyNote::onControlChange (int controller, int value){

}
    
// Called to render the note's next buffer of audio (generates the sound)
// (return false to terminate the note)
bool MyNote::process (float** outputBuffer, int numChannels, int numSamples)
{
    float* pfOutBuffer0 = outputBuffer[0], *pfOutBuffer1 = outputBuffer[1];

    float fFormantGain1 = parameters[0];
    float fVibDepth = parameters[2] * 2;
    float fFormantGain2 = parameters[4];
    
    float fOutGain = parameters[9];

    float fModIndex = 3;
    
    while(numSamples--)
    {
        
        float fFadeEnvelope = crossfade.tick();
        float fAmplitudeEnv = amplitudeEG.tick();
        float fVibrato = vibrato.tick();
        fFrequency = portamento.tick();
        
        // CONDITIONING
        if (NoteLength > 0) {
            VibDepth = NoteLength; // Increase vibrato on longer notes
        }
        else {
            VibDepth = 0;
        }
        
        if (Position == 1) { // End
            Gain2 = (1 - (parameters[6] * 0.9)); // Turn down 2nd formant gain at the end for smoother fade
        }
        else {
            Gain2 = 1;
        }
        
        if (Position < 0.2) { // Intro
            Gain2 = 0; // Turn down second formant in intro section
            formant[0].setFrequency(fFrequency * 0.5 * pitchEG.tick()); // Add slight detune to simulate inconsistencies                                                                    while 'singing'
        }
        else {
            formant[0].setFrequency(fFrequency * 0.5);
        }
        
        formant[1].setFrequency((fVibrato * fModIndex * fVibDepth) + (fFrequency * 0.5)); // only apply vibrato to "Ee"/"Oh"
        
        // FIRST FORMANT "Ah"
        float fFirstFormant = formant[0].tick(); // Saw Wave
        firstFormant.applyFormantFilters(fFirstFiltered, fFirstFormant); // Apply Formant Filters
        fFirstFormant = fFirstFiltered[0] + fFirstFiltered[1] + fFirstFiltered[2] + fFirstFiltered[3];
        firstFormant.filterOutput(fFirstFormant);
        fFirstFormant *= (1 - fFadeEnvelope); // Crossfade between "Ah" & "Ee"/"Oh"
        fFirstFormant *= fFormantGain1;

        // SECOND FORMANT "Ee" / "Oh" -> depending on pitch
        float fSecondFormant = formant[1].tick(); // Saw Wave
        secondFormant.applyFormantFilters(fSecondFiltered, fSecondFormant); // Apply Formant Filters
        fSecondFormant = fSecondFiltered[0] + fSecondFiltered[1] + fSecondFiltered[2] + fSecondFiltered[3];
        secondFormant.filterOutput(fSecondFormant);
        fSecondFormant *= fFadeEnvelope; // Crossfade between "Ah" & "Ee"/"Oh"
        fSecondFormant *= fFormantGain2;
        
        // STEREO
        float fLeft = fFirstFormant; // "Ah" on both sides
        float fRight = fFirstFormant;
        if (iPitch <= 60) { // "Oh" = left, "Ee" = right
            fSecondFormant *= 1.2;
            fLeft = fFirstFormant + fSecondFormant;
        }
        else {
            fRight = fFirstFormant + fSecondFormant;
        }

        fLeft *= fAmplitudeEnv * fLevel;
        fRight *= fAmplitudeEnv * fLevel;

        *pfOutBuffer0++ = fLeft * fOutGain;
        *pfOutBuffer1++ = fRight * fOutGain;
    }

    return amplitudeEG.getStage() != Envelope::STAGE::ENV_OFF;
}


