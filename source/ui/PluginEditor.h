#pragma once

#include "../dsp/PluginProcessor.h"
#include "melatonin_inspector/melatonin_inspector.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment   = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    PluginProcessor& processorRef;

    juce::Slider       pitchSlider;
    juce::Slider       mixSlider;
    juce::ToggleButton monoListenButton { "Mono Listen" };
    juce::ComboBox     algorithmCombo;

    juce::Label pitchLabel { {}, "Pitch" };
    juce::Label mixLabel { {}, "Mix" };
    juce::Label algorithmLabel { {}, "Algorithm" };

    std::unique_ptr<SliderAttachment>   pitchAttachment;
    std::unique_ptr<SliderAttachment>   mixAttachment;
    std::unique_ptr<ButtonAttachment>   monoListenAttachment;
    std::unique_ptr<ComboBoxAttachment> algorithmAttachment;

    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton                      inspectButton { "Inspect the UI" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
