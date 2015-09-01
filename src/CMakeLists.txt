########################################################################
# Project setup
########################################################################

cmake_minimum_required(VERSION 2.8)
project(ODR-DabMod)
enable_language(CXX)
enable_testing()

# Select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)


########################################################################
# Version information
########################################################################

set(VERSION_INFO_MAJOR  0)
set(VERSION_INFO_MINOR  5)
set(VERSION_INFO_PATCH  3)

if(NOT DEFINED VERSION_INFO_EXTRA)
    set(VERSION_INFO_EXTRA "git")
endif()
include(Version)

if(NOT DEFINED VERSION)
    #set(VERSION "\"${VERSION_INFO_MAJOR}.${VERSION_INFO_MINOR}.${VERSION_INFO_PATCH}\"")
    set(VERSION "\"${VERSION_INFO}\"")
endif()

set(GITVERSION "\"${VERSION_INFO}\"")
set(PACKAGE_NAME "\"ODR-DabMod\"")
set(PACKAGE "\"ODR-DabMod\"")

#add_definitions(-DVERSION="${VERSION_INFO}")
#add_definitions(-DGITVERSION="${VERSION_INFO}")
#add_definitions(-DPACKAGE="odr-dabmod")
#add_definitions(-DPACKAGE_NAME="ODR-DabMod")


########################################################################
# Compiler specific setup
########################################################################

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|x86")
    set(USE_SIMD "SSE2" CACHE STRING "Use SIMD instructions")
else()
    set(USE_SIMD "no" CACHE STRING "Use SIMD instructions")
endif()
    #set(USE_SIMD_VALUES "no" "SSE2" "AVX")
    set(USE_SIMD_VALUES "no" "SSE2")
    set_property(CACHE USE_SIMD PROPERTY STRINGS ${USE_SIMD_VALUES})
    list(FIND USE_SIMD_VALUES ${USE_SIMD} USE_SIMD_INDEX)
    if(${USE_SIMD_INDEX} EQUAL -1)
        message(FATAL_ERROR "Option ${USE_SIMD} not supported, valid entries are ${USE_SIMD_VALUES}")
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    add_definitions(-Wall)
    #add_definitions(-Wextra)
    #add_definitions(-Wsign-compare)
    if(NOT WIN32)
        #only export symbols that are declared to be part of the api (non dll platforms)
        add_definitions(-fvisibility=hidden)
        add_definitions(-fvisibility-inlines-hidden)
    endif(NOT WIN32)
    if(USE_SIMD MATCHES SSE2)
        add_definitions(-msse2)
        #add_definitions(-DUSE_SSE2)
    endif()
    if(USE_SIMD MATCHES AVX)
        add_definitions(-march=native)
        #add_definitions(-DUSE_AVX)
    endif()
elseif(MSVC)
    if(USE_SIMD MATCHES SSE2)
        add_definitions(/arch:SSE2)
        add_definitions(-DUSE_SSE2)
    endif()
    if(USE_SIMD MATCHES AVX)
        add_definitions(/arch:AVX)
        #add_definitions(-DUSE_AVX)
    endif()
endif()


########################################################################
# Find build dependencies
########################################################################

find_package(PkgConfig)
# Threads
find_package(Threads REQUIRED)
# FFTW3
pkg_check_modules(FFTW3 fftw3f)
# libzmq
pkg_check_modules(ZMQ libzmq >= 4.0)
# UHD
pkg_check_modules(UHD uhd)
# Boost
find_package(Boost 1.41 REQUIRED COMPONENTS system thread)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost required to compile ODR-DabMod\n")
else()
    include_directories(${Boost_INCLUDE_DIRS})
endif()


########################################################################
# config.h
########################################################################

add_definitions(-DHAVE_CONFIG_H)

#Includes
include(CheckIncludeFile)
check_include_file("netinet/in.h"        HAVE_NETINET_IN_H)

#Functions
include(CheckFunctionExists)
check_function_exists("bzero"            HAVE_BZERO)
check_function_exists("gettimeofday"     HAVE_GETTIMEOFDAY)

#Declarations
#include(CheckPrototypeExists)
#check_prototype_exists(_mm_malloc "mm_malloc.h" HAVE_DECL__MM_MALLOC)


configure_file ("${PROJECT_SOURCE_DIR}/cmakeconfig.h.in" "${PROJECT_BINARY_DIR}/config.h" )
include_directories(${PROJECT_BINARY_DIR})


########################################################################
# Options
########################################################################

option(ENABLE_ZMQ "Enable ZeroMQ input, output and remote control" ${ZMQ_FOUND})
option(ENABLE_UHD "Enable UHD output" ${UHD_FOUND})
option(ENABLE_FFTW3 "Enable FFTW3 FFT" ${FFTW3_FOUND})
#option(ENABLE_FFTW3 "Enable FFTW3 FFT" OFF)
#option(ENABLE_KISS_FFT "Prefer KISS FFT over FFTW3" OFF)
#option(ENABLE_KISS_SIMD "Enable SIMD instructions for kiss-fft (unstable)" OFF)


if(ENABLE_FFTW3)
    if(NOT FFTW3_FOUND)
        message(FATAL_ERROR "fftw3 required to compile ODR-DabMod with ENABLE_FFTW3 \n")
    endif()
    include_directories(${FFTW3_INCLUDE_DIRS})
    add_definitions(-DUSE_FFTW)
    list(APPEND EXTRA_LIBS ${FFTW3_LIBRARIES} m)
endif()

if(ENABLE_ZMQ)
    if(NOT ZMQ_FOUND)
        message(FATAL_ERROR "libzmq required to compile ODR-DabMod with ENABLE_ZEROMQ \n")
    endif()
    include_directories(${ZMQ_INCLUDE_DIRS})
    add_definitions(-DHAVE_ZEROMQ)
    list(APPEND EXTRA_LIBS ${ZMQ_LIBRARIES})
endif()



if(ENABLE_UHD)
    if(NOT UHD_FOUND)
        message(FATAL_ERROR "libuhd required to compile ODR-DabMod with ENABLE_UHD \n")
    endif()
    include_directories(${UHD_INCLUDE_DIRS})
    add_definitions(-DHAVE_OUTPUT_UHD)
    list(APPEND EXTRA_LIBS ${UHD_LIBRARIES})
    # TODO: dist_bin_SCRIPTS = src/crc-dwap.py
endif()


if(ENABLE_KISS_FFT)
    # TODO
    # TODO
    # TODO
endif()


if(NOT(ENABLE_FFTW3 OR ENABLE_KISS_FFT))
    message(FATAL_ERROR "Need at least on FFT-library (FFTW3 or KISS FFT) to compile ODR-DabMod \n")
endif()


########################################################################
# Setup apps
########################################################################

#list(APPEND kiss_fft_sources
#    lib/kiss_fft129/kiss_fft.c
#    lib/kiss_fft129/kiss_fft.h
#    lib/kiss_fft129/tools/kiss_fftr.c
#    lib/kiss_fft129/tools/kiss_fftr.h
#    src/kiss_fftsimd.c
#    src/kiss_fftsimd.h
#    )


list(APPEND odr-dabmod_sources
    DabMod.cpp
    porting.c
    DabModulator.cpp
    Buffer.cpp
    ModCodec.cpp
    ModPlugin.cpp
    ModFormat.cpp
    EtiReader.cpp
    Eti.cpp
    FicSource.cpp
    FIRFilter.cpp
    ModInput.cpp
    PuncturingRule.cpp
    PuncturingEncoder.cpp
    SubchannelSource.cpp
    Flowgraph.cpp
    GainControl.cpp
    OutputMemory.cpp
    OutputZeroMQ.cpp
    TimestampDecoder.cpp
    OutputUHD.cpp
    ModOutput.cpp
    InputMemory.cpp
    InputFileReader.cpp
    InputZeroMQReader.cpp
    OutputFile.cpp
    FrameMultiplexer.cpp
    ModMux.cpp
    PrbsGenerator.cpp
    BlockPartitioner.cpp
    QpskSymbolMapper.cpp
    FrequencyInterleaver.cpp
    PhaseReference.cpp
    DifferentialModulator.cpp
    NullSymbol.cpp
    SignalMultiplexer.cpp
    CicEqualizer.cpp
    OfdmGenerator.cpp
    GuardIntervalInserter.cpp
    Resampler.cpp
    ConvEncoder.cpp
    TimeInterleaver.cpp
    Log.cpp
    RemoteControl.cpp
    FormatConverter.cpp
    Utils.cpp
    )

set_source_files_properties(${odr-dabmod_sources} PROPERTIES LANGUAGE "CXX")
add_executable(odr-dabmod ${odr-dabmod_sources})
target_link_libraries(odr-dabmod ${Boost_LIBRARIES} ${EXTRA_LIBS})

install(TARGETS odr-dabmod DESTINATION bin)


########################################################################
# Print Summary
########################################################################
message(STATUS "")
message(STATUS "################################################")
message(STATUS "## Building for version: ${VERSION}")
message(STATUS "## Using install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "################################################")
message(STATUS "")

