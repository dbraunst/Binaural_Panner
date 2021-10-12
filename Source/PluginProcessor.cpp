/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "hrtf.h"
#include "main.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BinauralizerAudioProcessor::BinauralizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::mono(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    hrtf.readHRTFJUCE(formatManager);
    
    tempBuffer.setSize(2, 512 + HRTF_LEN - 1);
    
    azimuth = 0.0f;
    elevation = 0.0f;
    gain = 0.0f;
}

BinauralizerAudioProcessor::~BinauralizerAudioProcessor()
{
    
}

//==============================================================================
const String BinauralizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BinauralizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BinauralizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BinauralizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BinauralizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BinauralizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BinauralizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BinauralizerAudioProcessor::setCurrentProgram (int index)
{
}

const String BinauralizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void BinauralizerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void BinauralizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
}

void BinauralizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BinauralizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    
    //if inputs aren't 1 OR 0, and outputs aren't 2
    if ((layouts.getMainInputChannelSet() != AudioChannelSet::mono()
         || layouts.getMainInputChannelSet() != AudioChannelSet::stereo())
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    return true;
  #endif
}
#endif

void BinauralizerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //retrieve HRTF Index based on the current azimuth/elevation of the sliders
    HRTFindex = hrtf.selectHRTF(azimuth, elevation);
    
//TODO: if input == 2ch convert stereo->mono

    const auto* pin = buffer.getReadPointer(0);
    auto pinTracker = pin[0];
    
    auto* tempL = tempBuffer.getWritePointer(0, 0);
    auto* tempR = tempBuffer.getWritePointer(1, 0);
    auto* outputL = buffer.getWritePointer(0);
    auto* outputR = buffer.getWritePointer(1);
    
    //getNumSamples returns samples/channel in buffer
    int num_frames = buffer.getNumSamples();
    
    for (int i=0; i < num_frames; i++)
    {
        tempL[HRTF_LEN - 1 + i] = pin[i];
        tempR[HRTF_LEN - 1 + i] = pin[i];
    }

    hrtf.convolveHRTF(&tempL[HRTF_LEN], &tempR[HRTF_LEN], outputL, outputR, HRTFindex, num_frames, gain);
    
    tempL = tempBuffer.getWritePointer(0, 0);
    tempR = tempBuffer.getWritePointer(1, 0);
    
    for (int i=0; i<HRTF_LEN-1; i++)
    {
        tempL[i] = tempL[num_frames+i];
        tempR[i] = tempR[num_frames+i];
    }
}

//==============================================================================
bool BinauralizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BinauralizerAudioProcessor::createEditor()
{
    return new BinauralizerAudioProcessorEditor (*this);
}

//==============================================================================
void BinauralizerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BinauralizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BinauralizerAudioProcessor();
}
