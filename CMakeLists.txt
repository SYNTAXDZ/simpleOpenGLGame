cmake_minimum_required(VERSION 3.0.0)
project(simple_opengl_game VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(simple_opengl_game main.cpp)

find_package(glad CONFIG REQUIRED)
    target_link_libraries(simple_opengl_game PRIVATE glad::glad)

find_package(glfw3 REQUIRED)
    target_link_libraries(simple_opengl_game PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
    target_link_libraries(simple_opengl_game PRIVATE glm)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
