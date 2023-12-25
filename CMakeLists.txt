cmake_minimum_required(VERSION 3.16)

project(TombRaiderLinuxLauncher)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets WebEngineWidgets REQUIRED)

find_package(CURL REQUIRED)

include_directories(include)

add_subdirectory(libs/quazip)

set(SOURCES
    src/main.cpp
    src/TombRaiderLinuxLauncher.cpp
    src/TombRaiderLinuxLauncher.ui
    src/resources.qrc
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::WebEngineWidgets
    QuaZip
    ${CURL_LIBRARIES}
)


target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIR} )

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
