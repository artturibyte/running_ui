cmake_minimum_required(VERSION 3.15)
project(RunningTracker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Qt5 package
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Enable automoc for Qt
set(CMAKE_AUTOMOC ON)

# Add executable
add_executable(running_tracker
    src/main.cpp
    src/MainWindow.cpp
    src/TrackWidget.cpp
    include/MainWindow.h
    include/TrackWidget.h
)

# Include directories
target_include_directories(running_tracker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(running_tracker
    Qt5::Widgets
)

# Compile options
target_compile_options(running_tracker PRIVATE
    -Wall
    -Wextra
    -pedantic
)
