# Local header files here ONLY
add_library( 

    game
    ${CMAKE_CURRENT_SOURCE_DIR}/game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resource_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sprite_renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/game_object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/game_level.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ball_object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/particle_generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/post_processing.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/game.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resource_manager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/texture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sprite_renderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/game_object.h
    ${CMAKE_CURRENT_SOURCE_DIR}/game_level.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ball_object.h
    ${CMAKE_CURRENT_SOURCE_DIR}/particle_generator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/post_processing.h

)

find_package( soil2 CONFIG REQUIRED )
    target_link_libraries( game PRIVATE soil2 )


target_include_directories( game PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" )
#target_include_directories( resource_manager PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" )