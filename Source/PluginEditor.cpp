/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BinauralizerAudioProcessorEditor::BinauralizerAudioProcessorEditor (BinauralizerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    azimuthSlider.setSliderStyle(Slider::Rotary);
    azimuthSlider.setRange(-179, 180, 1);
    azimuthSlider.setRotaryParameters(float_Pi, float_Pi *3, false);
    azimuthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 20);
    azimuthSlider.setValue(0.0);
    azimuthSlider.setDoubleClickReturnValue(true, 0.0);
    
    elevationSlider.setSliderStyle(Slider::LinearVertical);
    elevationSlider.setRange(-40, 90, 1);
    elevationSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 20);
    elevationSlider.setValue(0.0);
    elevationSlider.setDoubleClickReturnValue(true, 0.0);
    
    gainSlider.setSliderStyle(Slider::LinearVertical);
    gainSlider.setRange(-80, 12, 1);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 20);
    gainSlider.setValue(0.0f);
    gainSlider.setDoubleClickReturnValue(true, 0.0);
    
    //add sliders to GUI
    addAndMakeVisible(&azimuthSlider);
    addAndMakeVisible(&elevationSlider);
    addAndMakeVisible(&gainSlider);
    
    //attach label to sliders
    azimuthLabel.setText("Azimuth", dontSendNotification);
    azimuthLabel.attachToComponent(&azimuthSlider, false);
    azimuthLabel.setJustificationType(Justification::centredTop);
    elevationLabel.setText("Elevation", dontSendNotification);
    elevationLabel.attachToComponent(&elevationSlider, false);
    elevationLabel.setJustificationType(Justification::centredTop);
    gainLabel.setText("Gain", dontSendNotification);
    gainLabel.attachToComponent(&gainSlider, false);
    gainLabel.setJustificationType(Justification::centredTop);
    
    //add this as listener
    azimuthSlider.addListener(this);
    elevationSlider.addListener(this);
    gainSlider.addListener(this);
    
}

BinauralizerAudioProcessorEditor::~BinauralizerAudioProcessorEditor()
{
}

//==============================================================================
void BinauralizerAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::bottom, 1);
}

void BinauralizerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    azimuthSlider.setBounds(40, 40, 150, 150);
    elevationSlider.setBounds(200, 40, 65, 150);
    gainSlider.setBounds(295, 40, 65, 150);
}

void BinauralizerAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    if (slider == &azimuthSlider)
    {
        processor.azimuth = slider->getValue();
    }
    else if (slider == &elevationSlider)
    {
        processor.elevation = slider->getValue();
    }
    else if (slider == &gainSlider)
    {
        processor.gain = slider->getValue();
    }
}
