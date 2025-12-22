cmake_minimum_required(VERSION 3.16)
project(mana_graphics)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages installed by vcpkg
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

# Find all .cpp files in the current directory (excluding build)
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Create executable for each .cpp file
foreach(SOURCE ${SOURCES})
    get_filename_component(NAME ${SOURCE} NAME_WE)
    add_executable(${NAME} ${SOURCE})
    target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${NAME} PRIVATE glfw glad::glad)
    if(WIN32)
        target_link_libraries(${NAME} PRIVATE opengl32)
    endif()
endforeach()
