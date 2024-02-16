cmake_minimum_required(VERSION 3.13)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(
        -fexceptions
        -frtti
        -std=c++=20
        -Wall
        -Wpedantic
        -DFOLLY_NO_CONFIG=1
)

set(LIBRARY_TARGET_NAME CommonCppPackage)

add_library(${LIBRARY_TARGET_NAME} SHARED
        Module.cpp
        sharedlogic/Address.cpp
        sharedlogic/Sdk.cpp
        sharedlogic/User.cpp
)

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        sharedlogic
)
