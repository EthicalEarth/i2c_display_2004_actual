cmake_minimum_required(VERSION 2.6)
project(i2c_display_2004)

add_executable(i2c_display_2004 main.cpp)

install(TARGETS i2c_display_2004 RUNTIME DESTINATION bin)
