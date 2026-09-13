cmake_minimum_required(VERSION 3.10)
project(CircleUnlock C)

if(NOT DEFINED ENV{VITASDK})
    message(FATAL_ERROR "VITASDK environment variable is not set. Please install VitaSDK and export VITASDK path.")
endif()

include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -Wno-unused-variable")

add_library(circle_unlock SHARED main.c)

target_link_libraries(circle_unlock
    SceCtrl_stub
    SceShellUtil_stub
    SceLibKernel_stub
    taihen_stub
)

vita_create_stubs(circle_unlock circle_unlock)
vita_create_prx(circle_unlock.suprx circle_unlock)
