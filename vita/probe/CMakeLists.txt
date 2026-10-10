cmake_minimum_required(VERSION 3.16)
project(corsixth_m1a_probe LANGUAGES CXX)

if(NOT DEFINED VITASDK)
  message(FATAL_ERROR "Configure with the VitaSDK toolchain and VITASDK environment variable")
endif()
include("${VITASDK}/share/vita.cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

execute_process(COMMAND git rev-parse --short=12 HEAD
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  OUTPUT_VARIABLE PROBE_GIT_SHA OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET)
if(NOT PROBE_GIT_SHA)
  set(PROBE_GIT_SHA "unknown")
endif()
set(PROBE_SDK_ID "unrecorded" CACHE STRING "VitaSDK release/sequence recorded in the probe log")

add_executable(corsixth_m1a_probe main.cpp)
target_compile_options(corsixth_m1a_probe PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(corsixth_m1a_probe PRIVATE
  PROBE_GIT_SHA="${PROBE_GIT_SHA}"
  PROBE_SDK_ID="${PROBE_SDK_ID}"
  PROBE_BUILD_MODE="$<CONFIG>")
target_include_directories(corsixth_m1a_probe PRIVATE "${VITASDK}/arm-vita-eabi/include/SDL2")
target_link_libraries(corsixth_m1a_probe PRIVATE
  SDL2
  SceGxm_stub SceDisplay_stub SceCtrl_stub SceAppMgr_stub SceAppUtil_stub
  SceAudio_stub SceAudioIn_stub SceSysmodule_stub SceIofilemgr_stub
  SceCommonDialog_stub SceTouch_stub SceHid_stub SceMotion_stub
  ScePower_stub SceProcessmgr_stub m)

# Keep the unstripped CMake executable for symbol inspection and crash triage.
vita_create_self(eboot.bin corsixth_m1a_probe)
vita_create_vpk(corsixth_m1a_probe.vpk CORSXP001 eboot.bin
  VERSION 01.00 NAME "CorsixTH Vita GXM Probe")
