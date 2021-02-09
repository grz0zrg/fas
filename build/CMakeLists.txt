cmake_minimum_required(VERSION 3.5)
project(fas VERSION 1.0)

############################################################
# source and headers files

get_filename_component(ROOT ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
set (SRCDIR ${ROOT}/src)
set (BINDIR ${ROOT}/bin)

if(WITH_SOUNDPIPE)
    file (GLOB SRC ${SRCDIR}/*.c ${SRCDIR}/afSTFT/*.c ${SRCDIR}/lodepng/*.c ${SRCDIR}/yin/*.c ${SRCDIR}/ccwt/*.c ${SRCDIR}/AHEasing/*.c ${SRCDIR}/Soundpipe/*.c)
else()
    file (GLOB SRC ${SRCDIR}/*.c ${SRCDIR}/afSTFT/*.c ${SRCDIR}/lodepng/*.c ${SRCDIR}/yin/*.c ${SRCDIR}/ccwt/*.c ${SRCDIR}/AHEasing/*.c)
endif()
file (GLOB HEADERS  ${SRCDIR}/*.h ${SRCDIR}/afSTFT/*.h ${SRCDIR}/lodepng/*.h ${SRCDIR}/yin/*.h ${SRCDIR}/tinydir/*.h ${SRCDIR}/ccwt/*.h ${SRCDIR}/AHEasing/*.h)

############################################################
# compiler options

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED TRUE)

# -Wall -Werror
add_compile_options(-Wextra -pedantic)

############################################################
# Output options

option (LIBLFDS720 "Use liblfds720 (arm64 support)" off)
#option (WITH_OSC "Use OSC output" off)
option (WITH_FAUST "Use Faust (custom generators / effects support)" on)
option (WITH_JACK "Use Jack instead of PortAudio" off)
option (WITH_AUBIO "Use Aubio to automatically detect samples fundamental frequencies" on)
option (WITH_SOUNDPIPE "Use Soundpipe (extended generators / effects)" on)
#option (WITH_RUBBERBAND "Use Rubberband (granular pitch shifting)" on)
option (MAGIC_CIRCLE "Use additive synthesis magic circle oscillator" on)
option (PARTIAL_FX "Use additive synthesis per partial effects" on)
option (USE_NEON "Use NEON instructions (ARM platforms)" off)
option (USE_SSE "Use SSE instructions (Desktop platforms)" off)
option (USE_DOUBLE "Use double precision" off)
option (INTERLEAVED_SAMPLE_FORMAT "PortAudio use interleaved buffers (not used when WITH_JACK is set)" on)

############################################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ROOT}/bin)

add_executable(fas ${SRC} ${HEADERS})

############################################################
# Dependencies

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/modules)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
    #add_definitions(-DDEBUG_FRAME_DATA)
endif()

if (LIBLFDS720)
    find_package(Liblfds720 MODULE REQUIRED)
    include_directories(${LIBLFDS720_INCLUDE_DIR})

    target_link_libraries(fas ${LIBLFDS720_LIBRARIES})
else()
    find_package(Liblfds711 MODULE REQUIRED)
    include_directories(${LIBLFDS711_INCLUDE_DIR})

    add_definitions(-DLFDS711)
    target_link_libraries(fas ${LIBLFDS711_LIBRARIES})
endif()

if (WITH_FAUST)
    find_package(LibFaust MODULE REQUIRED)
    include_directories(${LIBFAUST_INCLUDE_DIR})

    add_definitions(-DWITH_FAUST)
    target_link_libraries(fas ${LIBFAUST_LIBRARIES})

    find_package(LLVM REQUIRED CONFIG)

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})

    # ${llvm_libs} does not seem to work
    #llvm_map_components_to_libnames(llvm_libs all)
    #list(REMOVE_ITEM llvm_libs lto)

    target_link_libraries(fas LLVM)

    # for some reasons cmake will link with cc but LLVM need to be linked with C++ linker
    set_target_properties(fas PROPERTIES LINKER_LANGUAGE CXX)
endif()

if (WITH_JACK)
    find_package(LibJack MODULE REQUIRED)
    include_directories(${JACK_INCLUDE_DIRS})

    add_definitions(-DWITH_JACK)
    add_definitions(${JACK_DEFINITIONS})
    target_link_libraries(fas ${JACK_LIBRARIES})
else()
    # PortAudio
    find_package(LibPortAudio MODULE REQUIRED)

    include_directories(${PORTAUDIO_INCLUDE_DIRS})

    target_link_libraries(fas ${PORTAUDIO_LIBRARIES})

    if (INTERLEAVED_SAMPLE_FORMAT)
        add_definitions(-DINTERLEAVED_SAMPLE_FORMAT)
    endif()
endif()

if (WITH_AUBIO)
    find_package(LibAubio MODULE REQUIRED)
    include_directories(${LIBAUBIO_INCLUDE_DIR})

    add_definitions(-DWITH_AUBIO)
    target_link_libraries(fas ${LIBAUBIO_LIBRARIES})
endif()

#if (WITH_RUBBERBAND)
#    find_package(LibRubberband MODULE REQUIRED)
#    include_directories(${LIBRUBBERBAND_INCLUDE_DIR})

#    add_definitions(-DWITH_RUBBERBAND)
#    target_link_libraries(fas ${LIBRUBBERBAND_LIBRARIES})
#endif()

if (WITH_SOUNDPIPE)
    find_package(LibSoundpipe MODULE REQUIRED)
    include_directories(${LIBSOUNDPIPE_INCLUDE_DIR})

    add_definitions(-DWITH_SOUNDPIPE)
    target_link_libraries(fas ${LIBSOUNDPIPE_LIBRARIES})
endif()

#if (WITH_OSC)
#    find_package(Liblo MODULE REQUIRED)
#    include_directories(${LIBLO_INCLUDE_DIR})

#    add_definitions(-DWITH_OSC)
#    target_link_libraries(fas ${LIBLO_LIBRARIES})
#endif()

if (MAGIC_CIRCLE)
    add_definitions(-DMAGIC_CIRCLE)
endif()

if (PARTIAL_FX)
    add_definitions(-DPARTIAL_FX)
endif()

if (USE_SSE)
    FIND_PACKAGE(SSE)
    IF(C_SSE2_FOUND)
    SET(CMAKE_C_FLAGS "${C_SSE2_FLAGS} -DUSE_SSE2 ${CMAKE_C_FLAGS}")
    ENDIF(C_SSE2_FOUND)
    IF(C_SSE3_FOUND)
    SET(CMAKE_C_FLAGS "${C_SSE3_FLAGS} -DUSE_SSE3 ${CMAKE_C_FLAGS}")
    ENDIF(C_SSE3_FOUND)
    IF(C_SSE4_1_FOUND)
    SET(CMAKE_C_FLAGS "${C_SSE4_1_FLAGS} -DUSE_SSE4_1 ${CMAKE_C_FLAGS}")
    ENDIF(C_SSE4_1_FOUND)
    IF(C_SSE4_2_FOUND)
    SET(CMAKE_C_FLAGS "${C_SSE4_2_FLAGS} -DUSE_SSE4_2 ${CMAKE_C_FLAGS}")
    ENDIF(C_SSE4_2_FOUND)
endif()

if (USE_NEON)
    FIND_PACKAGE(ARM)
    IF (NEON_FOUND)
    MESSAGE(STATUS "Neon found with compiler flag : -mfpu=neon -D__NEON__")
    SET(CMAKE_C_FLAGS "-mfpu=neon -D__NEON__ ${CMAKE_C_FLAGS}")
    ENDIF (NEON_FOUND)
    IF (CORTEXA8_FOUND)
    MESSAGE(STATUS "Cortex-A8 Found with compiler flag : -mcpu=cortex-a8")
    SET(CMAKE_C_FLAGS "-mcpu=cortex-a8 -fprefetch-loop-arrays ${CMAKE_C_FLAGS}")
    ENDIF (CORTEXA8_FOUND)
    IF (CORTEXA9_FOUND)
    MESSAGE(STATUS "Cortex-A9 Found with compiler flag : -mcpu=cortex-a9")
    SET(CMAKE_C_FLAGS "-mcpu=cortex-a9 ${CMAKE_C_FLAGS}")
    ENDIF (CORTEXA9_FOUND)
endif()

if (USE_DOUBLE)
    add_definitions(-DUSE_DOUBLE)
    add_definitions(-DFAUSTFLOAT=double)
    add_definitions(-DSPFLOAT=double)
    add_definitions(-DAH_EASING_USE_DBL_PRECIS)
endif()

find_package(Libsamplerate MODULE REQUIRED)
find_package(LibSndFile MODULE REQUIRED)
find_package(LibWebSockets MODULE REQUIRED)
find_package(Threads REQUIRED)

include_directories(${LIBSNDFILE_INCLUDE_DIR} ${LIBSAMPLERATE_INCLUDE_DIR} ${LIBWEBSOCKETS_INCLUDE_DIR})
if (WIN32)
    if (NOT WITH_JACK)
        # add Windows audio libs (note: consider for now that portaudio was compiled with support for wmme/directx/asio/wdmks)
        target_link_libraries(fas m atomic Threads::Threads ${LIBSNDFILE_LIBRARY} ${LIBSAMPLERATE_LIBRARIES} ${LIBWEBSOCKETS_LIBRARIES} z stdc++ winmm setupapi ws2_32)
    endif()
else()
    target_link_libraries(fas m atomic Threads::Threads ${LIBSNDFILE_LIBRARY} ${LIBSAMPLERATE_LIBRARIES} ${LIBWEBSOCKETS_LIBRARIES})
endif()

############################################################
# Install

install(TARGETS fas DESTINATION bin)

############################################################
# Testing

enable_testing()

# does the application run
add_test(NAME Runs COMMAND fas --i)

############################################################
# Packaging

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${ROOT}/LICENSE")
set(CPACK_PACKAGE_DIRECTORY "${ROOT}/packages")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.fsynth.com")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
include(CPack)
