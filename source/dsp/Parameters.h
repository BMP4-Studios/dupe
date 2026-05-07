#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace Parameters
{
inline constexpr auto pitchID      = "pitch";
inline constexpr auto mixID        = "mix";
inline constexpr auto monoListenID = "monoListen";
inline constexpr auto algorithmID  = "algorithm";

enum Algorithm
{
    Classic = 0,
    MSWide  = 1
};

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using namespace juce;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID { pitchID, 1 },
                                                             "Pitch",
                                                             NormalisableRange<float> { 0.0f, 30.0f, 0.1f },
                                                             7.0f,
                                                             AudioParameterFloatAttributes().withLabel ("cents")));

    params.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID { mixID, 1 }, "Mix", NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 1.0f));

    params.push_back (std::make_unique<AudioParameterBool> (ParameterID { monoListenID, 1 }, "Mono Listen", false));

    params.push_back (std::make_unique<AudioParameterChoice> (
        ParameterID { algorithmID, 1 }, "Algorithm", StringArray { "Classic", "M/S Wide" }, Algorithm::Classic));

    return { params.begin(), params.end() };
}
} // namespace Parameters
