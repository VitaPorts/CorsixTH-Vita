cmake_minimum_required(VERSION 3.16)
project(corsixth_m1b_probe LANGUAGES C CXX)
if(NOT DEFINED VITASDK)
  message(FATAL_ERROR "VITASDK and the VitaSDK toolchain are required")
endif()
include("${VITASDK}/share/vita.cmake")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(PROBE_SDK_ID "unrecorded" CACHE STRING "Exact VitaSDK release and sequence")
execute_process(COMMAND git rev-parse --short=12 HEAD
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  OUTPUT_VARIABLE PROBE_GIT_SHA OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT PROBE_GIT_SHA)
  set(PROBE_GIT_SHA "unknown")
endif()

set(VENDOR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party")
add_library(lpeg_probe STATIC
  "${VENDOR_ROOT}/lpeg-1.1.0/lpcap.c"
  "${VENDOR_ROOT}/lpeg-1.1.0/lpcode.c"
  "${VENDOR_ROOT}/lpeg-1.1.0/lpcset.c"
  "${VENDOR_ROOT}/lpeg-1.1.0/lptree.c"
  "${VENDOR_ROOT}/lpeg-1.1.0/lpvm.c")
add_library(lfs_probe STATIC "${VENDOR_ROOT}/luafilesystem-1.8.0/src/lfs.c")
set(LUAJIT_INCLUDE "${VITASDK}/arm-vita-eabi/include/luajit-2.1")
target_include_directories(lpeg_probe PRIVATE "${LUAJIT_INCLUDE}")
target_include_directories(lfs_probe PRIVATE "${LUAJIT_INCLUDE}")

add_executable(corsixth_m1b_probe main.cpp vita_compat.c)
target_include_directories(corsixth_m1b_probe PRIVATE "${LUAJIT_INCLUDE}" "${VITASDK}/arm-vita-eabi/include/SDL2")
target_compile_options(corsixth_m1b_probe PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(corsixth_m1b_probe PRIVATE
  PROBE_GIT_SHA="${PROBE_GIT_SHA}" PROBE_SDK_ID="${PROBE_SDK_ID}"
  PROBE_BUILD_MODE="$<CONFIG>")
target_link_libraries(corsixth_m1b_probe PRIVATE
  lfs_probe lpeg_probe luajit-5.1 SDL2
  SceGxm_stub SceDisplay_stub SceCtrl_stub SceAppMgr_stub SceAppUtil_stub
  SceAudio_stub SceAudioIn_stub SceSysmodule_stub SceIofilemgr_stub
  SceCommonDialog_stub SceTouch_stub SceHid_stub SceMotion_stub
  ScePower_stub SceProcessmgr_stub m)

vita_create_self(eboot.bin corsixth_m1b_probe)
vita_create_vpk(corsixth_m1b_probe.vpk CORSXP002 eboot.bin
  VERSION 01.00 NAME "CorsixTH Vita Lua Probe")
