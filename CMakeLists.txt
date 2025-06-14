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

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/database/tombll.db")
    message(STATUS "Database not found, running get_trle.sh...")
    execute_process(
        COMMAND bash "${CMAKE_SOURCE_DIR}/database/get_trle.sh"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()

add_subdirectory(libs/miniz)

# Add LIEF as a subdirectory
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

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/LIEF/CMakeLists.txt" AND NOT NO_DATABASE)
    message(STATUS "Submodule 'libs/LIEF' not found. Updating submodules...")
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

add_subdirectory(libs/LIEF)

# Update submodules if we forget
# git submodule update --init --recursive
# git submodule update --remote --merge
# git submodule sync # (optional) if URL change

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
    src/binary.cpp
    src/levelViewList.hpp
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
    libs/LIEF/include
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
