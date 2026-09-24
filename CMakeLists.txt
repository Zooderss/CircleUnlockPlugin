cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif()
endif()

project(circle_unlock)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Circle Unlock")
set(VITA_TITLEID  "CIRCUNLK1")

add_executable(circle_unlock main.c)
target_link_libraries(circle_unlock
    taihen_stub
    SceCtrl_stub
    SceTouch_stub
    SceKernelModulemgr_stub
    SceKernelThreadMgr_stub
)

vita_create_self(circle_unlock.suprx circle_unlock CONFIG exports.yml UNSAFE)
