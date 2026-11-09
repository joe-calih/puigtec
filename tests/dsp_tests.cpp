#include "../Source/PluginProcessor.h"

#include <cmath>
#include <iostream>
#include <string>

namespace
{
int failedTests = 0;
constexpr double testSampleRate = 48000.0;
constexpr int testBufferSize = 48000;

void check(bool passed, const std::string& description)
{
    if (!passed)
    {
        std::cerr << "FAIL: " << description << '\n';
        ++failedTests;
    }
    else
    {
        std::cout << "PASS: " << description << '\n';
    }
}

void setValue(BlueSteelEQAudioProcessor& processor, const char* id, float value)
{
    processor.setParameterValue(id, value);
}

float measureSine(
    BlueSteelEQAudioProcessor& processor,
    float frequency,
    bool findPeak = false)
{
    juce::AudioBuffer<float> buffer(2, testBufferSize);
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    constexpr float amplitude = 0.2f;

    for (int index = 0; index < testBufferSize; ++index)
    {
        const auto phase = 2.0 * juce::MathConstants<double>::pi
            * static_cast<double>(frequency)
            * static_cast<double>(index)
            / testSampleRate;
        const auto sample = amplitude * static_cast<float>(std::sin(phase));
        left[index] = sample;
        right[index] = sample;
    }

    processor.prepareToPlay(testSampleRate, 512);
    juce::MidiBuffer midi;
    processor.processBlock(buffer, midi);

    if (findPeak)
    {
        auto peak = 0.0f;
        for (int index = testBufferSize / 4; index < testBufferSize; ++index)
            peak = juce::jmax(peak, std::abs(left[index]));
        return peak;
    }

    double sum = 0.0;
    int numberOfSamples = 0;
    for (int index = testBufferSize / 4; index < testBufferSize; ++index)
    {
        sum += static_cast<double>(left[index]) * left[index];
        ++numberOfSamples;
    }

    return static_cast<float>(std::sqrt(sum / numberOfSamples));
}

float measureAtGain(
    const char* parameterId,
    float parameterValue,
    float frequency,
    bool findPeak = false)
{
    BlueSteelEQAudioProcessor processor;
    setValue(processor, ParameterIDs::input, 0.0f);
    setValue(processor, ParameterIDs::output, 0.0f);
    processor.applyFactoryPreset(0);
    setValue(processor, parameterId, parameterValue);
    return measureSine(processor, frequency, findPeak);
}

void testNeutralResponse()
{
    BlueSteelEQAudioProcessor processor;
    processor.applyFactoryPreset(0);
    processor.prepareToPlay(testSampleRate, 512);

    for (const auto frequency : {40.0f, 1000.0f, 10000.0f})
        check(
            std::abs(processor.getFrequencyResponse(frequency)) < 0.15f,
            "The flat preset is neutral at " + std::to_string(juce::roundToInt(frequency)) + " Hz");
}

void testLowShelf()
{
    const auto flat = measureAtGain(ParameterIDs::lowBoost, 0.0f, 20.0f);
    const auto boosted = measureAtGain(ParameterIDs::lowBoost, 5.0f, 20.0f);
    check(boosted > flat * 1.45f, "The low shelf increases a low-frequency sine");

    const auto lowAttenuation = measureAtGain(ParameterIDs::lowAttenuation, 5.0f, 50.0f);
    const auto flatLow = measureAtGain(ParameterIDs::lowAttenuation, 0.0f, 50.0f);
    check(lowAttenuation < flatLow * 0.82f, "The low attenuation shelf reduces bass");
}

void testMidBands()
{
    const auto flatPeak = measureAtGain(ParameterIDs::midPeakGain, 0.0f, 1000.0f);
    const auto boostedPeak = measureAtGain(ParameterIDs::midPeakGain, 5.0f, 1000.0f);
    check(boostedPeak > flatPeak * 1.45f, "The mid-peak band boosts its selected frequency");

    const auto flatDip = measureAtGain(ParameterIDs::midDipGain, 0.0f, 3000.0f);
    const auto attenuatedDip = measureAtGain(ParameterIDs::midDipGain, 5.0f, 3000.0f);
    check(attenuatedDip < flatDip * 0.82f, "The mid-dip band attenuates its selected frequency");
}

void testHighBands()
{
    const auto flatHigh = measureAtGain(ParameterIDs::highBoost, 0.0f, 8000.0f);
    const auto boostedHigh = measureAtGain(ParameterIDs::highBoost, 5.0f, 8000.0f);
    check(boostedHigh > flatHigh * 1.35f, "The high-frequency peak adds presence");

    const auto flatAttenuation = measureAtGain(ParameterIDs::highAttenuation, 0.0f, 14000.0f);
    const auto highAttenuation = measureAtGain(ParameterIDs::highAttenuation, 5.0f, 14000.0f);
    check(highAttenuation < flatAttenuation * 0.85f, "The high attenuation shelf reduces treble");
}

void testCutFilters()
{
    BlueSteelEQAudioProcessor processor;
    processor.applyFactoryPreset(0);
    setValue(processor, ParameterIDs::lowCut, 3.0f);
    setValue(processor, ParameterIDs::highCut, 4.0f);
    processor.prepareToPlay(testSampleRate, 512);

    check(
        processor.getFrequencyResponse(40.0f) < processor.getFrequencyResponse(1000.0f) - 5.0f,
        "The low-cut filter attenuates frequencies below its cutoff");
    check(
        processor.getFrequencyResponse(12000.0f) < processor.getFrequencyResponse(1000.0f) - 5.0f,
        "The high-cut filter attenuates frequencies above its cutoff");
}

void testInputAndOutputGain()
{
    const auto nominal = measureAtGain(ParameterIDs::input, 0.0f, 1000.0f);
    const auto gained = measureAtGain(ParameterIDs::input, 6.0f, 1000.0f);
    check(gained > nominal * 1.85f && gained < nominal * 2.15f, "Input gain follows the decibel control");

    BlueSteelEQAudioProcessor processor;
    processor.applyFactoryPreset(0);
    setValue(processor, ParameterIDs::input, 0.0f);
    setValue(processor, ParameterIDs::output, -6.0f);
    const auto reduced = measureSine(processor, 1000.0f);
    check(reduced < nominal * 0.53f && reduced > nominal * 0.48f, "Output gain follows the decibel control");
}

void testSoftClipper()
{
    BlueSteelEQAudioProcessor processor;
    processor.applyFactoryPreset(0);
    setValue(processor, ParameterIDs::input, 12.0f);
    setValue(processor, ParameterIDs::clipperEnabled, 1.0f);
    setValue(processor, ParameterIDs::clipLevel, -6.0f);

    const auto clippedPeak = measureSine(processor, 1000.0f, true);
    check(clippedPeak < 0.53f && clippedPeak > 0.46f, "The soft clipper respects its -6 dB ceiling");

    BlueSteelEQAudioProcessor cleanProcessor;
    cleanProcessor.applyFactoryPreset(0);
    setValue(cleanProcessor, ParameterIDs::input, 12.0f);
    const auto cleanPeak = measureSine(cleanProcessor, 1000.0f, true);
    const auto expectedLinearPeak = 0.2f * juce::Decibels::decibelsToGain(12.0f);
    check(
        std::abs(cleanPeak - expectedLinearPeak) < 0.01f,
        "Disabling the clipper preserves the full linear input gain");
}

void testBypass()
{
    BlueSteelEQAudioProcessor processor;
    processor.applyFactoryPreset(0);
    setValue(processor, ParameterIDs::input, 12.0f);
    setValue(processor, ParameterIDs::lowBoost, 10.0f);
    setValue(processor, ParameterIDs::bypass, 1.0f);

    const auto peak = measureSine(processor, 40.0f, true);
    check(std::abs(peak - 0.2f) < 0.005f, "Bypass preserves input and skips gain and EQ");
}

void testPresetsAndSavedState()
{
    BlueSteelEQAudioProcessor processor;
    check(
        processor.getNumPrograms() == 13 && processor.getProgramName(2) == "Vocal Focus",
        "All 13 named factory presets are exposed to the host");
    check(
        processor.getProgramName(6) == "Galaxy Tape",
        "The Galaxy Tape preset is available to FL Studio and other hosts");
    check(
        processor.getProgramName(7) == "SSL Bus EQ"
            && processor.getProgramName(8) == "SSL Vocal Lift"
            && processor.getProgramName(9) == "SSL Drum Punch"
            && processor.getProgramName(10) == "SSL Mix Bus"
            && processor.getProgramName(11) == "Afro House Master"
            && processor.getProgramName(12) == "Tape Vocal Warmth",
        "The SSL-style, Afro house and tape-inspired presets all load by name");

    processor.setCurrentProgram(2);
    check(
        processor.getCurrentProgram() == 2,
        "The host receives the index of the selected factory preset");
    const auto savedState = processor.copyCurrentState();
    const auto savedBoost = processor.getParameterValue(ParameterIDs::midPeakGain);
    processor.setParameterValue(ParameterIDs::midPeakGain, 8.0f);
    processor.setUiScale(1.5f);
    processor.restoreState(savedState);
    check(
        std::abs(processor.getParameterValue(ParameterIDs::midPeakGain) - savedBoost) < 0.02f,
        "A saved plugin state restores all parameter values");
    check(
        std::abs(processor.getUiScale() - 1.0f) < 0.01f,
        "Restoring a plug-in state restores its default 100% user-interface scale");

    processor.setUiScale(1.25f);
    juce::MemoryBlock state;
    processor.getStateInformation(state);
    processor.setParameterValue(ParameterIDs::output, 9.0f);
    processor.setUiScale(0.8f);
    processor.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    check(
        std::abs(processor.getParameterValue(ParameterIDs::output)) < 0.02f,
        "Host save/restore serializes the full plugin state");
    check(
        std::abs(processor.getUiScale() - 1.25f) < 0.01f,
        "Host save/restore preserves the selected custom window scale");
    check(
        processor.getCurrentProgram() == 2,
        "Host save/restore preserves the active factory-preset name");
}
}

int main()
{
    testNeutralResponse();
    testLowShelf();
    testMidBands();
    testHighBands();
    testCutFilters();
    testInputAndOutputGain();
    testSoftClipper();
    testBypass();
    testPresetsAndSavedState();

    if (failedTests != 0)
    {
        std::cerr << failedTests << " DSP test(s) failed.\n";
        return 1;
    }

    std::cout << "All DSP tests passed.\n";
    return 0;
}
