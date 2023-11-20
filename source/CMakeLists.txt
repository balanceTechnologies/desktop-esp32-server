cmake_minimum_required(VERSION 3.2)
project(MyApp LANGUAGES CXX)
add_executable(MyExe main.cpp esp32commands.cpp esp32.cpp servercommands.cpp)
set_property(TARGET MyExe PROPERTY CXX_STANDARD 17)
