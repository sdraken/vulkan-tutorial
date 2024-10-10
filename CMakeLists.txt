cmake_minimum_required(VERSION 3.16)
project(vulkan-tutorial)
#add_compile_definitions(NDEBUG)
set(CMAKE_CXX_STANDARD 17)


find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(DevelopmentEnvironment src/01-DevelopmentEnvironment/main.cpp)
target_link_libraries(DevelopmentEnvironment glfw  Vulkan::Vulkan)

add_executable(DrawingATriangle src/02-DrawingATriangle/main.cpp)
target_link_libraries(DrawingATriangle glfw  Vulkan::Vulkan)

add_executable(VertexBuffers src/03-VertexBuffers/main.cpp)
target_link_libraries(VertexBuffers glfw  Vulkan::Vulkan)

add_executable(UniformBuffers src/04-UniformBuffers/main.cpp)
target_link_libraries(UniformBuffers glfw  Vulkan::Vulkan)

add_executable(TextureMapping src/05-TextureMapping/main.cpp)
target_link_libraries(TextureMapping glfw  Vulkan::Vulkan)
target_include_directories(TextureMapping
    PRIVATE 
        ${THIRD_PARTY_SINGLE_HEADER_LIBRARIES}
)

add_executable(DepthBuffering src/06-DepthBuffering/main.cpp)
target_link_libraries(DepthBuffering glfw  Vulkan::Vulkan)
target_include_directories(DepthBuffering
    PRIVATE 
        ${THIRD_PARTY_SINGLE_HEADER_LIBRARIES}
)

add_executable(LoadingModels src/07-LoadingModels/main.cpp)
target_link_libraries(LoadingModels glfw  Vulkan::Vulkan)
target_include_directories(LoadingModels
    PRIVATE 
        ${THIRD_PARTY_SINGLE_HEADER_LIBRARIES}
)