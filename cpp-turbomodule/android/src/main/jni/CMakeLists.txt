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

# Use `codegenConfig.name` value from library's `package.json`
set(LIBRARY_TARGET_NAME MyCppTurboPackage)

set(LIBRARY_ANDROID_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(LIBRARY_COMMON_DIR ${LIBRARY_ANDROID_DIR}/../cpp)

add_library(${LIBRARY_TARGET_NAME} STATIC
        ${LIBRARY_COMMON_DIR}/NativeMyCppModuleAdapter.cpp
)

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC ${LIBRARY_COMMON_DIR})

# link C++ TurboModule dependencies
target_link_libraries(${LIBRARY_TARGET_NAME}
        fbjni
        jsi
        react_nativemodule_core
)

# link Codegen headers
target_link_libraries(${LIBRARY_TARGET_NAME} react_codegen_MyCppTurboPackage)

# add shared C++ code
add_subdirectory(${LIBRARY_COMMON_DIR}/../../common-cpp common_cpp_package_build)

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC ${LIBRARY_COMMON_DIR}/../../common-cpp)

target_link_libraries(${LIBRARY_TARGET_NAME} CommonCppPackage)
