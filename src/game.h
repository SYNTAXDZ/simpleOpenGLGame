#ifndef GAME_H
#define GAME_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "game_level.h"
#include "ball_object.h"
#include "power_up.h"

// Represents the current state of the game
enum GameState {
    
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN

};

enum Direction {
	
    UP,
	RIGHT,
	DOWN,
	LEFT

};    

typedef std::tuple<bool, Direction, glm::vec2> Collision;

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE( 100, 20 );
// Initial velocity of the player paddle
const GLfloat PLAYER_VELOCITY( 500.0f );

// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY( 100.0f, -350.0f );
// Radius of the ball object
const float BALL_RADIUS = 12.5f;
  

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game {

public:
    // Game state
    GameState              State;	
    GLboolean              Keys[1024];
    unsigned int           Width, Height;
    std::vector<GameLevel> Levels;
    unsigned int           Level;
    std::vector<PowerUp>   PowerUps;
    unsigned int           Lives;
    bool KeysProcessed[1024];

    // Constructor/Destructor
    Game( unsigned int width, unsigned int height );
    ~Game();
    // Initialize game state (load all shaders/textures/levels)
    void Init();
    // GameLoop
    void ProcessInput( float dt );
    bool CheckCollision( GameObject &one, GameObject &two );
    Collision CheckCollision( BallObject &one, GameObject &two );
    Direction VectorDirection( glm::vec2 target );
    void DoCollisions();
    void Update( float dt );
    void Render();

    void SpawnPowerUps( GameObject &block );
    void UpdatePowerUps( float dt );

    // Reset
    void ResetLevel();
    void ResetPlayer();

};

#endif
