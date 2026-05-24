cmake_minimum_required(VERSION 3.21)
project(RhenoCalc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.9.3/mingw_64")
elseif(UNIX AND NOT APPLE)
    set(CMAKE_PREFIX_PATH "/opt/qt/6.9.3/gcc_64")
elseif(APPLE)
    set(CMAKE_PREFIX_PATH "/usr/local/opt/qt")
endif()


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        Concurrent
        SerialPort
        Sql
        REQUIRED)

set(SOURCES
        src/resources/resources.qrc
        src/main.cpp
        src/ui/themecolors.h src/ui/themecolors.cpp
        src/ui/mainwindow.h src/ui/mainwindow.cpp
        src/pages/calculatorpage.h   src/pages/calculatorpage.cpp
        src/pages/baseconverterpage.h src/pages/baseconverterpage.cpp
        src/pages/unitconverterpage.h src/pages/unitconverterpage.cpp
#        src/pages/networkpage.h src/pages/networkpage.cpp
#        src/pages/modbuspage.h src/pages/modbuspage.cpp
#        src/pages/wirelesspage.h src/pages/wirelesspage.cpp
#        src/pages/bluetoothpage.h src/pages/bluetoothpage.cpp
#        src/pages/settingspage.h src/pages/settingspage.cpp
#        src/pages/infopage.h src/pages/infopage.cpp
#        src/pages/nmappage.h src/pages/nmappage.cpp
#        src/pages/restapipage.h src/pages/restapipage.cpp
#        src/utils/ExportManager.h src/utils/ExportManager.cpp
#        src/widgets/TableSearchWidget.h src/widgets/TableSearchWidget.cpp
#        src/database/ScanHistoryDB.h src/database/ScanHistoryDB.cpp
)

add_executable(RhenoCalc ${SOURCES})

if(WIN32)
    # Ensure Qt runtime DLLs/plugins are deployed next to the executable in build trees.
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

target_include_directories(RhenoCalc PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pages
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src/database
)

target_link_libraries(RhenoCalc
        Qt::Core
        Qt::Gui
        Qt::Widgets
#        Qt::Network
#        Qt::Concurrent
#        Qt::SerialPort
#        Qt::Sql
)
