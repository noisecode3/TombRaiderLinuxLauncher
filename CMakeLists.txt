cmake_minimum_required(VERSION 3.16)

project(TombRaiderLinuxLauncher)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets WebEngineWidgets Sql REQUIRED)

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)

add_subdirectory(libs/quazip)

set(SOURCES
    src/main.cpp
    src/network.cpp
    src/network.h
    src/worker.h
    src/controller.cpp
    src/controller.h
    src/fileop.h
    src/fileop.cpp
    src/TombRaiderLinuxLauncher.cpp
    src/TombRaiderLinuxLauncher.h
    src/TombRaiderLinuxLauncher.ui
    src/resources.qrc
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::WebEngineWidgets
    Qt5::Sql
    QuaZip
    ${CURL_LIBRARY}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIR} )

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION bin
)

install(FILES ${CMAKE_SOURCE_DIR}/utils/tombll.db
    DESTINATION share/${PROJECT_NAME}
)
