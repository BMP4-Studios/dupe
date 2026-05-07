#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <vector>

#include "../util/RealtimeAttributes.h"

/**
    Rotational double-tap micro-pitch-shifter.

    Two read heads on a circular buffer, offset by half its length. Each head moves at the pitch-shifted rate; an
    equal-power crossfade hides the wrap discontinuity at the active head's edge by handing off to the head that's
    currently mid-buffer. Cheap, low-latency, and the canonical algorithm for +/- 50 cent doubling effects.

    Mono in, mono out. For stereo, use two instances.
*/
template <typename SampleType>
class MicroPitchShifter
{
public:
    MicroPitchShifter() = default;

    void prepare (const juce::dsp::ProcessSpec& spec) RTSAN_BLOCKING
    {
        constexpr int  minBufferSamples = 32;
        constexpr auto msToSec          = 0.001;
        const auto     bufferSamples    = static_cast<int> ((bufferLengthMs * msToSec) * spec.sampleRate);

        bufferSize = juce::jmax (minBufferSamples, bufferSamples);
        buffer.assign (static_cast<size_t> (bufferSize), SampleType (0));

        reset();
    }

    void reset() noexcept
    {
        std::fill (buffer.begin(), buffer.end(), SampleType (0));
        writePos = 0;
        delayA   = SampleType (0);
    }

    /** Pitch shift in cents. Negative shifts down, positive shifts up. */
    void setCents (float cents) noexcept
    {
        constexpr auto centsPerOctave = SampleType (1200);
        ratio                         = std::pow (SampleType (2), static_cast<SampleType> (cents) / centsPerOctave);
    }

    SampleType processSample (SampleType input) noexcept RTSAN_NONBLOCKING
    {
        // Write input into the ring buffer first.
        buffer[static_cast<size_t> (writePos)] = input;

        const auto bufferSizeF = static_cast<SampleType> (bufferSize);
        const auto halfBuffer  = bufferSizeF * SampleType (0.5);

        // Two read positions: A trails the write head by delayA samples; B trails by delayA + halfBuffer. Wrap A
        // first, *then* derive B from the wrapped value — otherwise B can land more than one buffer below zero
        // and a single conditional wrap leaves it negative.
        auto readPosA = static_cast<SampleType> (writePos) - delayA;
        if (readPosA < SampleType (0))
            readPosA += bufferSizeF;

        auto readPosB = readPosA - halfBuffer;
        if (readPosB < SampleType (0))
            readPosB += bufferSizeF;

        const auto a = interpolate (readPosA);
        const auto b = interpolate (readPosB);

        // Hann-window crossfade. Heads read the *same* buffer at offset positions, so the two contributions are
        // correlated and need constant-amplitude (sin² + cos² = 1) — equal-power (sin + cos) would over-shoot by up to
        // √2 on slowly varying content.
        const auto phase   = delayA / bufferSizeF;
        const auto sinHalf = std::sin (phase * juce::MathConstants<SampleType>::pi);
        const auto cosHalf = std::cos (phase * juce::MathConstants<SampleType>::pi);

        const auto wA = sinHalf * sinHalf; // peaks at phase = 0.5
        const auto wB = cosHalf * cosHalf; // peaks at phase = 0 and 1

        const auto out = wA * a + wB * b;

        // Advance state. Sub-sample-accurate delay for smooth pitch tracking.
        delayA += SampleType (1) - ratio;
        if (delayA < SampleType (0))
            delayA += bufferSizeF;

        if (delayA >= bufferSizeF)
            delayA -= bufferSizeF;

        ++writePos;
        if (writePos >= bufferSize)
            writePos = 0;

        return out;
    }

    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept RTSAN_NONBLOCKING
    {
        const auto& inputBlock  = context.getInputBlock();
        auto&       outputBlock = context.getOutputBlock();

        const auto numSamples = outputBlock.getNumSamples();

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        // Mono shifter: only the first channel is processed.
        const auto* in  = inputBlock.getChannelPointer (0);
        auto*       out = outputBlock.getChannelPointer (0);

        for (size_t i = 0; i < numSamples; ++i)
            out[i] = processSample (in[i]);
    }

private:
    SampleType interpolate (SampleType pos) const noexcept RTSAN_NONBLOCKING
    {
        // Catmull-Rom cubic Hermite. Reads four samples around `pos`.
        const auto pos1 = static_cast<int> (pos);
        const auto frac = pos - static_cast<SampleType> (pos1);

        const auto i0 = (pos1 - 1 + bufferSize) % bufferSize;
        const auto i1 = pos1 % bufferSize;
        const auto i2 = (pos1 + 1) % bufferSize;
        const auto i3 = (pos1 + 2) % bufferSize;

        const auto y0 = buffer[static_cast<size_t> (i0)];
        const auto y1 = buffer[static_cast<size_t> (i1)];
        const auto y2 = buffer[static_cast<size_t> (i2)];
        const auto y3 = buffer[static_cast<size_t> (i3)];

        const auto c0 = y1;
        const auto c1 = SampleType (0.5) * (y2 - y0);
        const auto c2 = y0 - SampleType (2.5) * y1 + SampleType (2) * y2 - SampleType (0.5) * y3;
        const auto c3 = SampleType (0.5) * (y3 - y0) + SampleType (1.5) * (y1 - y2);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }

    static constexpr double bufferLengthMs = 50.0;

    std::vector<SampleType> buffer;
    int                     bufferSize = 0;
    int                     writePos   = 0;
    SampleType              delayA     = SampleType (0);
    SampleType              ratio      = SampleType (1);
};
