cmake_minimum_required(VERSION 3.10)

project(circle_unlock C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O2")

# Create a SHARED library so vita_create_self can generate the .velf / .suprx
add_library(circle_unlock SHARED
  main.c
)

target_link_libraries(circle_unlock
  SceCtrl_stub
  SceShellUtil_stub
  taihen_stub
)

# Convert to suprx
vita_create_self(circle_unlock.suprx circle_unlock
  UNCOMPRESSED
  CONFIG circle_unlock.yaml
)
