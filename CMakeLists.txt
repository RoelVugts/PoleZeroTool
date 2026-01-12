cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME PoZeTool)
set(PRODUCT_NAME "PoZeTool")
set(PLUGIN_VERSION 0.1.0)
set(COMPANY_NAME "RV Audio Design")

set(CMAKE_CXX_STANDARD 26)

set(HARDENED_RUNTIME_ENABLED YES)
set(HARDENED_RUNTIME_OPTIONS "com.apple.security.device.audio-input")

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64 x86_64) # Universal binary
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Support macOS down to High Sierra")
endif()

add_subdirectory(Libraries/JUCE)

project(${PROJECT_NAME} VERSION 0.0.1 LANGUAGES CXX C)

juce_add_plugin(${PROJECT_NAME}
        VERSION                     ${PLUGIN_VERSION}
        COMPANY_NAME                "RV Audio Design"
        IS_SYNTH                    FALSE
        NEEDS_MIDI_INPUT            FALSE               # Does the plugin need midi input?
        NEEDS_MIDI_OUTPUT           FALSE              # Does the plugin need midi output?
        IS_MIDI_EFFECT              FALSE                 # Is this plugin a MIDI effect?
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE    # Does the editor need keyboard focus?
        COPY_PLUGIN_AFTER_BUILD     TRUE        # Should the plugin be installed to a default location after building?
        PLUGIN_MANUFACTURER_CODE    Rvad               # A four-character manufacturer id with at least one upper-case character
        PLUGIN_CODE                 PoZe                           # A unique four-character plugin id with exactly one upper-case character
        FORMATS                     VST3 AU Standalone                 # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
        PRODUCT_NAME                ${PRODUCT_NAME}
        BUNDLE_ID                   com.rvaudiodesign.pozetool
        AAX_CATEGORY                Effect
        AAX_COPY_DIR                ${CMAKE_CURRENT_BINARY_DIR}/AAX_COPY
)

juce_generate_juce_header(${PROJECT_NAME})

file(GLOB_RECURSE SOURCE_FILES Source/*.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})

target_compile_definitions(${PROJECT_NAME}
        PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_LOG_ASSERTIONS=1
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        WIN32_LEAN_AND_MEAN=1
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        juce_audio_utils
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)