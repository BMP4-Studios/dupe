#include "PluginProcessor.h"
#include "../ui/PluginEditor.h"

PluginProcessor::PluginProcessor()
: AudioProcessor (BusesProperties()
                      .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
  apvts (*this, nullptr, "state", Parameters::createLayout())
{
    pitchParam      = apvts.getRawParameterValue (Parameters::pitchID);
    mixParam        = apvts.getRawParameterValue (Parameters::mixID);
    monoListenParam = apvts.getRawParameterValue (Parameters::monoListenID);
    algorithmParam  = apvts.getRawParameterValue (Parameters::algorithmID);
}

PluginProcessor::~PluginProcessor() = default;

const juce::String PluginProcessor::getName() const { return JucePlugin_Name; }
bool               PluginProcessor::acceptsMidi() const { return false; }
bool               PluginProcessor::producesMidi() const { return false; }
bool               PluginProcessor::isMidiEffect() const { return false; }
double             PluginProcessor::getTailLengthSeconds() const { return 0.0; }

int                PluginProcessor::getNumPrograms() { return 1; }
int                PluginProcessor::getCurrentProgram() { return 0; }
void               PluginProcessor::setCurrentProgram (int) {}
const juce::String PluginProcessor::getProgramName (int) { return {}; }
void               PluginProcessor::changeProgramName (int, const juce::String&) {}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) RTSAN_BLOCKING
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels      = 1;

    shifterUp.prepare (spec);
    shifterDown.prepare (spec);
}

void PluginProcessor::releaseResources()
{
    shifterUp.reset();
    shifterDown.reset();
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& main = layouts.getMainOutputChannelSet();
    if (main != juce::AudioChannelSet::stereo() && main != juce::AudioChannelSet::mono())
        return false;

    return main == layouts.getMainInputChannelSet();
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) noexcept RTSAN_NONBLOCKING
{
    juce::ScopedNoDenormals noDenormals;

    const auto cents      = pitchParam->load();
    const auto mix        = mixParam->load();
    const auto monoListen = monoListenParam->load() > 0.5f;
    const auto algorithm  = static_cast<int> (algorithmParam->load());

    shifterUp.setCents (cents);
    shifterDown.setCents (-cents);

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples  = buffer.getNumSamples();

    auto* L = buffer.getWritePointer (0);
    auto* R = numChannels > 1 ? buffer.getWritePointer (1) : L;

    for (int i = 0; i < numSamples; ++i)
    {
        // Sum input to mono — mono guitar sources may be on either or both channels.
        const float dry  = numChannels > 1 ? 0.5f * (L[i] + R[i]) : L[i];
        const float wetL = shifterUp.processSample (dry);
        const float wetR = shifterDown.processSample (dry);

        if (algorithm == Parameters::Algorithm::MSWide)
        {
            // Mid = dry, Side = decorrelated wet difference.
            // (L+R)/2 collapses to dry exactly, so mono is artifact-free.
            // mix [0,1] is rescaled to width [0,2] for stronger sides.
            const float side  = 0.5f * (wetL - wetR);
            const float width = 2.0f * mix;
            L[i]              = dry + width * side;
            R[i]              = dry - width * side;
        }
        else
        {
            L[i] = dry * (1.0f - mix) + wetL * mix;
            R[i] = dry * (1.0f - mix) + wetR * mix;
        }
    }

    if (monoListen && numChannels > 1)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const float sum = 0.5f * (L[i] + R[i]);
            L[i]            = sum;
            R[i]            = sum;
        }
    }
}

void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor*         PluginProcessor::createEditor() { return new PluginEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PluginProcessor(); }
