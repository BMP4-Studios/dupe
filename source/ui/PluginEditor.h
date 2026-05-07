#pragma once

#include "../dsp/PluginProcessor.h"
#include "melatonin_inspector/melatonin_inspector.h"

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions) -- JUCE_DECLARE_NON_COPYABLE deletes copies; moves
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    PluginProcessor& processorRef;

    juce::Slider       pitchSlider;
    juce::Slider       mixSlider;
    juce::Slider       haasSlider;
    juce::ToggleButton monoListenButton { "Mono Listen" };

    juce::Label pitchLabel { {}, "Pitch" };
    juce::Label mixLabel { {}, "Mix" };
    juce::Label haasLabel { {}, "Haas" };

    std::unique_ptr<SliderAttachment> pitchAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> haasAttachment;
    std::unique_ptr<ButtonAttachment> monoListenAttachment;

    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton                      inspectButton { "Inspect the UI" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
