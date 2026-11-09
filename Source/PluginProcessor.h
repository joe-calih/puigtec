#pragma once

#include <JuceHeader.h>

#include <array>
#include <atomic>
#include <memory>

namespace ParameterIDs
{
inline constexpr const char* input = "input";
inline constexpr const char* output = "output";
inline constexpr const char* lowFrequency = "lowFrequency";
inline constexpr const char* lowBoost = "lowBoost";
inline constexpr const char* lowAttenuation = "lowAttenuation";
inline constexpr const char* lowAttenuationFrequency = "lowAttenuationFrequency";
inline constexpr const char* midPeakFrequency = "midPeakFrequency";
inline constexpr const char* midPeakGain = "midPeakGain";
inline constexpr const char* midPeakWidth = "midPeakWidth";
inline constexpr const char* midDipFrequency = "midDipFrequency";
inline constexpr const char* midDipGain = "midDipGain";
inline constexpr const char* midDipWidth = "midDipWidth";
inline constexpr const char* highFrequency = "highFrequency";
inline constexpr const char* highBoost = "highBoost";
inline constexpr const char* highBandwidth = "highBandwidth";
inline constexpr const char* highAttenuationFrequency = "highAttenuationFrequency";
inline constexpr const char* highAttenuation = "highAttenuation";
inline constexpr const char* lowCut = "lowCut";
inline constexpr const char* highCut = "highCut";
inline constexpr const char* clipperEnabled = "clipperEnabled";
inline constexpr const char* clipLevel = "clipLevel";
inline constexpr const char* bypass = "bypass";
inline constexpr std::size_t count = 22;
}

class BlueSteelEQAudioProcessor final : public juce::AudioProcessor
{
public:
    struct FactoryPreset
    {
        juce::String name;
        std::array<float, ParameterIDs::count> values;
    };

    BlueSteelEQAudioProcessor();
    ~BlueSteelEQAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    void processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destinationData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;
    juce::UndoManager undoManager;

    float getParameterValue(const juce::String& id) const noexcept;
    float getFrequencyResponse(float frequency) const noexcept;
    float getInputPeak() const noexcept;
    float getOutputPeak() const noexcept;
    float getUiScale() const noexcept;

    void setParameterValue(const juce::String& id, float value);
    void setUiScale(float scale) noexcept;
    juce::ValueTree copyCurrentState() const;
    void restoreState(const juce::ValueTree& state);
    void refreshFrequencyResponse() const;

    static const std::array<FactoryPreset, 13>& getFactoryPresets();
    void applyFactoryPreset(int index);

private:
    using IIRFilter = juce::dsp::IIR::Filter<float>;
    using FilterPair = std::array<IIRFilter, 2>;
    using IIRCoefficients = juce::dsp::IIR::Coefficients<float>;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static juce::StringArray choicesForParameter(const juce::String& id);
    static float choiceFrequency(const juce::String& id, int choiceIndex);
    static int choiceIndex(const juce::String& id, float parameterValue);
    static void setCoefficients(
        FilterPair& filters,
        const IIRCoefficients::Ptr& coefficients);

    std::array<IIRCoefficients::Ptr, 8> makeFilterCoefficients() const;
    void publishFrequencyResponse(
        const std::array<IIRCoefficients::Ptr, 8>& coefficients) const;
    void updateFilters();
    void prepareFilters(double sampleRate, int maximumExpectedSamplesPerBlock);

    double currentSampleRate = 44100.0;
    std::array<FilterPair, 8> filters;
    std::array<float, ParameterIDs::count> lastFilterParameters{};
    mutable std::array<std::atomic<float>, 128> responseCurve{};
    bool filtersInitialized = false;
    std::atomic<int> currentProgram{0};
    std::atomic<float> inputPeak{0.0f};
    std::atomic<float> outputPeak{0.0f};
    std::atomic<float> uiScale{1.0f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlueSteelEQAudioProcessor)
};
