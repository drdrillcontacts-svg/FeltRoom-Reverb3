#pragma once
#include <JuceHeader.h>
class FeltRoomProcessor final : public juce::AudioProcessor {
public:
 FeltRoomProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName() const override {return "FeltRoom";}
 bool acceptsMidi() const override {return false;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return 15.0;}
 int getNumPrograms() override {return 1;}
 int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override {return "User";}
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 void applyPreset(const std::array<float,8>&);
 juce::AudioProcessorValueTreeState params;
private:
 juce::Reverb reverb;
 std::array<juce::dsp::IIR::Filter<float>,2> lowCut,highCut;
 juce::AudioBuffer<float> wet, delay;
 int delayWrite=0;
 double sr=44100;
 // Smoothed values to avoid zipper noise
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> mixSmoothed;
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> loCutSmoothed;
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> hiCutSmoothed;
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> roomSizeSmoothed;
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> dampSmoothed;
 float lastLoFreq = 100.0f;
 float lastHiFreq = 12000.0f;
 bool isBusesLayoutSupported(const BusesLayout& l) const override {
  return l.getMainInputChannelSet()==l.getMainOutputChannelSet() &&
  (l.getMainInputChannelSet()==juce::AudioChannelSet::mono() || l.getMainInputChannelSet()==juce::AudioChannelSet::stereo());
 }
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FeltRoomProcessor)
};
