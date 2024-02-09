cmake_minimum_required(VERSION 3.8)

add_executable(TriangleApplication "main.cpp" "Application.cpp")
target_include_directories(TriangleApplication PUBLIC "C:/VulkanSDK/1.3.275.0/Include"
                                           "${CMAKE_SOURCE_DIR}/glfw/include")
target_link_directories(TriangleApplication PUBLIC "C:/VulkanSDK/1.3.275.0/Lib"
                                           "${CMAKE_SOURCE_DIR}/glfw/lib-vc2022")
target_link_libraries(TriangleApplication PUBLIC "vulkan-1.lib"
                                         "glfw3.lib")