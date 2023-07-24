cmake_minimum_required(VERSION 3.16)
project(mcview VERSION 3.3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF)

set(mcview_macos_deployment_target "10.15")
set(mcview_apple_development_team_id "XM695N4A7T")

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
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
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "${mcview_macos_deployment_target}")
  set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${mcview_apple_development_team_id}")
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

add_subdirectory(ext/libminecraft-file)
add_subdirectory(ext/JUCE)

# Developer accidentally upload an app binary to the appstoreconnect with wrong version number.
# The appstoreconnect doesn't allow version numbers to go down, so we apply a higher value than CFBundleShortVersionString.
math(EXPR mcview_bundle_version_major "${PROJECT_VERSION_MAJOR} + 1")
juce_add_gui_app(mcview
  ICON_BIG "Resource/icon_512x512@2x.png"
  ICON_SMALL "Resource/icon_512x512.png"
  PRODUCT_NAME "mcview"
  APP_SANDBOX_ENABLED TRUE
  APP_SANDBOX_INHERIT FALSE
  APP_SANDBOX_OPTIONS "com.apple.security.files.user-selected.read-write;com.apple.security.files.bookmarks.app-scope"
  VERSION "${CMAKE_PROJECT_VERSION}"
  BUILD_VERSION "${mcview_bundle_version_major}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
  BUNDLE_ID "com.github.kbinani.mcview"
  HARDENED_RUNTIME_ENABLED TRUE
  PLIST_TO_MERGE "
    <plist>
      <dict>
        <key>LSApplicationCategoryType</key>
        <string>public.app-category.productivity</string>
        <key>ITSAppUsesNonExemptEncryption</key>
        <false/>
        <key>LSMinimumSystemVersion</key>
        <string>${mcview_macos_deployment_target}</string>
      </dict>
    </plist>"
  COMPANY_COPYRIGHT "Copyright © 2019, 2022, 2023 kbinani"
)

set(mcview_files
  Source/AboutComponent.hpp
  Source/BrowserComponent.hpp
  Source/defer.hpp
  Source/Dimension.hpp
  Source/DirectoryBrowserComponent.hpp
  Source/LocalizationHelper.cpp
  Source/LocalizationHelper.hpp
  Source/LookAndFeel.hpp
  Source/Application.cpp
  Source/MainComponent.hpp
  Source/MapViewComponent.hpp
  Source/OverScroller.hpp
  Source/Pin.hpp
  Source/PinComponent.hpp
  Source/PNGWriter.cpp
  Source/PNGWriter.hpp
  Source/Region.hpp
  Source/RegionTextureCache.hpp
  Source/RegionToTexture.cpp
  Source/RegionToTexture.hpp
  Source/Settings.hpp
  Source/SettingsComponent.hpp
  Source/TimerInstance.hpp
  Source/WorldData.hpp
  Source/File.hpp
  Source/MainWindow.hpp
  Source/SavePNGProgressWindow.hpp
  Source/LookAt.hpp
  Source/TextInputDialog.hpp
  Source/TexturePackJob.hpp
  Source/RegionUpdateChecker.hpp
  Source/GLUniforms.hpp
  Source/GLVertex.hpp
  Source/GLAttributes.hpp
  Source/GLBuffer.hpp
  Source/DirectoryBrowserModel.hpp
  Source/NotoSansTypeface.hpp
  Resource/Shader/tile.vert
  Resource/Shader/color.frag
  Source/PinEdit.hpp
  Source/Palette.hpp
  Source/Palette.cpp
  Source/PaletteType.hpp
  Source/LightingType.hpp
)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  list(APPEND mcview_files
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

juce_add_binary_data(binary-data
  NAMESPACE mcview::BinaryData
  HEADER_NAME BinaryData.hpp
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
    Resource/japanese.lang
    Resource/shader/tile.vert
    Resource/shader/color.frag)
juce_add_binary_data(font-data
  NAMESPACE mcview::FontData
  HEADER_NAME FontData.hpp
  SOURCES
    Resource/NotoSans-Regular.ttf
    Resource/NotoSans-Bold.ttf
    Resource/NotoSansJP-Regular.ttf
    Resource/NotoSansJP-Bold.ttf
    Resource/NotoSansSC-Regular.otf
    Resource/NotoSansSC-Bold.otf
    Resource/NotoSansTC-Regular.otf
    Resource/NotoSansTC-Bold.otf)

target_link_libraries(mcview
  PRIVATE
    binary-data
    font-data
    mcfile
    juce::juce_gui_extra
    juce::juce_opengl
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

target_include_directories(mcview
  PRIVATE
    ext/colormap-shaders/include
    ext/JUCE/modules/juce_graphics/image_formats)

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
