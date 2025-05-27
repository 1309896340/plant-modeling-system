cmake_minimum_required(VERSION 3.25.0)
project(plant-modeling-system VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

file(GLOB ALL_SOURCES CONFIGURE_DEPENDS
    "include/*.h" 
    "src/*.cpp" 
    "lsystem/include/*.h"
    "lsystem/src/*.cpp"
    "lsystem/antlr4/LSystemInput/include/*.h"
    "lsystem/antlr4/LSystemInput/src/*.cpp"
    "lsystem/antlr4/LSystemParser/include/*.h"
    "lsystem/antlr4/LSystemParser/src/*.cpp"
)

find_package(antlr4-runtime CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME}
    "${ALL_SOURCES}"
    "demo/main.cpp"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    antlr4_shared
    glm::glm-header-only
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "include"
    "lsystem/include"
    "lsystem/antlr4/LSystemInput/include"
    "lsystem/antlr4/LSystemParser/include"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)


