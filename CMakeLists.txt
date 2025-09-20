cmake_minimum_required(VERSION 3.28.0)
project(bopm LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_RELEASE "-g")

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wuninitialized -Wno-sign-compare)

find_package(termui REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE termui::termui)

find_package(cplotlib REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cplotlib::cplotlib)

find_package(nlohmann_json REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

find_package(cpr REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)

find_package(OpenBLAS)
if (OpenBLAS_FOUND)
    message(STATUS "Using OpenBLAS")
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENBLAS)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenBLAS::OpenBLAS)
else()
    message(STATUS "OpenBLAS not found. Building without it.")
endif()

find_package(CUDAToolkit)
if (CUDAToolkit_FOUND)
    message(STATUS "Using cuBLAS from CUDA Toolkit")
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_CUBLAS)
    target_link_libraries(${PROJECT_NAME} PRIVATE CUDA::cublas)
else()
    message(STATUS "CUDAToolkit not found. Building without cuBLAS.")
endif()
