/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "main.h"
#include "hrtf.h"
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class BinauralizerAudioProcessorEditor  : public AudioProcessorEditor,
                            public Slider::Listener
{
public:
    BinauralizerAudioProcessorEditor (BinauralizerAudioProcessor&);
    ~BinauralizerAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BinauralizerAudioProcessor& processor;
    
    void sliderValueChanged (Slider* slider) override;
    
    //Sliers + Labels
    Slider azimuthSlider;
    Label azimuthLabel;
    Slider elevationSlider;
    Label elevationLabel;
    Slider gainSlider;
    Label gainLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BinauralizerAudioProcessorEditor)
};
