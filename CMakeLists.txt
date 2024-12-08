cmake_minimum_required(VERSION 3.16)
project(mcview VERSION 4.2.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF)

set(mcview_macos_deployment_target "11.0")
set(mcview_apple_development_team_id "XM695N4A7T")

include(cmake/CPM.cmake)

if (MSVC)
  add_definitions(
    /DUNICODE
    /D_UNICODE
    /utf-8
    /we4828
    /wd4100 # Suppress "unreferenced formal parameter" warnings
  )
  add_compile_options(
    $<$<CONFIG:Debug>:/MDd>
    $<$<CONFIG:Release>:/MD>
    $<$<CONFIG:RelWithDebInfo>:/MD>
    $<$<CONFIG:MinSizeRel>:/MD>
    /MP
  )
elseif(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "${mcview_macos_deployment_target}")
  set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${mcview_apple_development_team_id}")
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
  add_compile_options(-Wno-shorten-64-to-32)
endif()

CPMAddPackage(
  NAME libminecraft-file
  GITHUB_REPOSITORY kbinani/libminecraft-file
  GIT_TAG 7c86f4f83b23b53ac464d80fbc7f17fabb4984be
)

CPMAddPackage(
  NAME mimalloc
  GITHUB_REPOSITORY microsoft/mimalloc
  VERSION 2.1.7
  OPTIONS "MI_OVERRIDE ON"
          "MI_BUILD_TESTS OFF"
          "MI_BUILD_OBJECT OFF"
          "MI_BUILD_STATIC OFF"
          "MI_WIN_REDIRECT ON")

add_subdirectory(ext/je2be-core)
add_subdirectory(ext/JUCE)

# Developer accidentally uploaded an app binary to the appstoreconnect with wrong version number.
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
  COMPANY_COPYRIGHT "Copyright © 2019, 2022-2024 kbinani"
)

list(APPEND mcview_files
  Source/AboutComponent.hpp
  Source/LeftPanel.hpp
  Source/defer.hpp
  Source/Dimension.hpp
  Source/GameDirectoryBrowser.hpp
  Source/LocalizationHelper.hpp
  Source/LookAndFeel.hpp
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
  Source/GLUniforms.hpp
  Source/GLVertex.hpp
  Source/GLAttributes.hpp
  Source/GLBuffer.hpp
  Source/GameDirectoryBrowserModel.hpp
  Resource/Shader/tile.vert
  Resource/Shader/color.frag
  Source/PinEdit.hpp
  Source/Palette.hpp
  Source/Palette.cpp
  Source/PaletteType.hpp
  Source/LightingType.hpp
  Source/Edition.hpp
  Source/ThreadPool.hpp
  Source/TexturePackThreadPool.hpp
  Source/TexturePackJob.hpp
  Source/JavaTexturePackJob.hpp
  Source/JavaTexturePackThreadPool.hpp
  Source/BedrockTexturePackJob.hpp
  Source/BedrockTexturePackThreadPool.hpp
  Source/VisibleRegions.hpp
  Source/JavaWorldScanThread.hpp
  Source/BedrockWorldScanThread.hpp
  Source/ColorMat.hpp
  Source/DirectoryCleanupThread.hpp
  Source/WorldScanThread.hpp
  Source/ImageButton.hpp
  Source/LeftPanelHeader.hpp
  Source/CustomDirectoryBrowser.hpp
  Source/ConcertinaPanel.hpp
  Source/GameDirectory.hpp
)
if (APPLE)
  list(APPEND mcview_files Source/Application.mm)
else()
  list(APPEND mcview_files Source/Application.cpp)
endif()

target_sources(mcview PRIVATE ${mcview_files})

juce_add_binary_data(mcview-binary-data
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
    Resource/shader/color.frag
)

target_compile_definitions(mcview
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:mcview,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:mcview,JUCE_VERSION>"
    # MCVIEW_ENABLE_PALETTE_PREP=1
)

if (MSVC)
  target_compile_definitions(mcview
    PRIVATE
      NOMINMAX
      WIN32_LEAN_AND_MEAN
  )
  target_compile_options(mcview PRIVATE $<$<CONFIG:Release>:/Zi>)
  target_link_options(mcview PRIVATE $<$<CONFIG:Release>:/DEBUG>)
elseif(APPLE)
  foreach (target IN ITEMS mcview leveldb je2be libdeflate_static minizip mcview-binary-data)
    set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,YES,NO>)
  endforeach()
endif()

target_link_libraries(mcview
  PRIVATE
    mcview-binary-data
    je2be
    mimalloc
    juce::juce_gui_extra
    juce::juce_opengl
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
)

target_include_directories(mcview
  PRIVATE
    ext/colormap-shaders/include
    ext/JUCE/modules/juce_graphics/image_formats
    ext/je2be-core/src
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${mcview_files})

if (MSVC)
  include_external_msproject(Package "${CMAKE_CURRENT_SOURCE_DIR}/Builds/Package/Package.wapproj"
    TYPE C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5
    GUID 33553e30-3cf6-4634-ad63-aa90ca486a7f
  )
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Package/Package.appxmanifest.in
  ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Package/Package.appxmanifest
)

foreach(target IN ITEMS mcview)
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "${mimalloc_BINARY_DIR}/\$(Configuration)/mimalloc.dll" "$<TARGET_FILE_DIR:${target}>/"
    COMMAND "${CMAKE_COMMAND}" -E copy "${mimalloc_BINARY_DIR}/\$(Configuration)/mimalloc-redirect.dll" "$<TARGET_FILE_DIR:${target}>/"
    COMMENT "Copy mimalloc dlls to output directory")
endforeach()
