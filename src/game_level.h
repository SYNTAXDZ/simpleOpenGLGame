#ifndef GAMELEVEL_H
#define GAMELEVEL_H

#include "game_object.h"
#include "sprite_renderer.h"
#include "resource_manager.h"

#include <vector>

class GameLevel {

public:

    std::vector<GameObject> Bricks;

    GameLevel() {}
    // Loads level from file
    void Load( const char *file, unsigned int levelWidth, unsigned int levelHeight );

    // Render level
    void Draw( SpriteRenderer &renderer );
    // Check if the level is completed (all non-solid tiles are destroyed)
    bool IsCompleted();

private:

    void init( std::vector<std::vector<unsigned int>>tileData, unsigned int levelWidth, unsigned int levelHeight );

};

#endif