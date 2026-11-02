# ============================================================================
#  RAGE LEAD  -  CMake build file
#  JUCE is downloaded automatically (no need to install anything yourself).
# ============================================================================
cmake_minimum_required(VERSION 3.22)

# Windows: link the C++ runtime statically so the plugin works on any PC
# without needing the "Visual C++ Redistributable" installed.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

# macOS: build for both Intel and Apple Silicon, runs on macOS 10.13+
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "" FORCE)
endif()

project(RAGE_LEAD VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- fetch JUCE 8 -----------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.9
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

# ---- the plugin ---------------------------------------------------------------
juce_add_plugin(RageLead
    COMPANY_NAME                "RageLabs"
    PLUGIN_MANUFACTURER_CODE    RgLb
    PLUGIN_CODE                 RgLd
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "RAGE LEAD"
    BUNDLE_ID                   com.rageLabs.rageLead
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             Instrument Synth)

target_sources(RageLead PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PresetManager.cpp
    Source/PresetManager.h
    Source/Parameters.h
    Source/Oscillator.h
    Source/Filter.h
    Source/Lfo.h
    Source/Voice.h
    Source/Crush.h
    Source/Distortion.h
    Source/Stutter.h
    Source/Chorus.h
    Source/Delay.h
    Source/Reverb.h
    Source/Limiter.h
    Source/LookAndFeel.h
    Source/ScopeComponent.h)

target_compile_definitions(RageLead PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(RageLead
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags)
