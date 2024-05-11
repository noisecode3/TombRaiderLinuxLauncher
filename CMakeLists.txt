cmake_minimum_required(VERSION 3.16)

project(TombRaiderLinuxLauncher)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_BUILD_TYPE Debug)

find_package(Qt5 COMPONENTS Core Gui Test Widgets WebEngineWidgets Sql REQUIRED)

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)

add_subdirectory(libs/miniz)

# suppress ZERO_CHECK dont think its needed
#set(CMAKE_SUPPRESS_REGENERATION true)

set(SOURCES
    src/main.cpp
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
    src/resources.qrc
)

set(TEST_SOURCES
    src/main.cpp
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
)

add_executable(${PROJECT_NAME} ${SOURCES} )

# create the test executable
#enable_testing(false)
#add_executable(${PROJECT_NAME}Test ${TEST_SOURCES})
#add_test(NAME ${PROJECT_NAME}Test COMMAND ${PROJECT_NAME}Test)

# I think it should link to miniz here also
target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::WebEngineWidgets
    Qt5::Sql
    miniz
    ${CURL_LIBRARY}
)
# link to Qt5::Test
# the test will be a console qt application
#target_link_libraries(${PROJECT_NAME}Test PUBLIC
#    Qt5::Core
#    Qt5::Gui
#    Qt5::Test
#    Qt5::Widgets
#    Qt5::Sql
#    miniz
#    ${CURL_LIBRARY}
#)

target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIR} libs/miniz )

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

install(FILES ${CMAKE_SOURCE_DIR}/utils/tombll.db
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
)

