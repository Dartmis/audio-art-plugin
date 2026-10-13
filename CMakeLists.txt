cmake_minimum_required(VERSION 3.22)

project(AudioART VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AudioART
    COMPANY_NAME "AudioART"
    PLUGIN_MANUFACTURER_CODE Dart
    PLUGIN_CODE Aart
    FORMATS Standalone VST3
    PRODUCT_NAME "Audio ART"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES "Analyzer" "Tools"
)

target_sources(AudioART PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Meters.cpp
    Source/LoudnessMeter.cpp
    Source/SpectrumAnalyzer.cpp
    Source/Goniometer.cpp
)

target_compile_definitions(AudioART PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AudioART PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
