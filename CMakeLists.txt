cmake_minimum_required(VERSION 3.16)
project(mcview VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(
    /DUNICODE
    /D_UNICODE
    /source-charset:utf-8
    /we4828
    /wd4100 # Suppress "unreferenced formal parameter" warnings
  )
  add_compile_options(
    $<$<CONFIG:Debug>:/MTd>
    $<$<CONFIG:Release>:/MT>
    $<$<CONFIG:RelWithDebInfo>:/MT>
    $<$<CONFIG:MinSizeRel>:/MT>
  )
endif()

add_subdirectory(ext/libminecraft-file)
add_subdirectory(ext/JUCE)

juce_add_gui_app(mcview
  ICON_BIG "Resource/icon_512x512@2x.png"
  ICON_SMALL "Resource/icon_512x512.png"
  PRODUCT_NAME "mcview")

set(mcview_files
  Source/AboutComponent.cpp
  Source/AboutComponent.h
  Source/BrowserComponent.cpp
  Source/BrowserComponent.h
  Source/defer.h
  Source/Dimension.h
  Source/DirectoryBrowserComponent.cpp
  Source/DirectoryBrowserComponent.h
  Source/GraphicsHelper.cpp
  Source/GraphicsHelper.h
  Source/LocalizationHelper.cpp
  Source/LocalizationHelper.h
  Source/LookAndFeel.h
  Source/Main.cpp
  Source/MainComponent.h
  Source/MapViewComponent.cpp
  Source/MapViewComponent.h
  Source/OverScroller.hpp
  Source/Pin.cpp
  Source/Pin.h
  Source/PinComponent.cpp
  Source/PinComponent.h
  Source/PNGWriter.cpp
  Source/PNGWriter.h
  Source/Region.h
  Source/RegionTextureCache.cpp
  Source/RegionTextureCache.h
  Source/RegionToTexture.cpp
  Source/RegionToTexture.h
  Source/Settings.cpp
  Source/Settings.h
  Source/SettingsComponent.cpp
  Source/SettingsComponent.h
  Source/TimerInstance.h
  Source/WorldData.cpp
  Source/WorldData.h
  Source/File.h
)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  list(APPEND mcview_files
    Source/mcview.entitlements
    Source/LocalizationHelper.mm
    Source/Settings.mm
  )
endif()

target_sources(mcview PRIVATE ${mcview_files})

target_compile_definitions(mcview
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:mcview,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:mcview,JUCE_VERSION>"
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  target_compile_definitions(mcview
    PRIVATE
      NOMINMAX
      WIN32_LEAN_AND_MEAN
  )
  target_compile_options(mcview PRIVATE $<$<CONFIG:Release>:/Zi>)
  target_link_options(mcview PRIVATE $<$<CONFIG:Release>:/DEBUG>)
endif()

juce_add_binary_data(app-data
  SOURCES
    Resource/baseline_add_white_18dp.png
    Resource/baseline_brightness_3_white_18dp.png
    Resource/baseline_camera_white_18dp.png
    Resource/baseline_keyboard_arrow_left_white_18dp.png
    Resource/baseline_keyboard_arrow_right_white_18dp.png
    Resource/baseline_landscape_white_18dp.png
    Resource/baseline_remove_white_18dp.png
    Resource/baseline_settings_white_18dp.png
    Resource/baseline_whatshot_white_18dp.png
    Resource/icon_512x512.png
    Resource/icon_512x512@2x.png
    Resource/icon_small.png
    Resource/icon_large.png
    Resource/japanese.lang)

target_link_libraries(mcview
  PRIVATE
    app-data
    mcfile
    juce::juce_gui_extra
    juce::juce_opengl
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

target_include_directories(mcview
  PRIVATE
    ext/colormap-shaders/include)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${mcview_files})

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  execute_process(
    COMMAND patch -p1 -N --input ../../JUCE.patch
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ext/JUCE)
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  include_external_msproject(Package "${CMAKE_CURRENT_SOURCE_DIR}/Builds/Package/Package.wapproj"
    TYPE C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5
    GUID 33553e30-3cf6-4634-ad63-aa90ca486a7f)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Builds/Package/Package.appxmanifest.in ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Package/Package.appxmanifest)
