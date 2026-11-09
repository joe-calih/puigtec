#include "PluginEditor.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace
{
namespace Colours
{
const juce::Colour midnight{0xff10171a};
const juce::Colour ink{0xff18262c};
const juce::Colour steelDark{0xff24485b};
const juce::Colour steel{0xff487991};
const juce::Colour steelLight{0xff78a0ae};
const juce::Colour ivory{0xffe6ece9};
const juce::Colour muted{0xffaac0c4};
const juce::Colour turquoise{0xff53e4d7};
const juce::Colour amber{0xffffbb58};
const juce::Colour red{0xffff6259};
}

constexpr auto windowWidth = 1180;
constexpr auto windowHeight = 668;

juce::Font makeFont(float size, bool bold = false)
{
    juce::Font font(size, bold ? juce::Font::bold : juce::Font::plain);
    font.setTypefaceName("Bahnschrift");
    return font;
}

float frequencyForResponseLabel(int index)
{
    static constexpr std::array<float, 13> frequencies{
        30.0f, 60.0f, 100.0f, 200.0f, 300.0f, 500.0f, 1000.0f,
        2000.0f, 3000.0f, 6000.0f, 10000.0f, 16000.0f, 20000.0f
    };
    return frequencies[static_cast<std::size_t>(
        juce::jlimit(0, static_cast<int>(frequencies.size()) - 1, index))];
}

juce::String formatFrequency(float frequency)
{
    if (frequency >= 1000.0f)
    {
        const auto kilohertz = frequency / 1000.0f;
        const auto decimals = frequency >= 10000.0f || frequency == 1000.0f ? 0 : 1;
        return juce::String(kilohertz, decimals) + "k";
    }

    return juce::String(juce::roundToInt(frequency));
}

void drawRivet(juce::Graphics& graphics, float centerX, float centerY)
{
    constexpr auto diameter = 11.0f;
    const auto bounds = juce::Rectangle<float>(
        centerX - diameter * 0.5f, centerY - diameter * 0.5f, diameter, diameter);

    graphics.setColour(Colours::midnight.withAlpha(0.55f));
    graphics.fillEllipse(bounds.translated(1.0f, 1.0f));

    graphics.setGradientFill(juce::ColourGradient(
        juce::Colour{0xffd5e0dc}, centerX - 2.0f, centerY - 4.0f,
        juce::Colour{0xff405762}, centerX + 3.0f, centerY + 4.0f,
        false));
    graphics.fillEllipse(bounds);

    graphics.setColour(juce::Colour{0xff213740});
    graphics.drawLine(centerX - 3.0f, centerY + 2.0f, centerX + 3.0f, centerY - 2.0f, 1.0f);
    graphics.setColour(juce::Colours::white.withAlpha(0.35f));
    graphics.drawEllipse(bounds, 0.8f);
}

juce::StringArray choicesForControl(const juce::String& id)
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

    return {};
}

void drawPanel(
    juce::Graphics& graphics,
    juce::Rectangle<float> bounds,
    const juce::String& title,
    const juce::Colour& accent)
{
    graphics.setColour(juce::Colours::black.withAlpha(0.28f));
    graphics.fillRoundedRectangle(bounds.translated(1.0f, 2.0f), 9.0f);

    graphics.setGradientFill(juce::ColourGradient(
        juce::Colour{0xff78a2ae}, bounds.getX(), bounds.getY(),
        juce::Colour{0xff53798c}, bounds.getRight(), bounds.getBottom(),
        false));
    graphics.fillRoundedRectangle(bounds, 9.0f);

    graphics.setColour(juce::Colours::white.withAlpha(0.32f));
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), 9.0f, 1.0f);

    const auto headingBounds = bounds.removeFromTop(25.0f);
    graphics.setColour(Colours::ink.withAlpha(0.72f));
    graphics.setFont(makeFont(10.0f, true));
    graphics.drawFittedText(
        title.toUpperCase(),
        headingBounds.toNearestInt().reduced(12, 0),
        juce::Justification::centredLeft,
        1);

    const auto lineY = headingBounds.getBottom() - 0.5f;
    graphics.setColour(accent.withAlpha(0.75f));
    graphics.drawLine(
        bounds.getX() + 11.0f, lineY, bounds.getRight() - 11.0f, lineY, 1.0f);

    drawRivet(graphics, bounds.getX() + 9.0f, bounds.getY() + 9.0f);
    drawRivet(graphics, bounds.getRight() - 9.0f, bounds.getY() + 9.0f);
}
}

BlueSteelLookAndFeel::BlueSteelLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, juce::Colour{0xff172329});
    setColour(juce::ComboBox::textColourId, Colours::ivory);
    setColour(juce::ComboBox::outlineColourId, Colours::steelLight.withAlpha(0.8f));
    setColour(juce::ComboBox::arrowColourId, Colours::turquoise);
    setColour(juce::PopupMenu::backgroundColourId, Colours::ink);
    setColour(juce::PopupMenu::textColourId, Colours::ivory);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Colours::steelDark);
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
}

void BlueSteelLookAndFeel::drawRotarySlider(
    juce::Graphics& graphics,
    int x,
    int y,
    int width,
    int height,
    float sliderPosition,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    juce::ignoreUnused(slider);

    const auto diameter = static_cast<float>(juce::jmin(width, height) - 12);
    const auto bounds = juce::Rectangle<float>(
        static_cast<float>(x) + (static_cast<float>(width) - diameter) * 0.5f,
        static_cast<float>(y) + (static_cast<float>(height) - diameter) * 0.5f,
        diameter,
        diameter);
    const auto center = bounds.getCentre();
    const auto radius = diameter * 0.5f;

    graphics.setColour(juce::Colours::black.withAlpha(0.45f));
    graphics.fillEllipse(bounds.translated(1.0f, 2.0f));

    graphics.setColour(Colours::midnight.withAlpha(0.75f));
    graphics.fillEllipse(bounds.expanded(3.0f));

    graphics.setGradientFill(juce::ColourGradient(
        juce::Colour{0xff66757b}, center.getX() - radius, center.getY() - radius,
        juce::Colour{0xff171b1d}, center.getX() + radius, center.getY() + radius,
        false));
    graphics.fillEllipse(bounds);
    graphics.setColour(juce::Colour{0xff849399}.withAlpha(0.65f));
    graphics.drawEllipse(bounds.reduced(1.0f), 1.0f);

    const auto tickRadius = radius + 1.0f;
    for (int tick = 0; tick <= 10; ++tick)
    {
        const auto proportion = static_cast<float>(tick) / 10.0f;
        const auto angle = rotaryStartAngle + proportion * (rotaryEndAngle - rotaryStartAngle);
        const auto innerRadius = tick % 5 == 0 ? tickRadius - 5.0f : tickRadius - 3.5f;
        const auto outerPoint = juce::Point<float>(
            center.getX() + tickRadius * std::cos(angle),
            center.getY() + tickRadius * std::sin(angle));
        const auto innerPoint = juce::Point<float>(
            center.getX() + innerRadius * std::cos(angle),
            center.getY() + innerRadius * std::sin(angle));

        graphics.setColour(
            tick % 5 == 0
                ? Colours::ivory.withAlpha(0.8f)
                : Colours::ink.withAlpha(0.55f));
        graphics.drawLine(
            innerPoint.getX(), innerPoint.getY(),
            outerPoint.getX(), outerPoint.getY(),
            tick % 5 == 0 ? 1.2f : 0.8f);
    }

    const auto pointerAngle = rotaryStartAngle
        + sliderPosition * (rotaryEndAngle - rotaryStartAngle);
    const auto indicatorDirection = juce::Point<float>(
        std::cos(pointerAngle), std::sin(pointerAngle));
    const auto indicatorOffset = juce::Point<float>(
        -indicatorDirection.y, indicatorDirection.x);
    const auto indicatorStart = center + indicatorDirection * (radius * 0.30f);
    const auto indicatorEnd = center + indicatorDirection * (radius * 0.77f);

    graphics.setColour(juce::Colours::black.withAlpha(0.88f));
    graphics.drawLine(
        indicatorStart + indicatorOffset * 1.0f,
        indicatorEnd + indicatorOffset * 1.0f,
        5.0f);
    graphics.setColour(juce::Colour{0xff171c1e});
    graphics.drawLine(indicatorStart, indicatorEnd, 3.5f);
    graphics.setColour(juce::Colour{0xff929994}.withAlpha(0.95f));
    graphics.drawLine(indicatorStart, indicatorEnd, 1.5f);
    graphics.setColour(Colours::ivory.withAlpha(0.95f));
    graphics.drawLine(
        indicatorStart - indicatorOffset * 0.7f,
        indicatorEnd - indicatorOffset * 0.7f,
        0.75f);
    graphics.setColour(Colours::steelLight.withAlpha(0.75f));
    graphics.drawLine(center, center + indicatorDirection * (radius * 0.27f), 1.0f);

    const auto hub = juce::Rectangle<float>(center.x - 7.0f, center.y - 7.0f, 14.0f, 14.0f);
    graphics.setGradientFill(juce::ColourGradient(
        juce::Colour{0xffc7d0cd}, center.getX() - 4.0f, center.getY() - 4.0f,
        juce::Colour{0xff353d40}, center.getX() + 5.0f, center.getY() + 5.0f,
        false));
    graphics.fillEllipse(hub);
    graphics.setColour(Colours::midnight.withAlpha(0.75f));
    graphics.fillEllipse(hub.reduced(3.0f));
    graphics.setColour(Colours::turquoise.withAlpha(0.9f));
    graphics.fillEllipse(juce::Rectangle<float>(center.x - 1.2f, center.y - 1.2f, 2.4f, 2.4f));
}

void BlueSteelLookAndFeel::drawButtonBackground(
    juce::Graphics& graphics,
    juce::Button& button,
    const juce::Colour& backgroundColour,
    bool isMouseOverButton,
    bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const auto base = button.getToggleState()
        ? Colours::steelDark
        : backgroundColour;
    const auto highlight = isButtonDown || button.getToggleState()
        ? Colours::turquoise.withAlpha(0.32f)
        : Colours::steelLight.withAlpha(isMouseOverButton ? 0.24f : 0.08f);

    graphics.setColour(juce::Colours::black.withAlpha(0.35f));
    graphics.fillRoundedRectangle(bounds.translated(0.0f, 1.0f), 6.0f);

    graphics.setGradientFill(juce::ColourGradient(
        base.brighter(isButtonDown ? 0.12f : 0.04f) + highlight,
        bounds.getX(), bounds.getY(),
        base.darker(0.26f), bounds.getX(), bounds.getBottom(),
        false));
    graphics.fillRoundedRectangle(bounds, 6.0f);

    graphics.setColour(Colours::ivory.withAlpha(isMouseOverButton ? 0.7f : 0.35f));
    graphics.drawRoundedRectangle(bounds, 6.0f, 0.9f);
}

void BlueSteelLookAndFeel::drawToggleButton(
    juce::Graphics& graphics,
    juce::ToggleButton& button,
    bool isMouseOverButton,
    bool isButtonDown)
{
    const auto area = button.getLocalBounds().toFloat().reduced(3.0f, 4.0f);
    const auto centerY = area.getCentreY();
    const auto led = juce::Rectangle<float>(area.getX() + 2.0f, centerY - 7.0f, 14.0f, 14.0f);
    const auto enabled = button.getToggleState();
    const auto ledColour = enabled
        ? button.findColour(juce::ToggleButton::tickColourId)
        : juce::Colour{0xff4a565a};

    graphics.setColour(juce::Colours::black.withAlpha(0.48f));
    graphics.fillRoundedRectangle(area.translated(0.0f, 1.0f), 7.0f);

    graphics.setGradientFill(juce::ColourGradient(
        juce::Colour{0xff354148}, area.getX(), area.getY(),
        juce::Colour{0xff172126}, area.getRight(), area.getBottom(),
        false));
    graphics.fillRoundedRectangle(area, 7.0f);

    if (enabled)
    {
        graphics.setColour(ledColour.withAlpha(0.24f));
        graphics.fillEllipse(led.expanded(5.0f));
    }

    graphics.setGradientFill(juce::ColourGradient(
        ledColour.brighter(enabled ? 0.55f : 0.12f), led.getX(), led.getY(),
        ledColour.darker(enabled ? 0.18f : 0.44f), led.getRight(), led.getBottom(),
        false));
    graphics.fillEllipse(led);
    graphics.setColour(juce::Colours::white.withAlpha(enabled ? 0.75f : 0.24f));
    graphics.fillEllipse(led.reduced(4.0f));

    graphics.setColour(
        isButtonDown ? Colours::turquoise : Colours::ivory.withAlpha(isMouseOverButton ? 1.0f : 0.86f));
    graphics.setFont(makeFont(10.0f, true));
    graphics.drawFittedText(
        button.getButtonText(),
        juce::Rectangle<int>(
            juce::roundToInt(led.getRight() + 10.0f),
            0,
            juce::jmax(0, button.getWidth() - juce::roundToInt(led.getRight()) - 12),
            button.getHeight()),
        juce::Justification::centredLeft,
        1);
}

void BlueSteelLookAndFeel::drawComboBox(
    juce::Graphics& graphics,
    int width,
    int height,
    bool isButtonDown,
    int buttonX,
    int buttonY,
    int buttonWidth,
    int buttonHeight,
    juce::ComboBox& comboBox)
{
    juce::ignoreUnused(buttonX, buttonY, buttonWidth, buttonHeight);

    const auto bounds = juce::Rectangle<float>(
        0.5f, 0.5f, static_cast<float>(width) - 1.0f, static_cast<float>(height) - 1.0f);
    graphics.setColour(juce::Colours::black.withAlpha(0.42f));
    graphics.fillRoundedRectangle(bounds.translated(0.0f, 1.0f), 6.0f);

    graphics.setGradientFill(juce::ColourGradient(
        juce::Colour{0xff273841}, 0.0f, 1.0f,
        isButtonDown ? juce::Colour{0xff304853} : Colours::midnight,
        0.0f, static_cast<float>(height),
        false));
    graphics.fillRoundedRectangle(bounds, 6.0f);

    graphics.setColour(
        comboBox.hasKeyboardFocus(false) ? Colours::turquoise : Colours::steelLight.withAlpha(0.7f));
    graphics.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    const auto arrowX = static_cast<float>(width) - 22.0f;
    const auto arrowY = static_cast<float>(height) * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath(arrowX - 4.0f, arrowY - 2.0f);
    arrow.lineTo(arrowX, arrowY + 2.5f);
    arrow.lineTo(arrowX + 4.0f, arrowY - 2.0f);
    graphics.setColour(Colours::turquoise.withAlpha(0.95f));
    graphics.strokePath(arrow, juce::PathStrokeType(1.5f));
}

void BlueSteelLookAndFeel::drawPopupMenuBackground(
    juce::Graphics& graphics,
    int width,
    int height)
{
    graphics.setColour(Colours::ink);
    graphics.fillAll();
    graphics.setColour(Colours::steelLight.withAlpha(0.55f));
    graphics.drawRect(0, 0, width, height, 1);
}

class BlueSteelEQAudioProcessorEditor::ParameterControl final : public juce::Component
{
public:
    ParameterControl(
        BlueSteelEQAudioProcessor& audioProcessor,
        BlueSteelLookAndFeel& lookAndFeel,
        juce::String parameterID,
        juce::String controlTitle)
        : processor(audioProcessor),
          id(std::move(parameterID)),
          title(std::move(controlTitle)),
          choices(choicesForControl(id))
    {
        caption.setText(title.toUpperCase(), juce::dontSendNotification);
        caption.setJustificationType(juce::Justification::centred);
        caption.setColour(juce::Label::textColourId, Colours::muted);
        caption.setFont(makeFont(9.0f, true));
        addAndMakeVisible(caption);

        value.setJustificationType(juce::Justification::centred);
        value.setColour(juce::Label::textColourId, Colours::ivory);
        value.setFont(makeFont(9.0f));
        addAndMakeVisible(value);

        slider.setLookAndFeel(&lookAndFeel);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setRotaryParameters(
            juce::MathConstants<float>::pi * 1.25f,
            juce::MathConstants<float>::pi * 2.75f,
            true);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, Colours::steelLight);
        slider.setColour(juce::Slider::rotarySliderFillColourId, Colours::turquoise);
        slider.setColour(juce::Slider::thumbColourId, Colours::ivory);

        if (choices.isEmpty())
        {
            slider.setTextValueSuffix(title.containsIgnoreCase("width") ? "" : " dB");
        }
        else
        {
            slider.setRange(0.0, static_cast<double>(choices.size() - 1), 1.0);
        }

        if (const auto* parameter = processor.parameters.getParameter(id))
        {
            const auto defaultValue = parameter->convertFrom0to1(parameter->getDefaultValue());
            slider.setDoubleClickReturnValue(true, static_cast<double>(defaultValue));
        }

        slider.setTooltip(title + (choices.isEmpty() ? " — double-click to reset" : " — frequency selector"));
        slider.onValueChange = [this]
        {
            updateValueLabel();
            processor.refreshFrequencyResponse();
        };
        addAndMakeVisible(slider);
        attachment = std::make_unique<SliderAttachment>(processor.parameters, id, slider);
        updateValueLabel();
    }

    void paint(juce::Graphics& graphics) override
    {
        juce::ignoreUnused(graphics);
    }

    void resized() override
    {
        const auto bounds = getLocalBounds();
        caption.setBounds(bounds.removeFromTop(18));

        const auto knobSize = juce::jmax(
            35,
            juce::jmin(94, juce::jmin(bounds.getWidth() - 4, bounds.getHeight() - 19)));
        const auto knobY = bounds.getY()
            + juce::jmax(0, (bounds.getHeight() - 19 - knobSize) / 2);
        slider.setBounds(
            bounds.getCentreX() - knobSize / 2,
            knobY,
            knobSize,
            knobSize);
        value.setBounds(
            bounds.getX(),
            bounds.getBottom() - 18,
            bounds.getWidth(),
            16);
    }

    void updateValueLabel()
    {
        if (!choices.isEmpty())
        {
            const auto index = juce::jlimit(
                0, choices.size() - 1, juce::roundToInt(static_cast<float>(slider.getValue())));
            value.setText(choices[index], juce::dontSendNotification);
            return;
        }

        const auto parameterValue = processor.getParameterValue(id);
        if (id == ParameterIDs::midPeakWidth || id == ParameterIDs::midDipWidth
            || id == ParameterIDs::highBandwidth)
        {
            value.setText("Q " + juce::String(parameterValue, 2), juce::dontSendNotification);
        }
        else if (id == ParameterIDs::input || id == ParameterIDs::output
                 || id == ParameterIDs::clipLevel)
        {
            value.setText(
                juce::String(parameterValue >= 0.0f ? "+" : "")
                    + juce::String(parameterValue, 1) + " dB",
                juce::dontSendNotification);
        }
        else
        {
            value.setText(juce::String(parameterValue, 1), juce::dontSendNotification);
        }
    }

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    BlueSteelEQAudioProcessor& processor;
    juce::String id;
    juce::String title;
    juce::StringArray choices;
    juce::Slider slider;
    juce::Label caption;
    juce::Label value;
    std::unique_ptr<SliderAttachment> attachment;
};

class BlueSteelEQAudioProcessorEditor::ResponseDisplay final : public juce::Component
{
public:
    explicit ResponseDisplay(BlueSteelEQAudioProcessor& audioProcessor)
        : processor(audioProcessor)
    {
        setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics& graphics) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto panel = bounds.reduced(2.0f);
        graphics.setColour(juce::Colours::black.withAlpha(0.75f));
        graphics.fillRoundedRectangle(panel, 7.0f);
        graphics.setColour(Colours::ivory.withAlpha(0.2f));
        graphics.drawRoundedRectangle(panel, 7.0f, 1.0f);

        const auto plot = panel.reduced(30.0f, 25.0f);
        if (plot.getWidth() < 80.0f || plot.getHeight() < 45.0f)
            return;

        const auto dbToY = [&plot](float decibels)
        {
            const auto clampedDb = juce::jlimit(-18.0f, 18.0f, decibels);
            return plot.getY() + (18.0f - clampedDb) / 36.0f * plot.getHeight();
        };
        const auto xForFrequency = [&plot](float frequency)
        {
            const auto position = std::log(frequency / 20.0f) / std::log(1000.0f);
            return plot.getX() + static_cast<float>(position) * plot.getWidth();
        };

        for (const auto decibels : {-18, -12, -6, 0, 6, 12, 18})
        {
            const auto y = dbToY(static_cast<float>(decibels));
            graphics.setColour(decibels == 0
                ? Colours::ivory.withAlpha(0.36f)
                : Colours::ivory.withAlpha(0.15f));
            graphics.drawHorizontalLine(
                juce::roundToInt(y), plot.getX(), plot.getRight());
            graphics.setFont(makeFont(8.0f));
            graphics.setColour(Colours::muted.withAlpha(0.88f));
            graphics.drawText(
                juce::String(decibels),
                juce::Rectangle<int>(
                    juce::roundToInt(panel.getX() + 4.0f),
                    juce::roundToInt(y - 6.0f),
                    23,
                    12),
                juce::Justification::centredRight);
        }

        for (int frequencyIndex = 0; frequencyIndex < 13; ++frequencyIndex)
        {
            const auto frequency = frequencyForResponseLabel(frequencyIndex);
            const auto x = xForFrequency(frequency);
            graphics.setColour(Colours::ivory.withAlpha(0.14f));
            graphics.drawVerticalLine(
                juce::roundToInt(x), plot.getY(), plot.getBottom());
            graphics.setColour(Colours::muted.withAlpha(0.9f));
            graphics.setFont(makeFont(7.5f));
            graphics.drawFittedText(
                formatFrequency(frequency),
                juce::Rectangle<int>(
                    juce::roundToInt(x - 16.0f),
                    juce::roundToInt(plot.getBottom() + 4.0f),
                    32,
                    12),
                juce::Justification::centred,
                1);
        }

        juce::Path response;
        constexpr auto numberOfPoints = 112;
        for (int point = 0; point <= numberOfPoints; ++point)
        {
            const auto proportion = static_cast<float>(point) / numberOfPoints;
            const auto frequency = 20.0f * std::pow(1000.0f, proportion);
            const auto x = plot.getX() + proportion * plot.getWidth();
            const auto y = dbToY(processor.getFrequencyResponse(frequency));

            if (point == 0)
                response.startNewSubPath(x, y);
            else
                response.lineTo(x, y);
        }

        juce::Path fill = response;
        fill.lineTo(plot.getRight(), dbToY(0.0f));
        fill.lineTo(plot.getX(), dbToY(0.0f));
        fill.closeSubPath();
        graphics.setColour(Colours::turquoise.withAlpha(0.12f));
        graphics.fillPath(fill);
        graphics.setColour(Colours::midnight.withAlpha(0.8f));
        graphics.strokePath(response, juce::PathStrokeType(4.0f));
        graphics.setColour(Colours::turquoise);
        graphics.strokePath(response, juce::PathStrokeType(1.8f));

        drawMeters(graphics, panel);

        graphics.setColour(Colours::ivory.withAlpha(0.78f));
        graphics.setFont(makeFont(8.0f, true));
        graphics.drawText(
            "EQ RESPONSE",
            panel.toNearestInt().withTrimmedLeft(8).withTrimmedTop(4).withHeight(14),
            juce::Justification::centredLeft);
    }

private:
    void drawMeters(juce::Graphics& graphics, const juce::Rectangle<float>& panel)
    {
        const auto meterY = panel.getY() + 8.0f;
        const auto meterWidth = 108.0f;
        const auto inputWidth = meterWidth * meterProportion(processor.getInputPeak());
        const auto outputWidth = meterWidth * meterProportion(processor.getOutputPeak());
        const auto drawMeter = [&graphics, meterY](float x, float width, const juce::String& name, float level)
        {
            graphics.setColour(juce::Colours::black.withAlpha(0.72f));
            graphics.fillRoundedRectangle(x, meterY, width, 5.0f, 2.0f);
            if (level > 0.0f)
            {
                graphics.setGradientFill(juce::ColourGradient(
                    Colours::turquoise, x, meterY,
                    level > 0.88f ? Colours::red : Colours::amber,
                    x + juce::jmax(1.0f, level), meterY,
                    false));
                graphics.fillRoundedRectangle(x, meterY, juce::jmax(1.0f, level), 5.0f, 2.0f);
            }

            graphics.setColour(Colours::muted);
            graphics.setFont(makeFont(7.0f, true));
            graphics.drawText(
                name,
                juce::Rectangle<int>(
                    juce::roundToInt(x + width + 5.0f),
                    juce::roundToInt(meterY - 4.0f),
                    29,
                    12),
                juce::Justification::centredLeft);
        };

        const auto usableWidth = panel.getWidth() - 44.0f;
        const auto firstX = panel.getX() + 36.0f;
        drawMeter(firstX, meterWidth, "IN", inputWidth);
        drawMeter(firstX + juce::jmax(0.0f, usableWidth - 238.0f), meterWidth, "OUT", outputWidth);
    }

    static float meterProportion(float peak)
    {
        const auto decibels = juce::Decibels::gainToDecibels(juce::jmax(peak, 0.00001f), -60.0f);
        return juce::jlimit(0.0f, 1.0f, (decibels + 48.0f) / 48.0f);
    }

    BlueSteelEQAudioProcessor& processor;
};

BlueSteelEQAudioProcessorEditor::BlueSteelEQAudioProcessorEditor(
    BlueSteelEQAudioProcessor& processor)
    : AudioProcessorEditor(processor),
      audioProcessor(processor),
      responseDisplay(std::make_unique<ResponseDisplay>(processor))
{
    setLookAndFeel(&blueSteelLookAndFeel);
    setOpaque(true);
    editorScale = audioProcessor.getUiScale();
    setSize(
        juce::roundToInt(static_cast<float>(windowWidth) * editorScale),
        juce::roundToInt(static_cast<float>(windowHeight) * editorScale));

    for (const auto* id : {ParameterIDs::input, ParameterIDs::output})
        addParameterControl(
            id,
            id == ParameterIDs::input ? "IN LEVEL" : "OUT LEVEL");

    addParameterControl(ParameterIDs::lowFrequency, "FREQUENCY");
    addParameterControl(ParameterIDs::lowBoost, "BOOST");
    addParameterControl(ParameterIDs::lowAttenuationFrequency, "ATTEN FREQ");
    addParameterControl(ParameterIDs::lowAttenuation, "ATTENUATE");

    addParameterControl(ParameterIDs::midPeakFrequency, "PEAK FREQ");
    addParameterControl(ParameterIDs::midPeakGain, "PEAK BOOST");
    addParameterControl(ParameterIDs::midPeakWidth, "PEAK WIDTH");
    addParameterControl(ParameterIDs::midDipFrequency, "DIP FREQ");
    addParameterControl(ParameterIDs::midDipGain, "DIP ATTEN");
    addParameterControl(ParameterIDs::midDipWidth, "DIP WIDTH");

    addParameterControl(ParameterIDs::highFrequency, "FREQUENCY");
    addParameterControl(ParameterIDs::highBoost, "BOOST");
    addParameterControl(ParameterIDs::highBandwidth, "BANDWIDTH");
    addParameterControl(ParameterIDs::highAttenuationFrequency, "ATTEN FREQ");
    addParameterControl(ParameterIDs::highAttenuation, "ATTENUATE");

    addParameterControl(ParameterIDs::lowCut, "LOW CUT");
    addParameterControl(ParameterIDs::highCut, "HIGH CUT");
    addParameterControl(ParameterIDs::clipLevel, "CEILING");

    addAndMakeVisible(*responseDisplay);

    for (int index = 0; index < static_cast<int>(audioProcessor.getFactoryPresets().size()); ++index)
        presetSelector.addItem(
            audioProcessor.getFactoryPresets()[static_cast<std::size_t>(index)].name,
            index + 1);
    presetSelector.setSelectedId(
        audioProcessor.getCurrentProgram() + 1,
        juce::dontSendNotification);
    presetSelector.setTooltip("Select a factory equalizer preset");
    presetSelector.onChange = [this]
    {
        choosePreset(presetSelector.getSelectedItemIndex());
    };
    addAndMakeVisible(presetSelector);

    powerButton.setClickingTogglesState(true);
    powerButton.setColour(juce::ToggleButton::tickColourId, Colours::turquoise);
    powerButton.setTooltip("Bypass the entire processor");
    powerButton.setToggleState(
        audioProcessor.getParameterValue(ParameterIDs::bypass) < 0.5f,
        juce::dontSendNotification);
    powerButton.onClick = [this]
    {
        audioProcessor.setParameterValue(ParameterIDs::bypass, powerButton.getToggleState() ? 0.0f : 1.0f);
    };
    addAndMakeVisible(powerButton);

    clipperButton.setClickingTogglesState(true);
    clipperButton.setColour(juce::ToggleButton::tickColourId, Colours::amber);
    clipperButton.setTooltip("Enable or disable the analog-style soft clipper");
    clipperButton.setToggleState(
        audioProcessor.getParameterValue(ParameterIDs::clipperEnabled) >= 0.5f,
        juce::dontSendNotification);
    clipperButton.onClick = [this]
    {
        audioProcessor.setParameterValue(
            ParameterIDs::clipperEnabled,
            clipperButton.getToggleState() ? 1.0f : 0.0f);
    };
    addAndMakeVisible(clipperButton);

    snapshotA = audioProcessor.copyCurrentState();
    snapshotB = snapshotA.createCopy();

    for (auto* button : {
             &snapshotAButton, &snapshotBButton, &storeButton, &aboutButton,
             &undoButton, &redoButton})
    {
        button->setColour(juce::TextButton::buttonColourId, Colours::ink);
        button->setColour(juce::TextButton::buttonOnColourId, Colours::steelDark);
        button->setColour(juce::TextButton::textColourOffId, Colours::ivory);
        button->setColour(juce::TextButton::textColourOnId, Colours::turquoise);
        addAndMakeVisible(*button);
    }

    snapshotAButton.setTooltip("Recall snapshot A");
    snapshotBButton.setTooltip("Recall snapshot B");
    storeButton.setTooltip("Store the current sound in the selected A/B snapshot");
    aboutButton.setTooltip("About LOOPS.CO.KE, view licenses, and visit the studio");
    undoButton.setTooltip("Undo the last parameter change");
    redoButton.setTooltip("Redo the last parameter change");

    snapshotAButton.onClick = [this] { selectSnapshot(true); };
    snapshotBButton.onClick = [this] { selectSnapshot(false); };
    storeButton.onClick = [this] { storeSnapshot(); };
    aboutButton.onClick = [this] { showAboutMenu(); };
    undoButton.onClick = [this]
    {
        audioProcessor.undoManager.undo();
        for (const auto& control : parameterControls)
            control->updateValueLabel();
        audioProcessor.refreshFrequencyResponse();
        updateUndoButtons();
    };
    redoButton.onClick = [this]
    {
        audioProcessor.undoManager.redo();
        for (const auto& control : parameterControls)
            control->updateValueLabel();
        audioProcessor.refreshFrequencyResponse();
        updateUndoButtons();
    };

    static constexpr std::array<int, 9> scalePercentages{
        70, 80, 90, 100, 110, 125, 150, 175, 200
    };
    for (const auto percentage : scalePercentages)
        sizeSelector.addItem(
            juce::String(percentage) + "%",
            percentage);
    sizeSelector.setTooltip("Set the complete control panel size from 70% to 200%");
    sizeSelector.setSelectedId(
        juce::roundToInt(editorScale * 100.0f),
        juce::dontSendNotification);
    sizeSelector.onChange = [this]
    {
        const auto percentage = sizeSelector.getSelectedId();
        if (percentage > 0)
            setEditorScale(static_cast<float>(percentage) * 0.01f);
    };
    addAndMakeVisible(sizeSelector);

    setResizable(true, true);
    setResizeLimits(
        juce::roundToInt(static_cast<float>(windowWidth) * 0.7f),
        juce::roundToInt(static_cast<float>(windowHeight) * 0.7f),
        windowWidth * 2,
        windowHeight * 2);

    selectSnapshot(true);
    updateUndoButtons();
    startTimerHz(30);
}

BlueSteelEQAudioProcessorEditor::~BlueSteelEQAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void BlueSteelEQAudioProcessorEditor::addParameterControl(
    const juce::String& id,
    const juce::String& name)
{
    auto control = std::make_unique<ParameterControl>(audioProcessor, blueSteelLookAndFeel, id, name);
    controlsById.emplace(id.toStdString(), control.get());
    addAndMakeVisible(*control);
    parameterControls.push_back(std::move(control));
}

void BlueSteelEQAudioProcessorEditor::layoutControl(
    const juce::String& id,
    const juce::Rectangle<int>& bounds)
{
    const auto control = controlsById.find(id.toStdString());
    jassert(control != controlsById.end());
    if (control != controlsById.end())
        control->second->setBounds(bounds);
}

void BlueSteelEQAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    const juce::Graphics::ScopedSaveState saveState(graphics);
    horizontalScale = static_cast<float>(getWidth()) / static_cast<float>(windowWidth);
    verticalScale = static_cast<float>(getHeight()) / static_cast<float>(windowHeight);
    graphics.addTransform(juce::AffineTransform::scale(horizontalScale, verticalScale));
    const auto designWidth = static_cast<float>(getWidth()) / horizontalScale;
    const auto designHeight = static_cast<float>(getHeight()) / verticalScale;

    auto background = juce::ColourGradient(
        juce::Colour{0xff3d687d}, 0.0f, 0.0f,
        juce::Colour{0xff24485d}, 0.0f, designHeight,
        false);
    background.addColour(0.42, juce::Colour{0xff5c8293});
    graphics.setGradientFill(background);
    graphics.fillRect(0.0f, 0.0f, designWidth, designHeight);

    for (int row = 0; row < juce::roundToInt(designHeight); row += 3)
    {
        const auto band = static_cast<float>((row * 17) % 31) / 31.0f;
        graphics.setColour(
            (row % 2 == 0 ? juce::Colours::white : Colours::ink)
                .withAlpha(0.012f + band * 0.009f));
        graphics.drawHorizontalLine(row, 0.0f, designWidth);
    }

    auto heading = juce::Rectangle<float>(9.0f, 8.0f, designWidth - 18.0f, 41.0f);
    graphics.setGradientFill(juce::ColourGradient(
        juce::Colour{0xff202a2e}, heading.getX(), heading.getY(),
        juce::Colour{0xff101619}, heading.getRight(), heading.getBottom(),
        false));
    graphics.fillRoundedRectangle(heading, 6.0f);
    graphics.setColour(Colours::steelLight.withAlpha(0.55f));
    graphics.drawRoundedRectangle(heading, 6.0f, 1.0f);

    graphics.setColour(Colours::turquoise);
    graphics.fillRoundedRectangle(heading.getX() + 10.0f, heading.getY() + 11.0f, 3.0f, 19.0f, 1.4f);
    graphics.setColour(Colours::ivory);
    graphics.setFont(    makeFont(17.0f, true));
    graphics.drawText(
        "BLUE STEEL",
        juce::Rectangle<int>(24, 9, 185, 23),
        juce::Justification::centredLeft);
    graphics.setColour(Colours::muted);
    graphics.setFont(makeFont(8.0f, true));
    graphics.drawText(
        "FREQUENCY SHAPER  /  V2.0",
        juce::Rectangle<int>(26, 31, 220, 12),
        juce::Justification::centredLeft);
    graphics.setColour(Colours::ivory.withAlpha(0.83f));
    graphics.setFont(makeFont(9.0f, true));
    graphics.drawText(
        "PREMIUM EQ  /  LOOPS.CO.KE",
        juce::Rectangle<int>(222, 14, 176, 20),
        juce::Justification::centredLeft);

    drawPanel(
        graphics, juce::Rectangle<float>(18.0f, 66.0f, designWidth - 36.0f, 174.0f),
        "GAIN / IN LEVEL / OUT LEVEL  •  ANALOG STAGE", Colours::turquoise);
    drawPanel(
        graphics, juce::Rectangle<float>(18.0f, 250.0f, 420.0f, 176.0f),
        "LOW END  •  SHELF + ATTENUATION", Colours::amber);
    drawPanel(
        graphics, juce::Rectangle<float>(448.0f, 250.0f, designWidth - 466.0f, 176.0f),
        "PEAK PRODUCTION  •  MID PEAK + DIP", Colours::turquoise);
    drawPanel(
        graphics, juce::Rectangle<float>(18.0f, 436.0f, 728.0f, 176.0f),
        "HIGH SHAPE  •  PRESENCE + ATTENUATION", Colours::amber);
    drawPanel(
        graphics, juce::Rectangle<float>(756.0f, 436.0f, designWidth - 774.0f, 176.0f),
        "CUT FILTERS", Colours::red);

    const auto footerY = 640;
    graphics.setColour(Colours::ink.withAlpha(0.84f));
    graphics.drawHorizontalLine(footerY - 8, 22.0f, designWidth - 22.0f);
    graphics.setColour(Colours::muted.withAlpha(0.9f));
    graphics.setFont(makeFont(8.0f));
    graphics.drawText(
        "DOUBLE-CLICK ANY CONTROL TO RESET    •    ALL KNOBS SUPPORT HOST AUTOMATION",
        juce::Rectangle<int>(20, footerY - 4, juce::roundToInt(designWidth) - 40, 16),
        juce::Justification::centred);

    drawRivet(graphics, 7.0f, 76.0f);
    drawRivet(graphics, designWidth - 7.0f, 76.0f);
    drawRivet(graphics, 7.0f, getHeight() - 14.0f);
    drawRivet(graphics, designWidth - 7.0f, designHeight - 14.0f);
}

void BlueSteelEQAudioProcessorEditor::resized()
{
    horizontalScale = static_cast<float>(getWidth()) / static_cast<float>(windowWidth);
    verticalScale = static_cast<float>(getHeight()) / static_cast<float>(windowHeight);
    presetSelector.setBounds(scaledBounds({397, 13, 164, 31}));
    storeButton.setBounds(scaledBounds({566, 13, 57, 31}));
    snapshotAButton.setBounds(scaledBounds({629, 13, 30, 31}));
    snapshotBButton.setBounds(scaledBounds({663, 13, 30, 31}));
    sizeSelector.setBounds(scaledBounds({701, 13, 67, 31}));
    aboutButton.setBounds(scaledBounds({773, 13, 68, 31}));
    undoButton.setBounds(scaledBounds({847, 13, 52, 31}));
    redoButton.setBounds(scaledBounds({903, 13, 52, 31}));
    powerButton.setBounds(scaledBounds({966, 12, 202, 33}));

    responseDisplay->setBounds(scaledBounds({147, 84, 682, 144}));
    layoutControl(ParameterIDs::input, scaledBounds({25, 87, 108, 146}));
    layoutControl(ParameterIDs::output, scaledBounds({844, 87, 108, 146}));
    clipperButton.setBounds(scaledBounds({979, 100, 163, 38}));
    layoutControl(ParameterIDs::clipLevel, scaledBounds({1007, 148, 105, 73}));

    const auto placeControls = [this](
        const std::array<const char*, 6>& ids,
        int count,
        juce::Rectangle<int> panel,
        float interiorRatio)
    {
        auto interior = panel.reduced(11, 26);
        const auto cellWidth = interior.getWidth() / count;
        const auto controlHeight = juce::jmax(82, interior.getHeight());

        for (int index = 0; index < count; ++index)
        {
            const auto x = interior.getX() + cellWidth * index;
            auto bounds = juce::Rectangle<int>(x, interior.getY(), cellWidth, controlHeight);
            const auto sideInset = static_cast<int>(static_cast<float>(cellWidth) * interiorRatio);
            layoutControl(
                ids[static_cast<std::size_t>(index)],
                scaledBounds(bounds.reduced(sideInset, 0)));
        }
    };

    placeControls(
        {ParameterIDs::lowFrequency, ParameterIDs::lowBoost,
         ParameterIDs::lowAttenuationFrequency, ParameterIDs::lowAttenuation,
         "", ""},
        4, {18, 250, 420, 176}, 0.05f);
    placeControls(
        {ParameterIDs::midPeakFrequency, ParameterIDs::midPeakGain, ParameterIDs::midPeakWidth,
         ParameterIDs::midDipFrequency, ParameterIDs::midDipGain, ParameterIDs::midDipWidth},
        6, {448, 250, 714, 176}, 0.045f);
    placeControls(
        {ParameterIDs::highFrequency, ParameterIDs::highBoost, ParameterIDs::highBandwidth,
         ParameterIDs::highAttenuationFrequency, ParameterIDs::highAttenuation, ""},
        5, {18, 436, 728, 176}, 0.05f);
    placeControls(
        {ParameterIDs::lowCut, ParameterIDs::highCut, "", "", "", ""},
        2, {756, 436, 406, 176}, 0.12f);
}

juce::Rectangle<int> BlueSteelEQAudioProcessorEditor::scaledBounds(
    const juce::Rectangle<int>& bounds) const
{
    return {
        juce::roundToInt(static_cast<float>(bounds.getX()) * horizontalScale),
        juce::roundToInt(static_cast<float>(bounds.getY()) * verticalScale),
        juce::roundToInt(static_cast<float>(bounds.getWidth()) * horizontalScale),
        juce::roundToInt(static_cast<float>(bounds.getHeight()) * verticalScale)
    };
}

void BlueSteelEQAudioProcessorEditor::setEditorScale(float scale)
{
    editorScale = juce::jlimit(0.7f, 2.0f, scale);
    audioProcessor.setUiScale(editorScale);

    const auto newWidth = juce::roundToInt(static_cast<float>(windowWidth) * editorScale);
    const auto newHeight = juce::roundToInt(static_cast<float>(windowHeight) * editorScale);
    if (getWidth() != newWidth || getHeight() != newHeight)
        setSize(newWidth, newHeight);

    resized();
    repaint();
}

void BlueSteelEQAudioProcessorEditor::showAboutMenu()
{
    juce::PopupMenu menu;
    menu.addItem(1, "About LOOPS.CO.KE");
    menu.addItem(2, "Visit loops.co.ke");
    menu.addSeparator();
    menu.addItem(3, "Open source licenses");
    menu.addItem(4, "Plugin manual and presets");

    juce::PopupMenu::Options options;
    options = options.withTargetComponent(&aboutButton);
    menu.showMenuAsync(options, [this](int selected)
    {
        if (selected == 1)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::InfoIcon,
                "ABOUT LOOPS.CO.KE",
                "LOOPS is a team of music enthusiasts offering producers a curated "
                "catalog of high-quality, royalty-free sounds and a smooth creative workflow.\n\n"
                "The LOOPS catalog covers house, techno, Afrobeats, hip-hop and cinematic "
                "sounds, alongside licensing information, production tools and creator services.\n\n"
                "BLUE STEEL EQ v2 is a LOOPS.CO.KE original audio effect. Its independently "
                "designed EQ presets are starting points, not affiliated, endorsed or "
                "official presets from any third-party audio hardware manufacturer.\n\n"
                "Find us at https://loops.co.ke");
        }
        else if (selected == 2)
        {
            juce::URL("https://loops.co.ke").launchInDefaultBrowser();
        }
        else if (selected == 3)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::InfoIcon,
                "OPEN-SOURCE LICENSES",
                "BLUE STEEL EQ v2: GNU AGPL-3.0-only.\n\n"
                "JUCE framework: GNU AGPL-3.0-or-later.\n\n"
                "The complete Blue Steel EQ and JUCE license notices are installed "
                "with the plugin and available at github.com/joe-calih/puigtec.");
            juce::URL(
                "https://github.com/joe-calih/puigtec/blob/main/LICENSES/AGPL-3.0.txt")
                .launchInDefaultBrowser();
        }
        else if (selected == 4)
        {
            juce::URL("https://github.com/joe-calih/puigtec").launchInDefaultBrowser();
        }
    });
}

void BlueSteelEQAudioProcessorEditor::choosePreset(int index)
{
    audioProcessor.applyFactoryPreset(index);
    audioProcessor.refreshFrequencyResponse();
    for (const auto& control : parameterControls)
        control->updateValueLabel();

    presetSelector.setSelectedId(
        audioProcessor.getCurrentProgram() + 1,
        juce::dontSendNotification);

    updateUndoButtons();
}

void BlueSteelEQAudioProcessorEditor::selectSnapshot(bool useA)
{
    activeSnapshotA = useA;
    auto& button = useA ? snapshotAButton : snapshotBButton;
    auto& otherButton = useA ? snapshotBButton : snapshotAButton;
    button.setColour(
        juce::TextButton::buttonColourId,
        Colours::steelDark);
    button.setColour(
        juce::TextButton::textColourOffId,
        Colours::turquoise);
    otherButton.setColour(
        juce::TextButton::buttonColourId,
        Colours::ink);
    otherButton.setColour(
        juce::TextButton::textColourOffId,
        Colours::ivory);

    const auto& state = useA ? snapshotA : snapshotB;
    if (state.isValid())
    {
        audioProcessor.restoreState(state);
        if (const auto* bypassParameter =
                audioProcessor.parameters.getRawParameterValue(ParameterIDs::bypass))
        {
            powerButton.setToggleState(
                bypassParameter->load(std::memory_order_relaxed) < 0.5f,
                juce::dontSendNotification);
        }

        if (const auto* clipperParameter =
                audioProcessor.parameters.getRawParameterValue(ParameterIDs::clipperEnabled))
        {
            clipperButton.setToggleState(
                clipperParameter->load(std::memory_order_relaxed) >= 0.5f,
                juce::dontSendNotification);
        }

        for (const auto& control : parameterControls)
            control->updateValueLabel();

        presetSelector.setSelectedId(
            audioProcessor.getCurrentProgram() + 1,
            juce::dontSendNotification);
    }

    audioProcessor.refreshFrequencyResponse();
    responseDisplay->repaint();
}

void BlueSteelEQAudioProcessorEditor::storeSnapshot()
{
    if (activeSnapshotA)
        snapshotA = audioProcessor.copyCurrentState();
    else
        snapshotB = audioProcessor.copyCurrentState();

    storeButton.setTooltip(
        activeSnapshotA
            ? "Snapshot A stored"
            : "Snapshot B stored");
    responseDisplay->repaint();
}

void BlueSteelEQAudioProcessorEditor::updateUndoButtons()
{
    undoButton.setEnabled(audioProcessor.undoManager.canUndo());
    redoButton.setEnabled(audioProcessor.undoManager.canRedo());
}

void BlueSteelEQAudioProcessorEditor::timerCallback()
{
    responseDisplay->repaint();

    const auto shouldClipperBeEnabled =
        audioProcessor.getParameterValue(ParameterIDs::clipperEnabled) >= 0.5f;
    if (clipperButton.getToggleState() != shouldClipperBeEnabled)
        clipperButton.setToggleState(shouldClipperBeEnabled, juce::dontSendNotification);

    const auto shouldBeEnabled = audioProcessor.getParameterValue(ParameterIDs::bypass) < 0.5f;
    if (powerButton.getToggleState() != shouldBeEnabled)
        powerButton.setToggleState(shouldBeEnabled, juce::dontSendNotification);

    const auto selectedPresetId = audioProcessor.getCurrentProgram() + 1;
    if (presetSelector.getSelectedId() != selectedPresetId)
        presetSelector.setSelectedId(selectedPresetId, juce::dontSendNotification);

    updateUndoButtons();
}
