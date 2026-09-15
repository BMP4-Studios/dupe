#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
constexpr auto editorWidth      = 420;
constexpr auto editorHeight     = 420;
constexpr auto knobHeight       = 150;
constexpr auto outerMargin      = 12;
constexpr auto gap              = 8;
constexpr auto labelHeight      = 24;
constexpr auto bottomRowHeight  = 30;
constexpr auto bottomButtonW    = 140;
constexpr auto sliderTextWidth  = 80;
constexpr auto sliderTextHeight = 20;
} // namespace

PluginEditor::PluginEditor (PluginProcessor& p) : AudioProcessorEditor (&p), processorRef (p)
{
    dupeLogo.setImage (juce::ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize));
    addAndMakeVisible (dupeLogo);

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
    monoListenAttachment = std::make_unique<ButtonAttachment> (processorRef.getApvts(), Parameters::monoListenID, monoListenButton);

    auto setupLabel = [this] (juce::Label& l)
    {
        addAndMakeVisible (l);
        l.setJustificationType (juce::Justification::centred);
    };

    setupLabel (pitchLabel);
    setupLabel (mixLabel);
    setupLabel (haasLabel);

#if JUCE_DEBUG
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
#endif

    setSize (editorWidth, editorHeight);
}

void PluginEditor::paint (juce::Graphics& g)
{
    const auto bg = juce::ImageCache::getFromMemory (BinaryData::background_jpg, BinaryData::background_jpgSize);
    g.drawImage (bg, getLocalBounds().toFloat());
}

void PluginEditor::resized()
{
    auto bounds = getLocalBounds().reduced (outerMargin);
    auto bottomRow = bounds.removeFromBottom (bottomRowHeight);
    [[maybe_unused]] const auto inspectBounds {bottomRow.removeFromRight (bottomButtonW)};

#if JUCE_DEBUG
    inspectButton.setBounds (inspectBounds);
#endif

    monoListenButton.setBounds (bottomRow.removeFromLeft (bottomButtonW));

    bounds.removeFromBottom (gap);

    auto       knobBounds     = bounds.removeFromBottom (knobHeight);
    const auto third     = knobBounds.getWidth() / 3;
    auto       pitchArea = knobBounds.removeFromLeft (third);
    auto       mixArea   = knobBounds.removeFromLeft (third);
    auto       haasArea  = knobBounds;

    pitchLabel.setBounds (pitchArea.removeFromTop (labelHeight));
    pitchSlider.setBounds (pitchArea);

    mixLabel.setBounds (mixArea.removeFromTop (labelHeight));
    mixSlider.setBounds (mixArea);

    haasLabel.setBounds (haasArea.removeFromTop (labelHeight));
    haasSlider.setBounds (haasArea);

    bounds.removeFromBottom (gap);
    dupeLogo.setBounds (bounds);
}
