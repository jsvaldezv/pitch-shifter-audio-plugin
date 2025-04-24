# =========================================== JUCE ============================================== #
set(JUCE_DEPENDENCIES 
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# ========================================== Project ============================================ #
set(FORMATS AU VST3 Standalone)

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME "jsvaldezv"
    COMPANY_WEBSITE ""
    COMPANY_EMAIL ""
    BUNDLE_ID com.jsvaldezv.${PROJECT_NAME}
    VERSION ${CURRENT_VERSION}
    PLUGIN_MANUFACTURER_CODE Jsau

    IS_SYNTH FALSE                       
    NEEDS_MIDI_INPUT FALSE               
    NEEDS_MIDI_OUTPUT FALSE              
    IS_MIDI_EFFECT FALSE                
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE		 
    
    PLUGIN_CODE ${CURRENT_PLUGIN_CODE}                     
    FORMATS "${FORMATS}"
    PRODUCT_NAME ${PROJECT_NAME}

    VST3_CATEGORIES Fx Pitch Shift
    VST2_CATEGORY kPlugCategEffect
    AU_MAIN_TYPE kAudioUnitType_Effect
    AAX_CATEGORY AAX_EPlugInCategory_PitchShift
)

# Set C++ version for a given target
target_compile_features(${PROJECT_NAME} PRIVATE ${CPP_VERSION})

# Add binary data
file(GLOB_RECURSE RESOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../Assets/*.*
)

if(RESOURCES)
    juce_add_binary_data(Assets 
        SOURCES 
            ${RESOURCES}
    )
endif()

# Add all source files
set(SourceFiles)
setSourceFiles(SourceFiles)

target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${SourceFiles})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Assets
    ${JUCE_DEPENDENCIES}
    RubberBand
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

juce_generate_juce_header(${PROJECT_NAME})

# JuceHeader.h Remove JuceHeader.h from the source file list and handle it individually
get_target_property(juce_library_code ${PROJECT_NAME} JUCE_GENERATED_SOURCES_DIRECTORY)
set(juce_header "${juce_library_code}/JuceHeader.h")
list(REMOVE_ITEM project_sources ${juce_header})

# Place JuceHeader.h and the associated CMake Rule file into a "JUCE Library Code" folder
source_group("JUCE Library Code" FILES ${juce_header})
source_group("JUCE Library Code/CMake Rules" FILES "${juce_header}.rule")

# Generate targets
foreach(target ${FORMATS} "All")
    if(TARGET ${PROJECT_NAME}_${target})
        set_target_properties(${PROJECT_NAME}_${target} PROPERTIES
                                FOLDER "Targets"
                                XCODE_GENERATE_SCHEME ON)
    endif()
endforeach()