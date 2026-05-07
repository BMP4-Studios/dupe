#include <dsp/MicroPitchShifter.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <juce_dsp/juce_dsp.h>

#include <cmath>

namespace
{
constexpr double defaultSampleRate = 44100.0;
constexpr int    defaultBlockSize  = 512;
constexpr int    rngSeed           = 1234;
constexpr int    oneSecondSamples  = 44100;
constexpr int    primeIterations   = 8192;
constexpr int    measureIterations = 1024;
constexpr float  dcTolerance       = 1.0e-3f;
constexpr float  freqTolerance     = 2.0f;
constexpr float  peakBound         = 1.5f;
constexpr float  testCentsSmall    = 10.0f;
constexpr float  testCentsTypical  = 7.0f;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- test helper, callers use named keyword-style invocations
juce::dsp::ProcessSpec makeSpec (double sampleRate = defaultSampleRate, int blockSize = defaultBlockSize)
{
    juce::dsp::ProcessSpec spec {};
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (blockSize);
    spec.numChannels      = 1;
    return spec;
}
} // namespace

TEST_CASE ("MicroPitchShifter:output stays bounded for white noise input", "[dsp][shifter]")
{
    MicroPitchShifter<float> shifter;
    shifter.prepare (makeSpec());
    shifter.setCents (testCentsSmall);

    juce::Random rng { rngSeed };

    float peak = 0.0f;
    for (int i = 0; i < oneSecondSamples; ++i)
    {
        const float in  = rng.nextFloat() * 2.0f - 1.0f;
        const float out = shifter.processSample (in);
        peak            = std::max (peak, std::abs (out));
    }

    // Equal-power crossfade of two interpolated samples can briefly exceed
    // the input's peak. peakBound is well within reason for [-1,1] white noise.
    REQUIRE (peak < peakBound);
}

TEST_CASE ("MicroPitchShifter:DC input remains DC after settling", "[dsp][shifter]")
{
    MicroPitchShifter<float> shifter;
    shifter.prepare (makeSpec());
    shifter.setCents (testCentsTypical);

    constexpr float dcLevel = 0.5f;

    // Prime the buffer with several full traversals so the read heads are
    // both pulling steady-state DC.
    for (int i = 0; i < primeIterations; ++i)
    {
        shifter.processSample (dcLevel);
    }

    for (int i = 0; i < measureIterations; ++i)
    {
        const float out = shifter.processSample (dcLevel);
        REQUIRE_THAT (out, Catch::Matchers::WithinAbs (dcLevel, dcTolerance));
    }
}

TEST_CASE ("MicroPitchShifter:shifts a sine wave's frequency", "[dsp][shifter]")
{
    constexpr double sampleRate            = defaultSampleRate;
    constexpr float  inputFreq             = 440.0f;      // A4
    constexpr float  cents                 = 100.0f;      // one semitone up
    constexpr float  semitoneRatio         = 1.05946309f; // 2^(1/12)
    constexpr float  expected              = inputFreq * semitoneRatio;
    constexpr float  zeroCrossingsPerCycle = 0.5f;

    MicroPitchShifter<float> shifter;
    shifter.prepare (makeSpec (sampleRate));
    shifter.setCents (cents);

    // Warm up past the buffer-fill latency.
    for (int i = 0; i < primeIterations; ++i)
    {
        const float in = std::sin (2.0f * juce::MathConstants<float>::pi * inputFreq * static_cast<float> (i)
                                   / static_cast<float> (sampleRate));
        shifter.processSample (in);
    }

    // Count zero-crossings over a 1-second window to estimate output frequency.
    int   crossings = 0;
    float prev      = 0.0f;
    for (int i = 0; i < static_cast<int> (sampleRate); ++i)
    {
        const float in  = std::sin (2.0f * juce::MathConstants<float>::pi * inputFreq * static_cast<float> (i)
                                    / static_cast<float> (sampleRate));
        const float out = shifter.processSample (in);

        if ((prev <= 0.0f) != (out <= 0.0f))
        {
            ++crossings;
        }
        prev = out;
    }

    // 2 zero crossings per cycle → freq ≈ crossings/2.
    const float measured = static_cast<float> (crossings) * zeroCrossingsPerCycle;

    // Allow a small tolerance: chorus modulation from the wrap rate adds jitter to the perceived pitch.
    REQUIRE_THAT (measured, Catch::Matchers::WithinAbs (expected, freqTolerance));
}
