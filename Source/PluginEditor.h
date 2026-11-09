#pragma once

#include "PluginProcessor.h"

#include <memory>
#include <unordered_map>
#include <vector>

class BlueSteelLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    BlueSteelLookAndFeel();

    void drawRotarySlider(
        juce::Graphics& graphics,
        int x,
        int y,
        int width,
        int height,
        float sliderPosition,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override;

    void drawButtonBackground(
        juce::Graphics& graphics,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool isMouseOverButton,
        bool isButtonDown) override;

    void drawToggleButton(
        juce::Graphics& graphics,
        juce::ToggleButton& button,
        bool isMouseOverButton,
        bool isButtonDown) override;

    void drawComboBox(
        juce::Graphics& graphics,
        int width,
        int height,
        bool isButtonDown,
        int buttonX,
        int buttonY,
        int buttonWidth,
        int buttonHeight,
        juce::ComboBox& comboBox) override;

    void drawPopupMenuBackground(
        juce::Graphics& graphics,
        int width,
        int height) override;
};

class BlueSteelEQAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit BlueSteelEQAudioProcessorEditor(BlueSteelEQAudioProcessor& processor);
    ~BlueSteelEQAudioProcessorEditor() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    class ParameterControl;
    class ResponseDisplay;

    void addParameterControl(const juce::String& id, const juce::String& name);
    void layoutControl(
        const juce::String& id,
        const juce::Rectangle<int>& bounds);
    void choosePreset(int index);
    void selectSnapshot(bool useA);
    void storeSnapshot();
    void showAboutMenu();
    void setEditorScale(float scale);
    juce::Rectangle<int> scaledBounds(const juce::Rectangle<int>& bounds) const;
    void updateUndoButtons();
    void timerCallback() override;

    BlueSteelEQAudioProcessor& audioProcessor;
    BlueSteelLookAndFeel blueSteelLookAndFeel;
    std::vector<std::unique_ptr<ParameterControl>> parameterControls;
    std::unordered_map<std::string, ParameterControl*> controlsById;

    std::unique_ptr<ResponseDisplay> responseDisplay;
    juce::ComboBox presetSelector;
    juce::ToggleButton powerButton{"POWER"};
    juce::ToggleButton clipperButton{"SOFT CLIP"};
    juce::TextButton snapshotAButton{"A"};
    juce::TextButton snapshotBButton{"B"};
    juce::TextButton storeButton{"STORE"};
    juce::TextButton aboutButton{"ABOUT"};
    juce::TextButton undoButton{"UNDO"};
    juce::TextButton redoButton{"REDO"};
    juce::ComboBox sizeSelector;

    juce::ValueTree snapshotA;
    juce::ValueTree snapshotB;
    bool activeSnapshotA = true;
    float editorScale = 1.0f;
    float horizontalScale = 1.0f;
    float verticalScale = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlueSteelEQAudioProcessorEditor)
};
