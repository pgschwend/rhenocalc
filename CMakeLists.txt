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

