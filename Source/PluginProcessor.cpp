#include "PluginProcessor.h"

#include "PluginEditor.h"

#include <algorithm>
#include <cmath>
#include <initializer_list>

namespace
{
constexpr float parameterStep = 0.01f;

using PresetValues = std::array<float, ParameterIDs::count>;

constexpr PresetValues cleanPreset{
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    5.0f, 0.0f, 1.0f, 3.0f, 0.0f, 1.0f,
    4.0f, 0.0f, 1.0f, 2.0f, 0.0f,
    0.0f, 8.0f, 0.0f, -3.0f, 0.0f
};

const std::array<BlueSteelEQAudioProcessor::FactoryPreset, 13> factoryPresets{{
    {"Initial Preset", cleanPreset},
    {"Warm Low End", {
        0.0f, 0.0f, 0.0f, 3.4f, 0.7f, 2.0f,
        3.0f, 0.4f, 0.9f, 3.0f, 0.0f, 1.0f,
        3.0f, 0.3f, 1.0f, 2.0f, 0.3f,
        1.0f, 8.0f, 0.0f, -4.0f, 0.0f
    }},
    {"Vocal Focus", {
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        7.0f, 1.7f, 1.1f, 4.0f, 1.0f, 1.0f,
        5.0f, 0.9f, 0.9f, 2.0f, 0.3f,
        1.0f, 8.0f, 0.0f, -4.0f, 0.0f
    }},
    {"Air and Clarity", {
        0.0f, 0.0f, 1.0f, 0.3f, 0.0f, 1.0f,
        7.0f, 0.3f, 0.9f, 4.0f, 0.4f, 1.0f,
        7.0f, 2.0f, 0.7f, 4.0f, 0.1f,
        1.0f, 8.0f, 0.0f, -4.0f, 0.0f
    }},
    {"Smooth Mix Bus", {
        1.0f, -1.0f, 1.0f, 1.0f, 0.5f, 2.0f,
        5.0f, 0.5f, 0.8f, 4.0f, 0.6f, 1.0f,
        5.0f, 0.6f, 0.8f, 2.0f, 0.5f,
        1.0f, 8.0f, 0.0f, -3.0f, 0.0f
    }},
    {"Saturated Presence", {
        4.0f, -3.0f, 1.0f, 1.1f, 0.0f, 2.0f,
        6.0f, 0.6f, 1.2f, 4.0f, 0.8f, 1.1f,
        6.0f, 1.0f, 0.7f, 2.0f, 0.2f,
        1.0f, 8.0f, 1.0f, -8.0f, 0.0f
    }},
    {"Galaxy Tape", {
        3.0f, -2.0f, 1.0f, 1.3f, 0.8f, 2.0f,
        4.0f, 0.8f, 0.8f, 3.0f, 0.7f, 0.9f,
        4.0f, 1.1f, 0.8f, 2.0f, 0.6f,
        1.0f, 8.0f, 1.0f, -5.0f, 0.0f
    }},
    {"SSL Bus EQ", {
        2.0f, -2.0f, 1.0f, 1.4f, 0.5f, 2.0f,
        4.0f, 0.5f, 0.7f, 3.0f, 0.4f, 0.9f,
        5.0f, 0.5f, 1.0f, 2.0f, 0.3f,
        1.0f, 8.0f, 0.0f, -6.0f, 0.0f
    }},
    {"SSL Vocal Lift", {
        0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 1.0f,
        5.0f, 1.6f, 1.0f, 4.0f, 1.0f, 1.0f,
        5.0f, 1.4f, 0.9f, 2.0f, 0.3f,
        1.0f, 8.0f, 0.0f, -4.0f, 0.0f
    }},
    {"SSL Drum Punch", {
        3.0f, -1.0f, 1.0f, 2.0f, 0.6f, 2.0f,
        3.0f, 1.2f, 0.7f, 4.0f, 0.3f, 0.9f,
        5.0f, 0.8f, 0.9f, 2.0f, 0.3f,
        1.0f, 8.0f, 1.0f, -6.0f, 0.0f
    }},
    {"SSL Mix Bus", {
        1.0f, -1.0f, 1.0f, 1.0f, 0.4f, 2.0f,
        4.0f, 0.6f, 0.8f, 3.0f, 0.5f, 0.9f,
        5.0f, 0.7f, 0.9f, 2.0f, 0.4f,
        1.0f, 8.0f, 0.0f, -5.0f, 0.0f
    }},
    {"Afro House Master", {
        1.0f, -1.0f, 1.0f, 1.0f, 0.5f, 2.0f,
        4.0f, 0.5f, 0.8f, 3.0f, 0.4f, 0.9f,
        5.0f, 1.0f, 0.8f, 3.0f, 0.3f,
        1.0f, 8.0f, 0.0f, -4.0f, 0.0f
    }},
    {"Tape Vocal Warmth", {
        2.0f, -1.0f, 1.0f, 1.8f, 0.8f, 2.0f,
        4.0f, 0.8f, 1.0f, 3.0f, 0.5f, 0.9f,
        5.0f, 0.9f, 0.9f, 2.0f, 0.4f,
        1.0f, 8.0f, 1.0f, -7.0f, 0.0f
    }}
}};

const std::array<const char*, ParameterIDs::count> orderedParameterIDs{{
    ParameterIDs::input,
    ParameterIDs::output,
    ParameterIDs::lowFrequency,
    ParameterIDs::lowBoost,
    ParameterIDs::lowAttenuation,
    ParameterIDs::lowAttenuationFrequency,
    ParameterIDs::midPeakFrequency,
    ParameterIDs::midPeakGain,
    ParameterIDs::midPeakWidth,
    ParameterIDs::midDipFrequency,
    ParameterIDs::midDipGain,
    ParameterIDs::midDipWidth,
    ParameterIDs::highFrequency,
    ParameterIDs::highBoost,
    ParameterIDs::highBandwidth,
    ParameterIDs::highAttenuationFrequency,
    ParameterIDs::highAttenuation,
    ParameterIDs::lowCut,
    ParameterIDs::highCut,
    ParameterIDs::clipperEnabled,
    ParameterIDs::clipLevel,
    ParameterIDs::bypass
}};

void addFloatParameter(
    juce::AudioProcessorValueTreeState::ParameterLayout& layout,
    const juce::String& id,
    const juce::String& name,
    float minimum,
    float maximum,
    float defaultValue,
    float step = parameterStep)
{
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{id, 1},
        name,
        juce::NormalisableRange<float>{minimum, maximum, step},
        defaultValue));
}

void addChoiceParameter(
    juce::AudioProcessorValueTreeState::ParameterLayout& layout,
    const juce::String& id,
    const juce::String& name,
    const juce::StringArray& choices,
    int defaultIndex)
{
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{id, 1}, name, choices, defaultIndex));
}
}

BlueSteelEQAudioProcessor::BlueSteelEQAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, &undoManager, "PARAMETERS", createParameterLayout())
{
    prepareFilters(currentSampleRate, 512);
}

juce::AudioProcessorValueTreeState::ParameterLayout
BlueSteelEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    addFloatParameter(layout, ParameterIDs::input, "Input", -24.0f, 24.0f, 0.0f);
    addFloatParameter(layout, ParameterIDs::output, "Output", -24.0f, 24.0f, 0.0f);
    addChoiceParameter(
        layout, ParameterIDs::lowFrequency, "Low boost frequency",
        juce::StringArray{"30 Hz", "60 Hz", "100 Hz", "150 Hz", "200 Hz"}, 1);
    addFloatParameter(layout, ParameterIDs::lowBoost, "Low boost", 0.0f, 10.0f, 0.0f);
    addFloatParameter(layout, ParameterIDs::lowAttenuation, "Low attenuation", 0.0f, 10.0f, 0.0f);
    addChoiceParameter(
        layout, ParameterIDs::lowAttenuationFrequency, "Low attenuation frequency",
        juce::StringArray{"30 Hz", "60 Hz", "100 Hz", "150 Hz", "200 Hz"}, 1);

    addChoiceParameter(
        layout, ParameterIDs::midPeakFrequency, "Mid boost frequency",
        juce::StringArray{"200 Hz", "300 Hz", "400 Hz", "500 Hz", "700 Hz", "1 kHz",
                          "1.5 kHz", "2 kHz", "3 kHz", "5 kHz", "7 kHz"}, 5);
    addFloatParameter(layout, ParameterIDs::midPeakGain, "Mid boost", 0.0f, 10.0f, 0.0f);
    addFloatParameter(layout, ParameterIDs::midPeakWidth, "Mid boost Q", 0.5f, 2.5f, 1.0f);
    addChoiceParameter(
        layout, ParameterIDs::midDipFrequency, "Mid dip frequency",
        juce::StringArray{"1 kHz", "1.5 kHz", "2 kHz", "3 kHz", "4 kHz", "5 kHz",
                          "7 kHz"}, 3);
    addFloatParameter(layout, ParameterIDs::midDipGain, "Mid attenuation", 0.0f, 10.0f, 0.0f);
    addFloatParameter(layout, ParameterIDs::midDipWidth, "Mid attenuation Q", 0.5f, 2.5f, 1.0f);

    addChoiceParameter(
        layout, ParameterIDs::highFrequency, "High boost frequency",
        juce::StringArray{"3 kHz", "4 kHz", "5 kHz", "6 kHz", "8 kHz", "10 kHz",
                          "12 kHz", "16 kHz"}, 4);
    addFloatParameter(layout, ParameterIDs::highBoost, "High boost", 0.0f, 10.0f, 0.0f);
    addFloatParameter(layout, ParameterIDs::highBandwidth, "High boost bandwidth", 0.5f, 2.5f, 1.0f);
    addChoiceParameter(
        layout, ParameterIDs::highAttenuationFrequency, "High attenuation frequency",
        juce::StringArray{"5 kHz", "8 kHz", "10 kHz", "12 kHz", "16 kHz", "20 kHz"}, 2);
    addFloatParameter(layout, ParameterIDs::highAttenuation, "High attenuation", 0.0f, 10.0f, 0.0f);

    addChoiceParameter(
        layout, ParameterIDs::lowCut, "Low cut",
        juce::StringArray{"Off", "30 Hz", "50 Hz", "80 Hz", "100 Hz", "150 Hz", "200 Hz"}, 0);
    addChoiceParameter(
        layout, ParameterIDs::highCut, "High cut",
        juce::StringArray{"500 Hz", "1 kHz", "2 kHz", "4 kHz", "6 kHz", "8 kHz",
                          "12 kHz", "16 kHz", "Off"}, 8);

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParameterIDs::clipperEnabled, 1}, "Soft clipper", false));
    addFloatParameter(layout, ParameterIDs::clipLevel, "Clip level", -12.0f, 0.0f, -3.0f);
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParameterIDs::bypass, 1}, "Bypass", false));

    jassert(orderedParameterIDs.size() == ParameterIDs::count);
    return layout;
}

juce::StringArray BlueSteelEQAudioProcessor::choicesForParameter(const juce::String& id)
{
    if (id == ParameterIDs::lowFrequency || id == ParameterIDs::lowAttenuationFrequency)
        return {"30 Hz", "60 Hz", "100 Hz", "150 Hz", "200 Hz"};

    if (id == ParameterIDs::midPeakFrequency)
        return {"200 Hz", "300 Hz", "400 Hz", "500 Hz", "700 Hz", "1 kHz",
                "1.5 kHz", "2 kHz", "3 kHz", "5 kHz", "7 kHz"};

    if (id == ParameterIDs::midDipFrequency)
        return {"1 kHz", "1.5 kHz", "2 kHz", "3 kHz", "4 kHz", "5 kHz", "7 kHz"};

    if (id == ParameterIDs::highFrequency)
        return {"3 kHz", "4 kHz", "5 kHz", "6 kHz", "8 kHz", "10 kHz",
                "12 kHz", "16 kHz"};

    if (id == ParameterIDs::highAttenuationFrequency)
        return {"5 kHz", "8 kHz", "10 kHz", "12 kHz", "16 kHz", "20 kHz"};

    if (id == ParameterIDs::lowCut)
        return {"Off", "30 Hz", "50 Hz", "80 Hz", "100 Hz", "150 Hz", "200 Hz"};

    if (id == ParameterIDs::highCut)
        return {"500 Hz", "1 kHz", "2 kHz", "4 kHz", "6 kHz", "8 kHz",
                "12 kHz", "16 kHz", "Off"};

    jassertfalse;
    return {};
}

float BlueSteelEQAudioProcessor::choiceFrequency(const juce::String& id, int choiceIndexValue)
{
    if (id == ParameterIDs::lowFrequency || id == ParameterIDs::lowAttenuationFrequency)
    {
        constexpr std::array<float, 5> choices{30.0f, 60.0f, 100.0f, 150.0f, 200.0f};
        return choices[static_cast<std::size_t>(juce::jlimit(0, 4, choiceIndexValue))];
    }

    if (id == ParameterIDs::midPeakFrequency)
    {
        constexpr std::array<float, 11> choices{
            200.0f, 300.0f, 400.0f, 500.0f, 700.0f, 1000.0f,
            1500.0f, 2000.0f, 3000.0f, 5000.0f, 7000.0f
        };
        return choices[static_cast<std::size_t>(juce::jlimit(0, 10, choiceIndexValue))];
    }

    if (id == ParameterIDs::midDipFrequency)
    {
        constexpr std::array<float, 7> choices{
            1000.0f, 1500.0f, 2000.0f, 3000.0f, 4000.0f, 5000.0f, 7000.0f
        };
        return choices[static_cast<std::size_t>(juce::jlimit(0, 6, choiceIndexValue))];
    }

    if (id == ParameterIDs::highFrequency)
    {
        constexpr std::array<float, 8> choices{
            3000.0f, 4000.0f, 5000.0f, 6000.0f, 8000.0f, 10000.0f,
            12000.0f, 16000.0f
        };
        return choices[static_cast<std::size_t>(juce::jlimit(0, 7, choiceIndexValue))];
    }

    if (id == ParameterIDs::highAttenuationFrequency)
    {
        constexpr std::array<float, 6> choices{
            5000.0f, 8000.0f, 10000.0f, 12000.0f, 16000.0f, 20000.0f
        };
        return choices[static_cast<std::size_t>(juce::jlimit(0, 5, choiceIndexValue))];
    }

    if (id == ParameterIDs::lowCut)
    {
        constexpr std::array<float, 7> choices{
            20.0f, 30.0f, 50.0f, 80.0f, 100.0f, 150.0f, 200.0f
        };
        return choices[static_cast<std::size_t>(juce::jlimit(0, 6, choiceIndexValue))];
    }

    if (id == ParameterIDs::highCut)
    {
        constexpr std::array<float, 9> choices{
            500.0f, 1000.0f, 2000.0f, 4000.0f, 6000.0f, 8000.0f,
            12000.0f, 16000.0f, 20000.0f
        };
        return choices[static_cast<std::size_t>(juce::jlimit(0, 8, choiceIndexValue))];
    }

    jassertfalse;
    return 1000.0f;
}

int BlueSteelEQAudioProcessor::choiceIndex(const juce::String& id, float parameterValue)
{
    const auto choices = choicesForParameter(id);
    jassert(!choices.isEmpty());
    return juce::jlimit(0, choices.size() - 1, juce::roundToInt(parameterValue));
}

const std::array<BlueSteelEQAudioProcessor::FactoryPreset, 13>&
BlueSteelEQAudioProcessor::getFactoryPresets()
{
    return factoryPresets;
}

void BlueSteelEQAudioProcessor::applyFactoryPreset(int index)
{
    if (!juce::isPositiveAndBelow(index, static_cast<int>(factoryPresets.size())))
        return;

    currentProgram.store(index, std::memory_order_relaxed);
    const auto& preset = factoryPresets[static_cast<std::size_t>(index)];
    for (std::size_t parameterIndex = 0; parameterIndex < orderedParameterIDs.size(); ++parameterIndex)
        setParameterValue(
            orderedParameterIDs[parameterIndex],
            preset.values[parameterIndex]);

    refreshFrequencyResponse();
}

void BlueSteelEQAudioProcessor::setParameterValue(const juce::String& id, float value)
{
    auto* parameter = parameters.getParameter(id);
    jassert(parameter != nullptr);
    if (parameter == nullptr)
        return;

    undoManager.beginNewTransaction();
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    parameter->endChangeGesture();
}

float BlueSteelEQAudioProcessor::getUiScale() const noexcept
{
    return uiScale.load(std::memory_order_relaxed);
}

void BlueSteelEQAudioProcessor::setUiScale(float scale) noexcept
{
    uiScale.store(juce::jlimit(0.7f, 2.0f, scale), std::memory_order_relaxed);
}

juce::ValueTree BlueSteelEQAudioProcessor::copyCurrentState()
{
    auto state = parameters.copyState();
    state.setProperty("uiScale", getUiScale(), nullptr);
    state.setProperty("factoryProgram", currentProgram.load(std::memory_order_relaxed), nullptr);
    return state;
}

void BlueSteelEQAudioProcessor::restoreState(const juce::ValueTree& state)
{
    if (state.hasType(parameters.state.getType()))
    {
        if (state.hasProperty("uiScale"))
            setUiScale(static_cast<float>(state.getProperty("uiScale")));
        if (state.hasProperty("factoryProgram"))
        {
            const auto savedProgram = static_cast<int>(state.getProperty("factoryProgram"));
            if (juce::isPositiveAndBelow(savedProgram, static_cast<int>(factoryPresets.size())))
                currentProgram.store(savedProgram, std::memory_order_relaxed);
        }
        parameters.replaceState(state.createCopy());
    }
}

void BlueSteelEQAudioProcessor::prepareFilters(
    double sampleRate,
    int maximumExpectedSamplesPerBlock)
{
    currentSampleRate = sampleRate;
    filtersInitialized = false;
    const juce::dsp::ProcessSpec specification{
        sampleRate,
        static_cast<juce::uint32>(juce::jmax(1, maximumExpectedSamplesPerBlock)),
        1
    };

    for (auto& pair : filters)
    {
        for (auto& channelFilter : pair)
        {
            channelFilter.prepare(specification);
            channelFilter.reset();
        }
    }

    updateFilters();
}

void BlueSteelEQAudioProcessor::prepareToPlay(
    double sampleRate,
    int maximumExpectedSamplesPerBlock)
{
    prepareFilters(sampleRate, maximumExpectedSamplesPerBlock);
    inputPeak.store(0.0f, std::memory_order_relaxed);
    outputPeak.store(0.0f, std::memory_order_relaxed);
}

void BlueSteelEQAudioProcessor::releaseResources()
{
    reset();
}

void BlueSteelEQAudioProcessor::reset()
{
    for (auto& pair : filters)
        for (auto& channelFilter : pair)
            channelFilter.reset();

    inputPeak.store(0.0f, std::memory_order_relaxed);
    outputPeak.store(0.0f, std::memory_order_relaxed);
}

bool BlueSteelEQAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainInput = layouts.getMainInputChannelSet();
    const auto mainOutput = layouts.getMainOutputChannelSet();
    const auto isSupportedChannelSet = [](const juce::AudioChannelSet& channelSet)
    {
        return channelSet == juce::AudioChannelSet::mono()
            || channelSet == juce::AudioChannelSet::stereo();
    };

    return isSupportedChannelSet(mainInput) && mainInput == mainOutput;
}

void BlueSteelEQAudioProcessor::setCoefficients(
    FilterPair& channelFilters,
    const IIRCoefficients::Ptr& coefficients)
{
    for (auto& channelFilter : channelFilters)
        channelFilter.coefficients = coefficients;
}

std::array<BlueSteelEQAudioProcessor::IIRCoefficients::Ptr, 8>
BlueSteelEQAudioProcessor::makeFilterCoefficients() const
{
    const auto sampleRate = juce::jmax(1000.0, currentSampleRate);
    const auto maxFrequency = static_cast<float>(sampleRate * 0.45);
    const auto clampedFrequency = [maxFrequency](float frequency)
    {
        return juce::jlimit(20.0f, maxFrequency, frequency);
    };

    const auto lowFrequency = clampedFrequency(choiceFrequency(
        ParameterIDs::lowFrequency,
        choiceIndex(ParameterIDs::lowFrequency, getParameterValue(ParameterIDs::lowFrequency))));
    const auto lowAttenuationFrequency = clampedFrequency(choiceFrequency(
        ParameterIDs::lowAttenuationFrequency,
        choiceIndex(
            ParameterIDs::lowAttenuationFrequency,
            getParameterValue(ParameterIDs::lowAttenuationFrequency))));
    const auto midPeakFrequency = clampedFrequency(choiceFrequency(
        ParameterIDs::midPeakFrequency,
        choiceIndex(ParameterIDs::midPeakFrequency, getParameterValue(ParameterIDs::midPeakFrequency))));
    const auto midDipFrequency = clampedFrequency(choiceFrequency(
        ParameterIDs::midDipFrequency,
        choiceIndex(ParameterIDs::midDipFrequency, getParameterValue(ParameterIDs::midDipFrequency))));
    const auto highFrequency = clampedFrequency(choiceFrequency(
        ParameterIDs::highFrequency,
        choiceIndex(ParameterIDs::highFrequency, getParameterValue(ParameterIDs::highFrequency))));
    const auto highAttenuationFrequency = clampedFrequency(choiceFrequency(
        ParameterIDs::highAttenuationFrequency,
        choiceIndex(
            ParameterIDs::highAttenuationFrequency,
            getParameterValue(ParameterIDs::highAttenuationFrequency))));
    const auto lowCutFrequency = choiceFrequency(
        ParameterIDs::lowCut,
        choiceIndex(ParameterIDs::lowCut, getParameterValue(ParameterIDs::lowCut)));
    const auto highCutFrequency = choiceFrequency(
        ParameterIDs::highCut,
        choiceIndex(ParameterIDs::highCut, getParameterValue(ParameterIDs::highCut)));

    return {{
        lowCutFrequency > 20.0f
            ? IIRCoefficients::makeHighPass(
                sampleRate, clampedFrequency(lowCutFrequency), 0.70710678)
            : IIRCoefficients::makePeakFilter(
                sampleRate, 1000.0f, 0.70710678, 1.0f),
        IIRCoefficients::makeLowShelf(
            sampleRate,
            lowFrequency,
            0.70710678,
            juce::Decibels::decibelsToGain(getParameterValue(ParameterIDs::lowBoost) * 1.2f)),
        IIRCoefficients::makeLowShelf(
            sampleRate,
            clampedFrequency(lowAttenuationFrequency * 1.35f),
            0.70710678,
            juce::Decibels::decibelsToGain(
                -getParameterValue(ParameterIDs::lowAttenuation) * 1.2f)),
        IIRCoefficients::makePeakFilter(
            sampleRate,
            midPeakFrequency,
            getParameterValue(ParameterIDs::midPeakWidth),
            juce::Decibels::decibelsToGain(getParameterValue(ParameterIDs::midPeakGain) * 1.2f)),
        IIRCoefficients::makePeakFilter(
            sampleRate,
            midDipFrequency,
            getParameterValue(ParameterIDs::midDipWidth),
            juce::Decibels::decibelsToGain(-getParameterValue(ParameterIDs::midDipGain) * 1.2f)),
        IIRCoefficients::makePeakFilter(
            sampleRate,
            highFrequency,
            getParameterValue(ParameterIDs::highBandwidth),
            juce::Decibels::decibelsToGain(getParameterValue(ParameterIDs::highBoost) * 1.2f)),
        IIRCoefficients::makeHighShelf(
            sampleRate,
            highAttenuationFrequency,
            0.70710678,
            juce::Decibels::decibelsToGain(
                -getParameterValue(ParameterIDs::highAttenuation) * 1.2f)),
        highCutFrequency < 20000.0f
            ? IIRCoefficients::makeLowPass(
                sampleRate, clampedFrequency(highCutFrequency), 0.70710678)
            : IIRCoefficients::makePeakFilter(
                sampleRate, 1000.0f, 0.70710678, 1.0f)
    }};
}

void BlueSteelEQAudioProcessor::publishFrequencyResponse(
    const std::array<IIRCoefficients::Ptr, 8>& coefficients) const
{
    const auto sampleRate = juce::jmax(1000.0, currentSampleRate);

    for (std::size_t point = 0; point < responseCurve.size(); ++point)
    {
        const auto proportion = static_cast<double>(point)
            / static_cast<double>(responseCurve.size() - 1);
        const auto frequency = 20.0 * std::pow(1000.0, proportion);
        auto magnitude = 1.0;
        for (const auto& coefficient : coefficients)
            magnitude *= coefficient->getMagnitudeForFrequency(frequency, sampleRate);

        responseCurve[point].store(
            static_cast<float>(juce::Decibels::gainToDecibels(magnitude, -120.0)),
            std::memory_order_relaxed);
    }
}

void BlueSteelEQAudioProcessor::refreshFrequencyResponse() const
{
    publishFrequencyResponse(makeFilterCoefficients());
}

void BlueSteelEQAudioProcessor::updateFilters()
{
    auto changed = !filtersInitialized;
    for (std::size_t parameterIndex = 2; parameterIndex < 19; ++parameterIndex)
    {
        const auto value = getParameterValue(orderedParameterIDs[parameterIndex]);
        changed = changed || value != lastFilterParameters[parameterIndex];
    }

    if (!changed)
        return;

    const auto coefficients = makeFilterCoefficients();
    for (std::size_t filterIndex = 0; filterIndex < filters.size(); ++filterIndex)
        setCoefficients(filters[filterIndex], coefficients[filterIndex]);

    for (std::size_t parameterIndex = 2; parameterIndex < 19; ++parameterIndex)
        lastFilterParameters[parameterIndex] =
            getParameterValue(orderedParameterIDs[parameterIndex]);

    filtersInitialized = true;
    publishFrequencyResponse(coefficients);
}

void BlueSteelEQAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const auto numberOfChannels = juce::jmin(buffer.getNumChannels(), 2);
    const auto numberOfSamples = buffer.getNumSamples();
    const auto inputGain = juce::Decibels::decibelsToGain(
        getParameterValue(ParameterIDs::input));
    const auto outputGain = juce::Decibels::decibelsToGain(
        getParameterValue(ParameterIDs::output));
    const auto clipEnabled = getParameterValue(ParameterIDs::clipperEnabled) >= 0.5f;
    const auto clipThreshold = juce::Decibels::decibelsToGain(
        getParameterValue(ParameterIDs::clipLevel));
    const auto bypassed = getParameterValue(ParameterIDs::bypass) >= 0.5f;

    updateFilters();

    float blockInputPeak = 0.0f;
    float blockOutputPeak = 0.0f;

    for (int channel = 0; channel < numberOfChannels; ++channel)
    {
        auto* samples = buffer.getWritePointer(channel);

        for (int sampleIndex = 0; sampleIndex < numberOfSamples; ++sampleIndex)
        {
            auto sample = samples[sampleIndex];
            blockInputPeak = juce::jmax(blockInputPeak, std::abs(sample));

            if (!bypassed)
            {
                sample *= inputGain;

                for (auto& filterPair : filters)
                            sample = filterPair[static_cast<std::size_t>(channel)].processSample(sample);

                if (clipEnabled)
                    sample = clipThreshold * std::tanh(sample / clipThreshold);

                sample *= outputGain;
            }

            blockOutputPeak = juce::jmax(blockOutputPeak, std::abs(sample));
            samples[sampleIndex] = sample;
        }
    }

    const auto previousInputPeak = inputPeak.load(std::memory_order_relaxed) * 0.88f;
    const auto previousOutputPeak = outputPeak.load(std::memory_order_relaxed) * 0.88f;
    inputPeak.store(juce::jmax(blockInputPeak, previousInputPeak), std::memory_order_relaxed);
    outputPeak.store(juce::jmax(blockOutputPeak, previousOutputPeak), std::memory_order_relaxed);
}

float BlueSteelEQAudioProcessor::getParameterValue(const juce::String& id) const noexcept
{
    const auto* rawParameter = parameters.getRawParameterValue(id);
    jassert(rawParameter != nullptr);
    return rawParameter->load(std::memory_order_relaxed);
}

float BlueSteelEQAudioProcessor::getFrequencyResponse(float frequency) const noexcept
{
    constexpr auto minimumFrequency = 20.0f;
    constexpr auto maximumFrequency = 20000.0f;
    const auto boundedFrequency = juce::jlimit(minimumFrequency, maximumFrequency, frequency);
    const auto position = std::log(boundedFrequency / minimumFrequency)
        / std::log(maximumFrequency / minimumFrequency)
        * static_cast<float>(responseCurve.size() - 1);
    const auto leftIndex = juce::jlimit(
        0, static_cast<int>(responseCurve.size() - 1), static_cast<int>(position));
    const auto rightIndex = juce::jmin(leftIndex + 1, static_cast<int>(responseCurve.size() - 1));
    const auto proportion = position - static_cast<float>(leftIndex);
    const auto left = responseCurve[static_cast<std::size_t>(leftIndex)].load(
        std::memory_order_relaxed);
    const auto right = responseCurve[static_cast<std::size_t>(rightIndex)].load(
        std::memory_order_relaxed);
    return left + (right - left) * proportion;
}

float BlueSteelEQAudioProcessor::getInputPeak() const noexcept
{
    return inputPeak.load(std::memory_order_relaxed);
}

float BlueSteelEQAudioProcessor::getOutputPeak() const noexcept
{
    return outputPeak.load(std::memory_order_relaxed);
}

const juce::String BlueSteelEQAudioProcessor::getName() const
{
    return "Blue Steel EQ v2";
}

bool BlueSteelEQAudioProcessor::acceptsMidi() const
{
    return false;
}

bool BlueSteelEQAudioProcessor::producesMidi() const
{
    return false;
}

bool BlueSteelEQAudioProcessor::isMidiEffect() const
{
    return false;
}

double BlueSteelEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BlueSteelEQAudioProcessor::getNumPrograms()
{
    return static_cast<int>(factoryPresets.size());
}

int BlueSteelEQAudioProcessor::getCurrentProgram()
{
    return currentProgram.load(std::memory_order_relaxed);
}

void BlueSteelEQAudioProcessor::setCurrentProgram(int index)
{
    applyFactoryPreset(index);
}

const juce::String BlueSteelEQAudioProcessor::getProgramName(int index)
{
    if (!juce::isPositiveAndBelow(index, static_cast<int>(factoryPresets.size())))
        return {};

    return factoryPresets[static_cast<std::size_t>(index)].name;
}

void BlueSteelEQAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

bool BlueSteelEQAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* BlueSteelEQAudioProcessor::createEditor()
{
    return new BlueSteelEQAudioProcessorEditor(*this);
}

void BlueSteelEQAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData)
{
    if (const auto xml = copyCurrentState().createXml())
        copyXmlToBinary(*xml, destinationData);
}

void BlueSteelEQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes);
        xml != nullptr && xml->hasTagName(parameters.state.getType()))
    {
        auto state = juce::ValueTree::fromXml(*xml);
        restoreState(state);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BlueSteelEQAudioProcessor();
}
