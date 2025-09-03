cmake_minimum_required(VERSION 3.16)

project(TombRaiderLinuxLauncher)

option(TEST "Enable building test" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT RELEASE)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Qt5 COMPONENTS Core Gui Test Widgets WebEngineWidgets Sql REQUIRED)

find_package(CURL REQUIRED)

# Add miniz as a subdirectory
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/miniz/CMakeLists.txt")
    message(STATUS "Submodule 'libs/miniz' not found. Updating submodules...")
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    file(READ "${CMAKE_SOURCE_DIR}/libs/miniz/CMakeLists.txt" MINIZ_CMAKE)
    string(REPLACE "cmake_minimum_required(VERSION 3.5)"
        "cmake_minimum_required(VERSION 3.16)"
        NEW_MINIZ_CMAKE
        "${MINIZ_CMAKE}"
    )
    file(WRITE "${CMAKE_SOURCE_DIR}/libs/miniz/CMakeLists.txt"
        "${NEW_MINIZ_CMAKE}"
    )
endif()
add_subdirectory(libs/miniz)

# Add LIEF as a subdirectory
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/LIEF/CMakeLists.txt")
    message(STATUS "Submodule 'libs/LIEF' not found. Updating submodules...")
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
set(LIEF_INSTALL OFF CACHE BOOL "Disable installation of LIEF")
set(LIEF_EXAMPLES OFF)
set(LIEF_TESTS OFF)
set(LIEF_DOC OFF)
set(LIEF_C_API OFF)
set(LIEF_ELF OFF)
set(LIEF_PE ON)
set(LIEF_MACHO OFF)
set(LIEF_DEX OFF)
set(LIEF_ART OFF)
add_subdirectory(libs/LIEF)

# Add libbacktrace as a subdirectory
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/libbacktrace/configure")
    message(STATUS
        "Submodule 'libs/libbacktrace' not found. Updating submodules..."
    )
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/libs/libbacktrace/configure")
    message(STATUS "Running autoconf to prepare libbacktrace")
    execute_process(
        COMMAND bash "${CMAKE_SOURCE_DIR}/libs/libbacktrace/configure"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/libs/libbacktrace"
        RESULT_VARIABLE AUTOCONF_RESULT
    )
    if(NOT AUTOCONF_RESULT EQUAL 0)
        message(
            FATAL_ERROR
            "Autoconf failed with exit code ${AUTOCONF_RESULT}"
        )
    endif()
    execute_process(
        COMMAND make
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/libs/libbacktrace"
        RESULT_VARIABLE MAKE_RESULT
    )
    if(NOT MAKE_RESULT EQUAL 0)
        message(
            FATAL_ERROR
            "Make failed with exit code ${MAKE__RESULT}"
        )
    endif()
endif()


if(NOT EXISTS "${CMAKE_SOURCE_DIR}/database/tombll.db")
    message(STATUS "Database not found, running get_trle.sh...")
    execute_process(
        COMMAND bash "${CMAKE_SOURCE_DIR}/database/get_trle.sh"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()

set(SOURCES_TESTS
    test/test.hpp
)

set(SOURCES_MC
    src/CommandLineParser.hpp
    src/CommandLineParser.cpp
    src/Controller.hpp
    src/Controller.cpp
    src/Data.hpp
    src/Data.cpp
    src/FileManager.hpp
    src/FileManager.cpp
    src/GameFileTree.hpp
    src/GameFileTree.cpp
    src/LevelViewList.hpp
    src/LevelViewList.cpp
    src/LoadingIndicator.hpp
    src/LoadingIndicator.cpp
    src/Model.hpp
    src/Model.cpp
    src/Network.hpp
    src/Network.cpp
    src/Path.cpp
    src/Path.hpp
    src/PyRunner.cpp
    src/PyRunner.hpp
    src/Runner.cpp
    src/Runner.hpp
    src/assert.hpp
    src/binary.hpp
    src/binary.cpp
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
    LIEF::LIEF
    ${CURL_LIBRARY}
    "${CMAKE_SOURCE_DIR}/libs/libbacktrace/.libs/libbacktrace.a"
)

set(LINK_GUI
    Qt5::Gui
)

set(INCLUDE_DIR
    ${CURL_INCLUDE_DIR}
    libs/miniz
    libs/LIEF/include
    libs/libbacktrace
    src
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(TEST)
    enable_testing()
    set(SOURCES ${SOURCES_MC} ${SOURCES_TESTS})
    add_executable(${PROJECT_NAME}Test ${SOURCES})
    add_test(NAME ${PROJECT_NAME}Test COMMAND ${PROJECT_NAME}Test)
    target_link_libraries(${PROJECT_NAME}Test PUBLIC ${LINK_COMMON} Qt5::Test)
    target_include_directories(${PROJECT_NAME}Test PRIVATE ${INCLUDE_DIR} test)
    target_compile_definitions(${PROJECT_NAME}Test PRIVATE TEST)
    set_target_properties(${PROJECT_NAME}Test PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    message(STATUS "Setup test files")
    execute_process(
        COMMAND bash set_up_files.sh "${CMAKE_SOURCE_DIR}/build"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
        RESULT_VARIABLE SETUP_RESULT
    )

    if(NOT SETUP_RESULT EQUAL 0)
        message(
            FATAL_ERROR
            "set_up_files.sh failed with exit code ${SETUP_RESULT}"
        )
    endif()
else()
    set(SOURCES ${SOURCES_MC} ${SOURCES_VIEW})
    add_executable(${PROJECT_NAME} ${SOURCES})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${LINK_COMMON} ${LINK_GUI})
    target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
endif()

if(NOT TEST)
    install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )

    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/database/data
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
    )

    install(FILES
        ${CMAKE_SOURCE_DIR}/database/data_factory.py
        ${CMAKE_SOURCE_DIR}/database/get_leaf_cert.py
        ${CMAKE_SOURCE_DIR}/database/https.py
        ${CMAKE_SOURCE_DIR}/database/main.py
        ${CMAKE_SOURCE_DIR}/database/make_tombll_database.py
        ${CMAKE_SOURCE_DIR}/database/scrape_common.py
        ${CMAKE_SOURCE_DIR}/database/scrape_trle.py
        ${CMAKE_SOURCE_DIR}/database/scrape_trle_download.py
        ${CMAKE_SOURCE_DIR}/database/tombll_common.py
        ${CMAKE_SOURCE_DIR}/database/tombll_create.py
        ${CMAKE_SOURCE_DIR}/database/tombll_delete.py
        ${CMAKE_SOURCE_DIR}/database/tombll_manage_data.py
        ${CMAKE_SOURCE_DIR}/database/tombll_read.py
        ${CMAKE_SOURCE_DIR}/database/tombll_update.py
        ${CMAKE_SOURCE_DIR}/database/tombll_view.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
    )

    if(NOT NO_DATABASE)
        install(FILES
            ${CMAKE_SOURCE_DIR}/database/tombll.db
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
        )
    endif()
endif()
