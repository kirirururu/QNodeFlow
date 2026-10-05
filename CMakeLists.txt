cmake_minimum_required(VERSION 4.3)
set(_UNUSED_CMAKE_ARGS ${CMAKE_C_COMPILER})

project(QNodeFlow LANGUAGES CXX)

option(QNODEFLOW_BUILD_EXAMPLE "Build example application" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(src)

if (QNODEFLOW_BUILD_EXAMPLE)
	add_subdirectory(example)
endif ()
