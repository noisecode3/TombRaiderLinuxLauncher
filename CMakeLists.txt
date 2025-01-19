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
    file(READ "${CMAKE_SOURCE_DIR}/libs/miniz/CMakeLists.txt" cmake)
    string(REPLACE "cmake_minimum_required(VERSION 3.5)"
        "cmake_minimum_required(VERSION 3.16)" my_cmake "${cmake}")
    file(WRITE "${CMAKE_SOURCE_DIR}/libs/miniz/CMakeLists.txt" "${my_cmake}")
endif()

# Update submodules if we forget
# git submodule update --init --recursive
# git submodule update --remote --merge
# git submodule sync # (optional) if URL change

add_subdirectory(libs/miniz)

set(SOURCES_TESTS
    test/test.hpp
)

set(SOURCES_MC
    src/Controller.hpp
    src/Controller.cpp
    src/Data.hpp
    src/Data.cpp
    src/FileManager.hpp
    src/FileManager.cpp
    src/GameFileTree.hpp
    src/GameFileTree.cpp
    src/Model.hpp
    src/Model.cpp
    src/Network.hpp
    src/Network.cpp
    src/Runner.cpp
    src/Runner.hpp
    src/binary.hpp
    src/main.cpp
    src/staticData.hpp
)

set(SOURCES_VIEW
    src/TombRaiderLinuxLauncher.hpp
    src/TombRaiderLinuxLauncher.cpp
    src/TombRaiderLinuxLauncher.ui
    src/resources.qrc
)

set(LINK_COMMON
    Qt5::Core
    Qt5::Widgets
    Qt5::WebEngineWidgets
    Qt5::Sql
    miniz
    ${CURL_LIBRARY}
    OpenSSL::SSL
    Boost::system
    Boost::filesystem
)

set(LINK_GUI
    Qt5::Gui
)

set(INCLUDE_DIR
    ${CURL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    libs/miniz
    src
)

set(COMPILE_OPT
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if(TEST)
    enable_testing()
    set(SOURCES ${SOURCES_MC} ${SOURCES_TESTS})
    add_executable(${PROJECT_NAME}Test ${SOURCES})
    add_test(NAME ${PROJECT_NAME}Test COMMAND ${PROJECT_NAME}Test)
    target_link_libraries(${PROJECT_NAME}Test PUBLIC ${LINK_COMMON} Qt5::Test)
    target_include_directories(${PROJECT_NAME}Test PRIVATE ${INCLUDE_DIR} test)
    set_target_properties(${PROJECT_NAME}Test PROPERTIES ${COMPILE_OPT})
    target_compile_definitions(${PROJECT_NAME}Test PRIVATE TEST)
else()
    set(SOURCES ${SOURCES_MC} ${SOURCES_VIEW})
    add_executable(${PROJECT_NAME} ${SOURCES})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${LINK_COMMON} ${LINK_GUI})
    target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
    set_target_properties(${PROJECT_NAME} PROPERTIES ${COMPILE_OPT})
endif()

if(NOT TEST)
    install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )

    install(FILES ${CMAKE_SOURCE_DIR}/database/tombll.db
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
    )
endif()

