#include "PluginEditor.h"
#include "BinaryData.h"

PluginEditor::PluginEditor (PluginProcessor& p) : AudioProcessorEditor (&p), processorRef (p)
{
    auto setupRotary = [this] (juce::Slider& s)
    {
        addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    };

    setupRotary (pitchSlider);
    setupRotary (mixSlider);

    pitchAttachment = std::make_unique<SliderAttachment> (processorRef.apvts, Parameters::pitchID, pitchSlider);
    mixAttachment   = std::make_unique<SliderAttachment> (processorRef.apvts, Parameters::mixID, mixSlider);

    addAndMakeVisible (monoListenButton);
    monoListenAttachment
        = std::make_unique<ButtonAttachment> (processorRef.apvts, Parameters::monoListenID, monoListenButton);

    addAndMakeVisible (algorithmCombo);
    algorithmCombo.addItem ("Classic", 1);
    algorithmCombo.addItem ("M/S Wide", 2);
    algorithmAttachment
        = std::make_unique<ComboBoxAttachment> (processorRef.apvts, Parameters::algorithmID, algorithmCombo);

    auto setupLabel = [this] (juce::Label& l)
    {
        addAndMakeVisible (l);
        l.setJustificationType (juce::Justification::centred);
    };

    setupLabel (pitchLabel);
    setupLabel (mixLabel);
    setupLabel (algorithmLabel);
    algorithmLabel.setJustificationType (juce::Justification::centredRight);

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

    setSize (420, 280);
}

PluginEditor::~PluginEditor() = default;

void PluginEditor::paint (juce::Graphics& g)
{
    const auto bg = juce::ImageCache::getFromMemory (BinaryData::background_jpg, BinaryData::background_jpgSize);
    g.drawImage (bg, getLocalBounds().toFloat());
}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced (12);

    auto top = area.removeFromTop (24);
    algorithmLabel.setBounds (top.removeFromLeft (90));
    algorithmCombo.setBounds (top.removeFromLeft (160));
    area.removeFromTop (8);

    auto bottom = area.removeFromBottom (30);
    inspectButton.setBounds (bottom.removeFromRight (140));
    monoListenButton.setBounds (bottom.removeFromLeft (140));

    area.removeFromBottom (8);

    auto knobs     = area;
    auto pitchArea = knobs.removeFromLeft (knobs.getWidth() / 2);

    pitchLabel.setBounds (pitchArea.removeFromTop (24));
    pitchSlider.setBounds (pitchArea);

    mixLabel.setBounds (knobs.removeFromTop (24));
    mixSlider.setBounds (knobs);
}
