cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME PoZeTool)
set(PRODUCT_NAME "PoZeTool")
set(PLUGIN_VERSION 1.0.3)
set(COMPANY_NAME "RV Audio Design")

set(CMAKE_CXX_STANDARD 26)

set(HARDENED_RUNTIME_ENABLED YES)
set(HARDENED_RUNTIME_OPTIONS "com.apple.security.device.audio-input")

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64 x86_64) # Universal binary
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Support macOS down to High Sierra")
endif()

add_subdirectory(lib/JUCE)

# This tells cmake we have goodies in the /cmake folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(${PROJECT_NAME} VERSION 0.0.1 LANGUAGES CXX C)

set(MOONBASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/moonbase_JUCEClient)

file(GLOB BINARY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/resource/*.png
    ${CMAKE_CURRENT_SOURCE_DIR}/resource/*.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/resource/MyriadPro-Regular.ttf
    ${CMAKE_CURRENT_SOURCE_DIR}/resource/moonbase_api_config.json
)

juce_add_binary_data(PoZeToolBinaryData SOURCES ${BINARY_SOURCES})

set(libs
        PoZeToolBinaryData
        juce_audio_plugin_client
        juce_audio_utils
        juce_analytics
        juce_animation
        juce_gui_basics
        juce_dsp # Only for SIMD
)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.1)
    include_directories(${GTK3_INCLUDE_DIRS} ${WEBKIT2GTK_INCLUDE_DIRS})

    # Make sure to link against libcurl.so.4 and not libcurl.gnutls.so.4
    find_package(CURL REQUIRED)
    list(APPEND libs CURL::libcurl)
endif()

set(PLUGIN_COMPILE_DEFINITIONS
        JUCE_DONT_DECLARE_PROJECTINFO=0
        JUCE_ALLOW_STATIC_NULL_VARIABLES=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_ENABLE_REPAINT_DEBUGGING=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_LOG_ASSERTIONS=1
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_WEB_BROWSER=1
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        WIN32_LEAN_AND_MEAN=1
        JUCE_USE_CURL=1
        PLUGIN_VERSION="${PLUGIN_VERSION}"
        PRODUCT_NAME="${PRODUCT_NAME}"
)

juce_add_module(lib/moonbase_JUCEClient)

list(APPEND INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/")
list(APPEND INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/Libraries")
list(APPEND INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/JUCE_Custom")
list(APPEND INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
list(APPEND INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/moonbase_JUCEClient/Source")

if (APPLE)
    set(PLUGIN_FORMATS AU VST3 AAX)
else()
    set(PLUGIN_FORMATS VST3 AAX)
endif()

message(STATUS "Building Plug-In Formats: ${PLUGIN_FORMATS}")

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
        PLUGIN_CODE                 diss                           # A unique four-character plugin id with exactly one upper-case character
        FORMATS                     ${PLUGIN_FORMATS}                 # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
        PRODUCT_NAME                ${PRODUCT_NAME}
        BUNDLE_ID                   com.rvaudiodesign.disseptor
        AAX_CATEGORY                Effect
        AAX_COPY_DIR                ${CMAKE_CURRENT_BINARY_DIR}/AAX_COPY
)

juce_generate_juce_header(${PROJECT_NAME})

file(GLOB PLUGIN_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/components/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/fft/fft4g.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/Libraries/Utilities/util.cpp)


target_sources(${PROJECT_NAME} PRIVATE ${PLUGIN_SOURCES})

target_compile_definitions(${PROJECT_NAME}
        PUBLIC
        ${PLUGIN_COMPILE_DEFINITIONS}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        # AudioPluginData           # If we'd created a binary data target, we'd link to it here
        PUBLIC
        ${libs}
        moonbase_JUCEClient
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRECTORY})

# Set environment variables
include(SetEnvironment)

if(APPLE OR WIN32)
    # Update installer text files (README, distribution.xml)
    include(UpdateInstallerFiles)
endif()