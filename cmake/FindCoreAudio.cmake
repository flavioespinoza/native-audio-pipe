# FindCoreAudio.cmake
# Locate Apple CoreAudio framework
#
# This module defines:
#   COREAUDIO_FOUND - System has CoreAudio
#   COREAUDIO_INCLUDE_DIRS - CoreAudio include directories
#   COREAUDIO_LIBRARIES - Libraries needed to use CoreAudio

if(APPLE)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(AUDIOUNIT_LIBRARY AudioUnit)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)

    set(COREAUDIO_LIBRARIES
        ${COREAUDIO_LIBRARY}
        ${AUDIOUNIT_LIBRARY}
        ${AUDIOTOOLBOX_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
    )

    if(COREAUDIO_LIBRARY)
        set(COREAUDIO_FOUND TRUE)
        set(COREAUDIO_INCLUDE_DIRS "")
        message(STATUS "Found CoreAudio: ${COREAUDIO_LIBRARY}")
    else()
        set(COREAUDIO_FOUND FALSE)
        message(STATUS "CoreAudio not found")
    endif()
else()
    set(COREAUDIO_FOUND FALSE)
    message(STATUS "CoreAudio is only available on macOS")
endif()

mark_as_advanced(
    COREAUDIO_LIBRARY
    AUDIOUNIT_LIBRARY
    AUDIOTOOLBOX_LIBRARY
    COREFOUNDATION_LIBRARY
)
