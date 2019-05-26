cmake_minimum_required(VERSION 3.0.0)
project(simple_opengl_game VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable( simple_opengl_game src/main.cpp )

add_subdirectory( src )
    target_link_libraries( simple_opengl_game PRIVATE game )
    
find_package(GLEW REQUIRED)
    target_link_libraries(simple_opengl_game PRIVATE GLEW::GLEW)

find_package(glfw3 REQUIRED)
    target_link_libraries(simple_opengl_game PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
    target_link_libraries(simple_opengl_game PRIVATE glm)

find_package( soil2 CONFIG REQUIRED )
    target_link_libraries( simple_opengl_game PRIVATE soil2 )

find_package( Freetype REQUIRED )
    target_link_libraries( simple_opengl_game PRIVATE Freetype::Freetype )



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
