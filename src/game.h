#ifndef GAME_H
#define GAME_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "game_level.h"

// Represents the current state of the game
enum GameState {
    
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN

};

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE( 100, 20 );
// Initial velocity of the player paddle
const GLfloat PLAYER_VELOCITY( 500.0f );

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game {

public:
    // Game state
    GameState              State;	
    GLboolean              Keys[1024];
    unsigned int                 Width, Height;
    std::vector<GameLevel> Levels;
    unsigned int           Level;
    // Constructor/Destructor
    Game( unsigned int width, unsigned int height );
    ~Game();
    // Initialize game state (load all shaders/textures/levels)
    void Init();
    // GameLoop
    void ProcessInput( float dt );
    void Update( float dt );
    void Render();

};

#endif
