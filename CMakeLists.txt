cmake_minimum_required(VERSION 3.22)

project(BlueSteelEQ VERSION 2.0.4 LANGUAGES C CXX)

include(FetchContent)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(BlueSteelEQ
    COMPANY_NAME "LOOPS.CO.KE"
    BUNDLE_ID com.joecalih.bluesteeleq
    PLUGIN_MANUFACTURER_CODE JcEq
    PLUGIN_CODE BsEq
    PRODUCT_NAME "Blue Steel EQ v2"
    DESCRIPTION "LOOPS.CO.KE premium equalizer and tape-inspired soft clipper"
    FORMATS VST3 Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx EQ
)

target_sources(BlueSteelEQ
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_features(BlueSteelEQ PUBLIC cxx_std_17)
target_compile_definitions(BlueSteelEQ
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(BlueSteelEQ
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
juce_generate_juce_header(BlueSteelEQ)

juce_add_console_app(BlueSteelEQTests
    PRODUCT_NAME "Blue Steel EQ DSP Tests"
)
target_sources(BlueSteelEQTests
    PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    tests/dsp_tests.cpp
)
target_compile_features(BlueSteelEQTests PRIVATE cxx_std_17)
target_compile_definitions(BlueSteelEQTests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)
target_link_libraries(BlueSteelEQTests
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
juce_generate_juce_header(BlueSteelEQTests)

enable_testing()
add_test(NAME BlueSteelEQTests COMMAND BlueSteelEQTests)
