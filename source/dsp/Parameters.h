#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace Parameters
{
inline constexpr auto pitchID      = "pitch";
inline constexpr auto mixID        = "mix";
inline constexpr auto monoListenID = "monoListen";
inline constexpr auto haasID       = "haas";

inline constexpr float pitchMinCents     = 0.0f;
inline constexpr float pitchMaxCents     = 40.0f;
inline constexpr float pitchStepCents    = 0.1f;
inline constexpr float pitchDefaultCents = 7.0f;

inline constexpr float mixMin     = 0.0f;
inline constexpr float mixMax     = 1.0f;
inline constexpr float mixStep    = 0.001f;
inline constexpr float mixDefault = 1.0f;

inline constexpr float haasMinMs     = 0.0f;
inline constexpr float haasMaxMs     = 30.0f;
inline constexpr float haasStepMs    = 0.1f;
inline constexpr float haasDefaultMs = 0.0f;

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using namespace juce;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID { pitchID, 1 },
        "Pitch",
        NormalisableRange<float> { pitchMinCents, pitchMaxCents, pitchStepCents },
        pitchDefaultCents,
        AudioParameterFloatAttributes().withLabel ("cents")));

    params.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID { mixID, 1 }, "Mix", NormalisableRange<float> { mixMin, mixMax, mixStep }, mixDefault));

    params.push_back (std::make_unique<AudioParameterBool> (ParameterID { monoListenID, 1 }, "Mono Listen", false));

    params.push_back (
        std::make_unique<AudioParameterFloat> (ParameterID { haasID, 1 },
                                               "Haas",
                                               NormalisableRange<float> { haasMinMs, haasMaxMs, haasStepMs },
                                               haasDefaultMs,
                                               AudioParameterFloatAttributes().withLabel ("ms")));

    return { params.begin(), params.end() };
}
} // namespace Parameters
