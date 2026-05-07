#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Parameters.h"
#include "MicroPitchShifter.h"
#include "../util/RealtimeAttributes.h"

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) RTSAN_BLOCKING override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) noexcept RTSAN_NONBLOCKING override;

    juce::AudioProcessorEditor* createEditor() override;
    bool                        hasEditor() const override { return true; }

    const juce::String getName() const override;

    bool   acceptsMidi() const override;
    bool   producesMidi() const override;
    bool   isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int                getNumPrograms() override;
    int                getCurrentProgram() override;
    void               setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void               changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    MicroPitchShifter<float>                                                  shifterUp;
    MicroPitchShifter<float>                                                  shifterDown;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> haasDelay;

    std::atomic<float>* pitchParam      = nullptr;
    std::atomic<float>* mixParam        = nullptr;
    std::atomic<float>* monoListenParam = nullptr;
    std::atomic<float>* haasParam       = nullptr;

    double currentSampleRate = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
