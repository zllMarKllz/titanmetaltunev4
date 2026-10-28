cmake_minimum_required(VERSION 3.22)
project(TitanTuneMetal VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Signalsmith Stretch and Signalsmith Linear headers are fetched by the build workflow
# into Source/ so the include paths are deterministic on Windows and local builds.

juce_add_plugin(TitanTuneMetal
    COMPANY_NAME "Titan Audio Lab"
    BUNDLE_ID com.titanaudiolab.titantunemetal
    PLUGIN_MANUFACTURER_CODE TtAL
    PLUGIN_CODE TtM1
    FORMATS VST3 Standalone
    PRODUCT_NAME "TitanTune Metal"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(TitanTuneMetal)

target_sources(TitanTuneMetal PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_features(TitanTuneMetal PRIVATE cxx_std_17)
target_include_directories(TitanTuneMetal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(TitanTuneMetal PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(TitanTuneMetal PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
