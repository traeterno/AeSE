cmake_minimum_required(VERSION 3.30)
project(AeSE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_executable(AeSE src/main.cpp src/FileChooser.cpp src/Toolbar.cpp)
target_link_libraries(AeSE sfml-window sfml-graphics sfml-system)