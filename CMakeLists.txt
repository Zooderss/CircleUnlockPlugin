cmake_minimum_required(VERSION 3.10)

project(circle_unlock C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O2")

# Create the plugin object
add_library(circle_unlock
  main.c
)

target_link_libraries(circle_unlock
  SceCtrl_stub
  SceShellUtil_stub
  taihen_stub
)

# Generate the .suprx binary directly using circle_unlock.yml
vita_create_self(circle_unlock.suprx circle_unlock
  UNCOMPRESSED
  CONFIG circle_unlock.yaml
)
