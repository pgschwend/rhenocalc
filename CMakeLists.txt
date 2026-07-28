cmake_minimum_required(VERSION 3.21)

# Set up project name and version
project(RhenoCalc VERSION 0.2.5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# -- Boost.Multiprecision via FetchContent -------------------------------------
include(FetchContent)
FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP ON
)
set(BOOST_INCLUDE_LIBRARIES multiprecision math)
FetchContent_MakeAvailable(Boost)

# Set Qt installation prefix path depending on the platform
if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.9.3/mingw_64")
elseif(UNIX AND NOT APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/qt/6.9.3/gcc_64")
elseif(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/Users/patrick/Qt/6.9.3/macos/")
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)

# List of source files excluding the info.h.in template
set(SOURCES
        src/resources/resources.qrc
        src/main.cpp
        src/ui/themecolors.h src/ui/themecolors.cpp
        src/ui/mainwindow.h src/ui/mainwindow.cpp
        src/ui/widgets/fixedtab.h
        src/ui/widgets/colorwheel.h src/ui/widgets/colorwheel.cpp
        src/ui/widgets/bitbutton.h src/ui/widgets/bitbutton.cpp
        src/pages/calculatorpage.h src/pages/calculatorpage.cpp
        src/pages/baseconverterpage.h src/pages/baseconverterpage.cpp
        src/pages/unitconverterpage.h src/pages/unitconverterpage.cpp
        src/pages/networkpage.h src/pages/networkpage.cpp
        src/pages/crchashpage.h src/pages/crchashpage.cpp
        src/pages/colorpage.h src/pages/colorpage.cpp
        src/pages/financepage.h src/pages/financepage.cpp
        src/pages/floatpage.h src/pages/floatpage.cpp
        src/pages/electronicspage.h src/pages/electronicspage.cpp
        src/pages/settingspage.h src/pages/settingspage.cpp
        src/core/calc/operations.h src/core/calc/operations.cpp
        src/core/calc/calcparser.h src/core/calc/calcparser.cpp
        src/core/calculator.h src/core/calculator.cpp
        src/core/baseconverter.h src/core/baseconverter.cpp
        src/core/unitconverter.h src/core/unitconverter.cpp
        src/core/network.h src/core/network.cpp
        src/core/crchash.h src/core/crchash.cpp
        src/core/color.h src/core/color.cpp
        src/core/finance.h src/core/finance.cpp
        src/core/floathandler.h src/core/floathandler.cpp
        src/core/electronics.h src/core/electronics.cpp
        src/core/updater.h src/core/updater.cpp
        src/info.h
)

# Generate info.h into a namespaced subdirectory inside the build directory
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/info.h"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/rhenocalc/info.h"
)


if(APPLE)
    # Configure macOS Application Bundle and App Store Sandboxing
    set(MAC_ICON "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/icons/rhenocalc.icns")
    set_source_files_properties(${MAC_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    list(APPEND SOURCES ${MAC_ICON})

    set(CMAKE_xcode_attribute_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
    add_executable(RhenoCalc MACOSX_BUNDLE ${SOURCES} "${CMAKE_CURRENT_BINARY_DIR}/generated/rhenocalc/info.h")

    set_target_properties(RhenoCalc PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "RhenoCalc"
            MACOSX_BUNDLE_GUI_IDENTIFIER "ch.rhenosys.rhenocalc"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
            MACOSX_BUNDLE_COPYRIGHT "Copyright © 2026 Rhenosys GmbH. All rights reserved."
            MACOSX_BUNDLE_ICON_FILE "rhenocalc.icns"
            XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/RhenoCalc.entitlements"
    )
else()
    # Windows and Linux build target configuration
    if(WIN32)
        list(APPEND SOURCES src/resources/rhenocalc.rc)
    endif()
    add_executable(RhenoCalc ${SOURCES})
endif()

if(WIN32)
    target_compile_definitions(RhenoCalc PRIVATE _USE_MATH_DEFINES)
    set_target_properties(RhenoCalc PROPERTIES WIN32_EXECUTABLE ON)

    # Automate windeployqt deployment post-build
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET RhenoCalc POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations "$<TARGET_FILE:RhenoCalc>"
                COMMENT "Deploying Qt runtime with windeployqt"
                VERBATIM)
    else()
        message(WARNING "windeployqt not found. RhenoCalc.exe may fail at runtime with missing Qt DLLs.")
    endif()
endif()

target_compile_definitions(RhenoCalc PRIVATE
        APP_VERSION_CMAKE_STRING="${PROJECT_VERSION}"
)

# Include local development directories and the generated headers directory
target_include_directories(RhenoCalc PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pages
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/calc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/resources
)

# Link shared libraries
target_link_libraries(RhenoCalc
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
        Boost::multiprecision
)

# Link platform-specific networking libraries for Windows
if(WIN32)
    target_link_libraries(RhenoCalc iphlpapi ws2_32)
endif()