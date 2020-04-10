cmake_minimum_required(VERSION 3.5)
project(fas)

############################################################
# source and headers files

get_filename_component(ROOT ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
set (SRCDIR ${ROOT}/src)
set (BINDIR ${ROOT}/bin)

file (GLOB SRC ${SRCDIR}/*.c ${SRCDIR}/afSTFT/*.c ${SRCDIR}/lodepng/*.c ${SRCDIR}/yin/*.c)
file (GLOB HEADERS  ${SRCDIR}/*.h ${SRCDIR}/afSTFT/*.h ${SRCDIR}/lodepng/*.h ${SRCDIR}/yin/*.h ${SRCDIR}/tinydir/*.h)

############################################################
# compiler options

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED TRUE)

# -Wall
add_compile_options(-Wextra -pedantic -Werror)

############################################################
# Output options

option (LIBLFDS720 "Use liblfds720 (arm64 support)" off)
option (WITH_OSC "Use OSC output" off)
option (WITH_FAUST "Use Faust (custom generators / effects support)" on)
option (WITH_SOUNDPIPE "Use Soundpipe (extended generators / effects)" on)
option (FIXED_WAVETABLE "Use additive synthesis fixed wavetable length oscillator" off)
option (MAGIC_CIRCLE "Use additive synthesis magic circle oscillator" on)
option (PARTIAL_FX "Use additive synthesis per partial effects" on)
option (BANDLIMITED_NOISE "Use additive synthesis + bandlimited noise" off)

############################################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ROOT}/bin)

add_executable(fas ${SRC} ${HEADERS})

############################################################
# Dependencies

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/modules)

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

    llvm_map_components_to_libnames(llvm_libs all)
    list(REMOVE_ITEM llvm_libs lto)

    target_link_libraries(fas ${llvm_libs})
endif()

if (WITH_SOUNDPIPE)
    find_package(LibSoundpipe MODULE REQUIRED)
    include_directories(${LIBSOUNDPIPE_INCLUDE_DIR})

    add_definitions(-DWITH_SOUNDPIPE)
    target_link_libraries(fas ${LIBSOUNDPIPE_LIBRARIES})
endif()

if (WITH_OSC)
    find_package(Liblo MODULE REQUIRED)
    include_directories(${LIBLO_INCLUDE_DIR})

    add_definitions(-DWITH_OSC)
    target_link_libraries(fas ${LIBLO_LIBRARIES})
endif()

if (FIXED_WAVETABLE)
    add_definitions(-DFIXED_WAVETABLE)
endif()

if (MAGIC_CIRCLE)
    add_definitions(-DMAGIC_CIRCLE)
endif()

if (PARTIAL_FX)
    add_definitions(-DPARTIAL_FX)
endif()

if (BANDLIMITED_NOISE)
    add_definitions(-DBANDLIMITED_NOISE)
endif()

find_package(Libsamplerate MODULE REQUIRED)
find_package(LibSndFile MODULE REQUIRED)
find_package(LibPortAudio MODULE REQUIRED)
find_package(LibWebSockets MODULE REQUIRED)
find_package(Threads REQUIRED)

include_directories(${LIBSNDFILE_INCLUDE_DIR} ${PORTAUDIO_INCLUDE_DIRS} ${LIBSAMPLERATE_INCLUDE_DIR} ${LIBWEBSOCKETS_INCLUDE_DIR})
target_link_libraries(fas m Threads::Threads ${LIBSNDFILE_LIBRARY} ${PORTAUDIO_LIBRARIES} ${LIBSAMPLERATE_LIBRARIES} ${LIBWEBSOCKETS_LIBRARIES})

############################################################
# Install

install(TARGETS fas DESTINATION bin)

############################################################
# Testing

enable_testing()

# does the application run
add_test(NAME Runs COMMAND fas --i)