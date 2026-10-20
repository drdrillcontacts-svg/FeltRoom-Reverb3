#pragma once
#include "PluginProcessor.h"
class FeltRoomEditor final:public juce::AudioProcessorEditor {
public:
 explicit FeltRoomEditor(FeltRoomProcessor&);
 ~FeltRoomEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 FeltRoomProcessor& proc;
 juce::Image piano;
 struct CopperLook final:juce::LookAndFeel_V4 {void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;} look;
 std::array<std::unique_ptr<juce::Slider>,7> knobs;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,7> attachments;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
 juce::ComboBox presets,mode;
 juce::TextButton save{"SAVE"},load{"LOAD"};
 std::unique_ptr<juce::FileChooser> chooser;
 void savePreset();void loadPreset();
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FeltRoomEditor)
};
