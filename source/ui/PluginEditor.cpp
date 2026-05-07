#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
constexpr int editorWidth      = 420;
constexpr int editorHeight     = 280;
constexpr int outerMargin      = 12;
constexpr int gap              = 8;
constexpr int labelHeight      = 24;
constexpr int bottomRowHeight  = 30;
constexpr int bottomButtonW    = 140;
constexpr int sliderTextWidth  = 80;
constexpr int sliderTextHeight = 20;
} // namespace

PluginEditor::PluginEditor (PluginProcessor& p) : AudioProcessorEditor (&p), processorRef (p)
{
    auto setupRotary = [this] (juce::Slider& s)
    {
        addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, sliderTextWidth, sliderTextHeight);
    };

    setupRotary (pitchSlider);
    setupRotary (mixSlider);
    setupRotary (haasSlider);

    pitchAttachment = std::make_unique<SliderAttachment> (processorRef.getApvts(), Parameters::pitchID, pitchSlider);
    mixAttachment   = std::make_unique<SliderAttachment> (processorRef.getApvts(), Parameters::mixID, mixSlider);
    haasAttachment  = std::make_unique<SliderAttachment> (processorRef.getApvts(), Parameters::haasID, haasSlider);

    addAndMakeVisible (monoListenButton);
    monoListenAttachment
        = std::make_unique<ButtonAttachment> (processorRef.getApvts(), Parameters::monoListenID, monoListenButton);

    auto setupLabel = [this] (juce::Label& l)
    {
        addAndMakeVisible (l);
        l.setJustificationType (juce::Justification::centred);
    };

    setupLabel (pitchLabel);
    setupLabel (mixLabel);
    setupLabel (haasLabel);

    addAndMakeVisible (inspectButton);
    inspectButton.onClick = [this]
    {
        if (! inspector)
        {
            inspector          = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this] { inspector.reset(); };
        }
        inspector->setVisible (true);
    };

    setSize (editorWidth, editorHeight);
}

PluginEditor::~PluginEditor() = default;

void PluginEditor::paint (juce::Graphics& g)
{
    const auto bg = juce::ImageCache::getFromMemory (BinaryData::background_jpg, BinaryData::background_jpgSize);
    g.drawImage (bg, getLocalBounds().toFloat());
}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced (outerMargin);

    auto bottom = area.removeFromBottom (bottomRowHeight);
    inspectButton.setBounds (bottom.removeFromRight (bottomButtonW));
    monoListenButton.setBounds (bottom.removeFromLeft (bottomButtonW));

    area.removeFromBottom (gap);

    auto       knobs     = area;
    const auto third     = knobs.getWidth() / 3;
    auto       pitchArea = knobs.removeFromLeft (third);
    auto       mixArea   = knobs.removeFromLeft (third);
    auto       haasArea  = knobs;

    pitchLabel.setBounds (pitchArea.removeFromTop (labelHeight));
    pitchSlider.setBounds (pitchArea);

    mixLabel.setBounds (mixArea.removeFromTop (labelHeight));
    mixSlider.setBounds (mixArea);

    haasLabel.setBounds (haasArea.removeFromTop (labelHeight));
    haasSlider.setBounds (haasArea);
}
