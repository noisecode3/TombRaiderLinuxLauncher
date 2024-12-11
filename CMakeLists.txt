cmake_minimum_required(VERSION 3.16)

project(TombRaiderLinuxLauncher)

option(TEST "Enable building test" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_BUILD_TYPE Debug)

find_package(Qt5 COMPONENTS Core Gui Test Widgets WebEngineWidgets Sql REQUIRED)

find_package(CURL REQUIRED)
# Set policy for Boost to ensure compatibility with newer CMake versions
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
find_package(Boost REQUIRED COMPONENTS system filesystem)

find_package(OpenSSL REQUIRED)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/miniz/CMakeLists.txt")
    message(STATUS "Submodule 'libs/miniz' not found. Updating submodules...")
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

add_subdirectory(libs/miniz)

set(SOURCES
    test/test.h
    src/main.cpp
    src/binary.h
    src/gameTree.h
    src/staticData.h
    src/Network.h
    src/Network.cpp
    src/Controller.h
    src/Controller.cpp
    src/FileManager.h
    src/FileManager.cpp
    src/Model.h
    src/Model.cpp
    src/Data.h
    src/Data.cpp
    src/TombRaiderLinuxLauncher.h
    src/TombRaiderLinuxLauncher.cpp
    src/TombRaiderLinuxLauncher.ui
    src/resources.qrc)

if(TEST)
    enable_testing(ON)
    add_executable(${PROJECT_NAME}Test ${SOURCES})
    add_test(NAME ${PROJECT_NAME}Test COMMAND ${PROJECT_NAME}Test)
else()
    enable_testing(OFF)
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt5::Core
    Qt5::Test
    Qt5::Gui
    Qt5::Widgets
    Qt5::WebEngineWidgets
    Qt5::Sql
    miniz
    ${CURL_LIBRARY}
    OpenSSL::SSL
    Boost::system
    Boost::filesystem
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CURL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    libs/miniz
    src
    test
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if(NOT TEST)
    install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )

    install(FILES ${CMAKE_SOURCE_DIR}/database/tombll.db
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
    )
endif()

