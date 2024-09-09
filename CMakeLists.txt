cmake_minimum_required(VERSION 3.16)
project(vulkan-tutorial)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(${PROJECT_NAME} src/01-DevelopmentEnvironment/main.cpp)

target_link_libraries(${PROJECT_NAME} glfw  Vulkan::Vulkan)