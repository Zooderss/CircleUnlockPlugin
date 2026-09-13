cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED ENV{VITASDK})
    message(FATAL_ERROR "VITASDK environment variable is not set.")
endif()

set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "VitaSDK Toolchain")
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

project(CircleUnlock C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2")

add_library(circle_unlock SHARED main.c)

target_link_libraries(circle_unlock
    SceCtrl_stub
    SceShellUtil_stub
    SceLibKernel_stub
    taihen_stub
)

# Convert compiled target into circle_unlock.suprx
vita_create_self(circle_unlock.suprx circle_unlock)
