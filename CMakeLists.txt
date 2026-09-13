cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK environment variable!")
  endif()
endif()

project(circle_unlock C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O2")

add_library(circle_unlock SHARED
  main.c
)

target_link_libraries(circle_unlock
  SceCtrl_stub
  SceShellUtil_stub
  taihen_stub
)

vita_create_stubs(stubs circle_unlock main.c)
vita_create_self(circle_unlock.suprx circle_unlock
  UNCOMPRESSED
  CONFIG circle_unlock.yml
)
