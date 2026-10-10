# Vita-only dependency and package path for the upstream CorsixTH target.
if(NOT VITA OR NOT VITASDK OR NOT EXISTS "${VITASDK}/share/vita.cmake")
  message(FATAL_ERROR "VITA requires the installed VitaSDK toolchain")
endif()
include("${VITASDK}/share/vita.cmake")

set(CORSIXTH_VITA_LIBDIR "${VITASDK}/arm-vita-eabi/lib")
set(CORSIXTH_VITA_INCLUDEDIR "${VITASDK}/arm-vita-eabi/include")

function(corsixth_vita_require_library name)
  if(NOT EXISTS "${CORSIXTH_VITA_LIBDIR}/lib${name}.a")
    message(FATAL_ERROR "Required VitaSDK archive missing: lib${name}.a")
  endif()
endfunction()

function(corsixth_vita_dependencies executable engine)
  foreach(name IN ITEMS SDL2 SDL2_mixer luajit-5.1 freetype png16 z bz2
      opusfile opus vorbisfile vorbis ogg modplug xmp-lite FLAC)
    corsixth_vita_require_library(${name})
  endforeach()

  set(vendor "${CMAKE_SOURCE_DIR}/third_party")
  add_library(corsixth_vita_lpeg STATIC
    "${vendor}/lpeg-1.1.0/lpcap.c"
    "${vendor}/lpeg-1.1.0/lpcode.c"
    "${vendor}/lpeg-1.1.0/lpcset.c"
    "${vendor}/lpeg-1.1.0/lptree.c"
    "${vendor}/lpeg-1.1.0/lpvm.c")
  add_library(corsixth_vita_lfs STATIC
    "${vendor}/luafilesystem-1.8.0/src/lfs.c")

  target_include_directories(corsixth_vita_lpeg PRIVATE "${CORSIXTH_VITA_INCLUDEDIR}/luajit-2.1")
  target_include_directories(corsixth_vita_lfs PRIVATE "${CORSIXTH_VITA_INCLUDEDIR}/luajit-2.1")
  target_include_directories(${engine} PUBLIC
    "${CORSIXTH_VITA_INCLUDEDIR}/SDL2"
    "${CORSIXTH_VITA_INCLUDEDIR}/luajit-2.1"
    "${CORSIXTH_VITA_INCLUDEDIR}/freetype2"
    "${CORSIXTH_VITA_INCLUDEDIR}/libpng16")

  # The codec archives are static. Keep them after SDL2_mixer, then the Vita
  # stubs after SDL2, matching the accepted native SDL2 diagnostic.
  target_link_libraries(${executable} PRIVATE
    corsixth_vita_lfs corsixth_vita_lpeg
    SDL2_mixer opusfile opus vorbisfile vorbis ogg modplug xmp-lite FLAC
    freetype png16 bz2 z luajit-5.1 SDL2
    SceGxm_stub SceDisplay_stub SceCtrl_stub SceAppMgr_stub SceAppUtil_stub
    SceAudio_stub SceAudioIn_stub SceSysmodule_stub SceIofilemgr_stub
    SceCommonDialog_stub SceTouch_stub SceHid_stub SceMotion_stub
    ScePower_stub SceProcessmgr_stub m)
endfunction()

function(corsixth_vita_package executable)
  set(title_id "CORSXP003")
  set(resource_root "${CMAKE_SOURCE_DIR}/CorsixTH")
  set(resource_paths CorsixTH.lua)
  foreach(dir IN ITEMS Lua Bitmap Campaigns Levels)
    file(GLOB_RECURSE entries CONFIGURE_DEPENDS RELATIVE "${resource_root}"
      "${resource_root}/${dir}/*")
    foreach(entry IN LISTS entries)
      if(NOT IS_DIRECTORY "${resource_root}/${entry}")
        if(NOT entry MATCHES "^Bitmap/" OR entry MATCHES "\\.(tab|pal|dat|png|pl8)$")
          list(APPEND resource_paths "${entry}")
        endif()
      endif()
    endforeach()
  endforeach()
  list(SORT resource_paths)
  set(vpk_files "${CMAKE_SOURCE_DIR}/LICENSE.txt" "LICENSE.txt")
  set(manifest "${CMAKE_BINARY_DIR}/vita-resource-manifest.txt")
  file(WRITE "${manifest}" "# CORSXP003 app0 resource manifest; source: CorsixTH/\nLICENSE.txt\n")
  foreach(entry IN LISTS resource_paths)
    list(APPEND vpk_files "${resource_root}/${entry}" "${entry}")
    file(APPEND "${manifest}" "${entry}\n")
  endforeach()
  list(LENGTH resource_paths resource_count)
  message(STATUS "Vita VPK: ${title_id}, ${resource_count} CorsixTH resources")
  vita_create_self(eboot.bin ${executable})
  vita_create_vpk(corsixth_vita_engine.vpk ${title_id} eboot.bin
    VERSION 01.00 NAME "CorsixTH Vita Engine Dev" FILE ${vpk_files})
endfunction()
