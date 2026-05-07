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
    haasParam       = apvts.getRawParameterValue (Parameters::haasID);
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

    currentSampleRate = sampleRate;

    // Haas-style precedence delay applied to wetR. Antisymmetric side preserves mono compatibility; the delay
    // decorrelates L/R in time to broaden stereo image. Allocate for the full parameter range; delay is set per-block.
    const auto maxHaasSamples = static_cast<int> (Parameters::haasMaxMs * 0.001f * sampleRate) + 1;
    haasDelay.setMaximumDelayInSamples (maxHaasSamples);
    haasDelay.prepare (spec);
    haasDelay.reset();
}

void PluginProcessor::releaseResources()
{
    shifterUp.reset();
    shifterDown.reset();
    haasDelay.reset();
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
    const auto haasMs     = haasParam->load();

    shifterUp.setCents (cents);
    shifterDown.setCents (-cents);

    haasDelay.setDelay (haasMs * 0.001f * static_cast<float> (currentSampleRate));

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

        haasDelay.pushSample (0, wetR);
        const float wetRDelayed = haasDelay.popSample (0);

        // Mid = dry, Side = decorrelated wet difference with optional Haas offset on the negative branch.
        // Side stays antisymmetric so (L+R)/2 collapses to dry exactly, preserving mono compatibility.
        // mix [0,1] rescaled to width [0,4] for aggressive sides.
        const float side  = 0.5f * (wetL - wetRDelayed);
        const float width = 4.0f * mix;
        L[i]              = dry + width * side;
        R[i]              = dry - width * side;
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
