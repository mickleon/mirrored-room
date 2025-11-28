cmake_minimum_required(VERSION 3.10)

project(MirroredRoom CXX)
set(CMAKE_CXX_STANDARD 17)
set(SOLUTION_ROOT ${CMAKE_CURRENT_LIST_DIR})
set_property(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}
)

add_subdirectory(libraries)

set(SOURCES
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} LINK_PRIVATE raylib)
target_link_libraries(${PROJECT_NAME} LINK_PRIVATE raygui)
set_property(
    TARGET ${PROJECT_NAME}
    PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${SOLUTION_ROOT}
)

