cmake_minimum_required(VERSION 3.10)

if(DEFINED ENV{VITASDK})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE FILEPATH "toolchain file")
endif()

project(circle_unlock)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2")

# Define target plugin
add_library(circle_unlock SHARED
  main.c
)

target_link_libraries(circle_unlock
  SceCtrl_stub
  SceShellUtil_stub
  taihen_stub
)

# Export configuration for suprx plugin format
vita_create_stubs(stubs circle_unlock main.c)
vita_create_self(circle_unlock.suprx circle_unlock
  UNCOMPRESSED
  CONFIG circle_unlock.yml
)
