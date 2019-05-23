/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "ball_object.h"
#include "particle_generator.h"
#include "post_processing.h"

#include <algorithm>

// Game-related State data
SpriteRenderer  *Renderer;
GameObject      *Player;
BallObject      *Ball;
ParticleGenerator *Particles;
PostProcessor   *Effects;

float ShakeTime = 0.0f;

Game::Game( unsigned int width, unsigned int height ) 
	: State( GAME_ACTIVE ), Keys(), Width( width ), Height( height ) { 

}

Game::~Game() {
    
    delete Renderer;
    delete Player;
    delete Ball;
    delete Particles;
    delete Effects; 

}

void Game::Init() {
    
    // Load shaders
    ResourceManager::LoadShader( "shaders/particle.vs", "shaders/particle.fs", nullptr, "particle" );
    ResourceManager::LoadShader( "shaders/sprite.vs", "shaders/sprite.fs", nullptr, "sprite" );
    ResourceManager::LoadShader("shaders/effects.vs", "shaders/effects.fs", nullptr, "postprocessing" );
    // Configure shaders
    glm::mat4 projection = glm::ortho( 0.0f, static_cast<float>( this->Width ), static_cast<float>( this->Height ), 0.0f, -1.0f, 1.0f );
    ResourceManager::GetShader( "sprite" ).Use().SetInteger( "image", 0 );
    ResourceManager::GetShader( "sprite" ).SetMatrix4( "projection", projection );
    ResourceManager::GetShader( "particle" ).Use().SetInteger( "sprite", 0 );
    ResourceManager::GetShader( "particle" ).SetMatrix4( "projection", projection );
    // Load textures
    ResourceManager::LoadTexture( "textures/ball.png", true, "face" );
    ResourceManager::LoadTexture( "textures/paddle.png", true, "paddle" );
    ResourceManager::LoadTexture( "textures/background.jpg", false, "background" );
    ResourceManager::LoadTexture( "textures/block.png", false, "block" );
    ResourceManager::LoadTexture( "textures/block_solid.png", false, "block_solid" );
    ResourceManager::LoadTexture( "textures/particle.png", true, "particle" );
    ResourceManager::LoadTexture( "textures/powerup_speed.png", true, "powerup_speed" );
    ResourceManager::LoadTexture( "textures/powerup_sticky.png", true, "powerup_sticky" );
    ResourceManager::LoadTexture( "textures/powerup_increase.png", true, "powerup_increase" );
    ResourceManager::LoadTexture( "textures/powerup_confuse.png", true, "powerup_confuse" );
    ResourceManager::LoadTexture( "textures/powerup_chaos.png", true, "powerup_chaos" );
    ResourceManager::LoadTexture( "textures/powerup_passthrough.png", true, "powerup_passthrough" );
    // Load levels
    GameLevel one; one.Load( "levels/one.lvl", this->Width, this->Height * 0.5 );
    GameLevel two; two.Load( "levels/two.lvl", this->Width, this->Height * 0.5 );
    GameLevel three; three.Load( "levels/three.lvl", this->Width, this->Height * 0.5 );
    GameLevel four; four.Load( "levels/four.lvl", this->Width, this->Height * 0.5 );
    this->Levels.push_back( one );
    this->Levels.push_back( two );
    this->Levels.push_back( three );
    this->Levels.push_back( four );
    this->Level = 0;
    
    auto sprite = ResourceManager::GetShader( "sprite" );
    // Set render-specific controls
    Renderer = new SpriteRenderer( sprite );
    Effects = new PostProcessor( ResourceManager::GetShader( "postprocessing" ), this->Width, this->Height );
    // the player pos
    glm::vec2 playerPos = glm::vec2(
        
        this->Width / 2 - PLAYER_SIZE.x / 2, 
        this->Height - PLAYER_SIZE.y
    
    );
    // setting the player
    Player = new GameObject( playerPos, PLAYER_SIZE, ResourceManager::GetTexture( "paddle" ) );

    glm::vec2 ballPos = playerPos + glm::vec2( PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2 );
    Ball = new BallObject( ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
                           ResourceManager::GetTexture( "face" ) );
    
    Particles = new ParticleGenerator(
        
        ResourceManager::GetShader( "particle" ), 
        ResourceManager::GetTexture( "particle" ), 
        500
    
    );

    //Effects->Shake = true;
    //Effects->Confuse = true;
    //Effects->Chaos = true;

}

void Game::Update( float dt ) {

    // Then we have to update the position of the ball each frame by calling its 
    // Move function within the game code's Update function
    Ball->Move( dt, this->Width );

    this->DoCollisions();

    // update particles,  particles will use the game object properties from the ball object,
    // spawn 2 particles each frame and their positions will be offset towards the center of the ball
    Particles->Update( dt, *Ball, 2, glm::vec2( Ball->Radius/ 2.0 ) );

    if( ShakeTime > 0.0f ) {
        
        ShakeTime -= dt;
        
        if( ShakeTime <= 0.0f )
            Effects->Shake = false;
    
    }

    this->UpdatePowerUps( dt );

}


/*
 Here we move the player paddle either in the left or right direction based on which key the user
 pressed (note how we multiply the velocity with the deltatime variable). If the paddle's x value 
 would be less than 0 it would've moved outside the left edge so we only move the paddle to the 
 left if the paddle's x value is higher than the left edge's x position (0.0). We do the same for when the paddle breaches the right edge, but we have to compare the right edge's position with the right edge of the paddle (subtract the paddle's width from the right edge's x position).
*/
void Game::ProcessInput( GLfloat dt ) {

    if( this->State == GAME_ACTIVE ) {
        
        float velocity = PLAYER_VELOCITY * dt;
        // Move playerboard
        if( this->Keys[GLFW_KEY_A] or this->Keys[GLFW_KEY_LEFT] ) {
            
            if( Player->Position.x >= 0 ) {
                
                Player->Position.x -= velocity;
                // if the user presses the space bar, the ball's Stuck variable is set 
                // to false. We also updated the ProcessInput function to move the position
                // of the ball alongside the paddle's position whenever the ball is stuck. 
                if( Ball->Stuck )
                    Ball->Position.x -= velocity;
            
            }
        
        }
        
        if( this->Keys[GLFW_KEY_D] or this->Keys[GLFW_KEY_RIGHT] ) {
            
            if( Player->Position.x <= this->Width - Player->Size.x ) {
                
                Player->Position.x += velocity;

                if( Ball->Stuck )
                    Ball->Position.x += velocity;
            
            }
        
        }

        // Furthermore, because the ball is initially stuck to the paddle, we have to give 
        // the player the ability to remove it from its stuck position. We select the space key for freeing the ball from the paddle
        if( this->Keys[GLFW_KEY_SPACE] ) {

            Ball->Stuck = false;

        }
    
    }

}

/*
 We check if the right side of the first object is greater than the left side of the second
 object and if the second object's right side is greater than the first object's left side; similarly for the vertical axis.
*/
bool Game::CheckCollision( GameObject &one, GameObject &two ) { // AABB - AABB collision
    
    // Collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    // Collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;
    // Collision only if on both axes
    return collisionX && collisionY;

}  

/*
 An overloaded function for CheckCollision was created that specifically deals with the case
 between a BallObject and a GameObject. Because we did not store the collision shape information
 in the objects themselves we have to calculate them: first the center of the ball is calculated,
 then the AABB's half-extents and its center. Using these collision shape attributes we calculate
 vector D¯ as difference that we then clamp to clamped and add to the AABB's center to get point
 P¯ as closest. Then we calculate the difference vector D′¯ between center and closest 
 and return whether the two shapes collided or not. 
*/
Collision Game::CheckCollision( BallObject &one, GameObject &two ) { // AABB - Circle collision
    
    // Get center point circle first 
    glm::vec2 center( one.Position + one.Radius );
    // Calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents( two.Size.x / 2, two.Size.y / 2 );
    glm::vec2 aabb_center(
        
        two.Position.x + aabb_half_extents.x, 
        two.Position.y + aabb_half_extents.y
    
    );
    // Get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp( difference, -aabb_half_extents, aabb_half_extents );
    // Add clamped value to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // Retrieve vector between center circle and closest point AABB and check 
    // if length <= radius
    difference = closest - center;
    
    // return direction and the deffirence vector
    if( glm::length( difference ) <= one.Radius )
        return std::make_tuple( true, VectorDirection( difference ), difference );
    else
        return std::make_tuple( false, UP, glm::vec2( 0.0f, 0.0f ) );

}      

void Game::Render() {
    //auto face = ResourceManager::GetTexture("face");
    //Renderer->DrawSprite(face, glm::vec2(200, 200), glm::vec2(300, 400), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    if( this->State == GAME_ACTIVE ) {
        
        Effects->BeginRender();

        auto background = ResourceManager::GetTexture( "background" );

        // Draw background
        Renderer->DrawSprite( background, glm::vec2( 0, 0 ),
            glm::vec2( this->Width, this->Height ), 0.0f );
        // Draw level
        this->Levels[this->Level].Draw( *Renderer );
        // draw the player
        Player->Draw( *Renderer );

        // draw powerups
        for( PowerUp &powerUp : this->PowerUps ) {

            if( !powerUp.Destroyed ) {

                powerUp.Draw( *Renderer );

            }

        }
        // Note that we render the particles before the ball is rendered and after the 
        // other item are rendered so the particles will end up in front of all other objects
        Particles->Draw();

        Ball->Draw( *Renderer );

        // Did ball reach bottom edge?
        if( Ball->Position.y > this->Height ) {

            this->ResetLevel();
            this->ResetPlayer();

        }
        Effects->EndRender();

        Effects->Render( ( float )glfwGetTime() );
    
    }

}

GLboolean ShouldSpawn( unsigned int chance ) {
    
    unsigned int random = rand() % chance;
    
    return random == 0;

}

void ActivatePowerUp( PowerUp &powerUp ) {
    
    // Initiate a powerup based type of powerup
    if( powerUp.Type == "speed" ) {
        
        Ball->Velocity *= 1.2;
    
    } else if( powerUp.Type == "sticky" ) {
        
        Ball->Sticky = false;
        Player->Color = glm::vec3( 1.0f, 0.5f, 1.0f );
    
    } else if( powerUp.Type == "pass-through" ) {
        
        Ball->PassThrough = true;
        Ball->Color = glm::vec3( 1.0f, 0.5f, 0.5f );
    
    } else if( powerUp.Type == "pad-size-increase" ) {
        
        Player->Size.x += 50;
    
    } else if( powerUp.Type == "confuse" ) {
        
        if( !Effects->Chaos )
            Effects->Confuse = true; // Only activate if chaos wasn't already active
    
    } else if( powerUp.Type == "chaos" ) {
        
        if( !Effects->Confuse )
            Effects->Chaos = true;
    
    }

} 

/*
 The SpawnPowerUps function creates a new PowerUp object based on a given chance (1 in 75
 for normal powerups and 1 in 15 for negative powerups) and sets their properties. Each 
 powerup is given a specific color to make them more recognizable for the user and a duration
 in seconds based on its type; here a duration of 0.0f means its duration is infinite. Additionally,
 each powerup is given the position of the destroyed block and one of the textures of the previous section.
*/
void Game::SpawnPowerUps( GameObject &block ) {
    
    if( ShouldSpawn( 75 ) ) // 1 in 75 chance
        this->PowerUps.push_back(
             PowerUp( "speed", glm::vec3( 0.5f, 0.5f, 1.0f ), 0.0f, block.Position,
              ResourceManager::GetTexture( "powerup_speed" )
         ) );
    
    if( ShouldSpawn( 75 ) )
        this->PowerUps.push_back(
            PowerUp( "sticky", glm::vec3( 1.0f, 0.5f, 1.0f ), 20.0f, block.Position,
            ResourceManager::GetTexture( "powerup_sticky" ) 
        ) );
    
    if( ShouldSpawn( 75 ) )
        this->PowerUps.push_back(
            PowerUp( "pass-through", glm::vec3( 0.5f, 1.0f, 0.5f ), 10.0f, block.Position,
            ResourceManager::GetTexture( "powerup_passthrough" )
        ) );
    
    if( ShouldSpawn( 75 ) )
        this->PowerUps.push_back(
            PowerUp( "pad-size-increase", glm::vec3( 1.0f, 0.6f, 0.4 ), 0.0f, block.Position,
            ResourceManager::GetTexture( "powerup_increase" )   
        ) );
    
    if( ShouldSpawn( 15 ) ) // Negative powerups should spawn more often
        this->PowerUps.push_back(
            PowerUp( "confuse", glm::vec3( 1.0f, 0.3f, 0.3f ), 15.0f, block.Position,
            ResourceManager::GetTexture( "powerup_confuse" )
        ) );
    
    if( ShouldSpawn( 15 ) )
        this->PowerUps.push_back(
            PowerUp( "chaos", glm::vec3( 0.9f, 0.25f, 0.25f ), 15.0f, block.Position,
            ResourceManager::GetTexture( "powerup_chaos" )
        ) );

}  

void Game::DoCollisions() {

    // for every brick in level bricks
    for( GameObject &box : this->Levels[this->Level].Bricks ) {
        
        // if is not destroyed, check for collision between the ball and the brick
        // and destory it if is collided 
        if( !box.Destroyed ) {

            Collision collistion = CheckCollision( *Ball, box );
            /* 
             if colistion, First we check for a collision and if so we destroy the block 
             if it is non-solid. Then we obtain the collision direction dir and the vector V¯ 
             as diff_vector from the tuple and finally do the collision resolution. We first
             check if the collision direction is either horizontal or vertical and then reverse
             the velocity accordingly. If horizontal, we calculate the penetration value R from
             the diff_vector's x component and either add or subtract this from the ball's 
             position based on its direction. The same applies to the vertical collisions, 
             but this time we operate on the y component of all the vectors. 
            */ 
            if( std::get<0>( collistion ) ) {

                if( !box.IsSolid ) { // Destroy block if not solid
                    
                    box.Destroyed = true;
                    this->SpawnPowerUps( box );
                
                } else {

                    // if block is solid, enable shake effect
                    ShakeTime = 0.05f;
                    Effects->Shake = true;

                }
                
                Direction dir = std::get<1>( collistion );

                glm::vec2 diff_vector = std::get<2>( collistion );

                 // don't do collision resolution on non-solid bricks if pass-through activated
                if( !( Ball->PassThrough && !box.IsSolid ) ) {
                
                    // horizontal collision
                    if( dir == LEFT or dir == RIGHT ) {

                        Ball->Velocity.x = -Ball->Velocity.x; // Reverse horizontal velocity
                        // Relocate
                        float penetration = Ball->Radius - std::abs( diff_vector.x );
                        
                        if( dir == LEFT )
                            Ball->Position.x += penetration; // Move ball to right
                        else
                            Ball->Position.x -= penetration; // Move ball to left;

                    } else {    // Vertical collision

                        Ball->Velocity.y = -Ball->Velocity.y; // Reverse vertical velocity
                        // Relocate
                        float penetration = Ball->Radius - std::abs( diff_vector.y );
                        if( dir == UP )
                            Ball->Position.y -= penetration; // Move ball back up
                        else
                            Ball->Position.y += penetration; // Move ball back down

                    }

                }

            }
        
        }
    
    }

    Collision result = CheckCollision( *Ball, *Player );
    /*
     If so (and the ball is not stuck to the paddle) we calculate the percentage of how far
     the ball's center is removed from the paddle's center compared to the half-extent
     of the paddle. The horizontal velocity of the ball is then updated based on the distance
     it hit the paddle from its center. Aside from updating the horizontal velocity we 
     also have to reverse the y velocity.
     Note that the old velocity is stored as oldVelocity. The reason for storing the old 
     velocity is that we only update the horizontal velocity of the ball's velocity vector
     while keeping its y velocity constant. This would mean that the length of the vector 
     constantly changes which has the effect that the ball's velocity vector is much larger
     (and thus stronger) if the ball hit the edge of the paddle compared to if the ball would
     hit the center of the paddle. For this reason the new velocity vector is normalized and multiplied
     by the length of the old velocity vector. This way, the strength and thus the velocity of the ball
     is always consistent, regardless of where it hits the paddle.
    */
    if( !Ball->Stuck and std::get<0>( result ) ) {

        // Check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = Player->Position.x + Player->Size.x / 2;
        float distance = ( Ball->Position.x + Ball->Radius ) - centerBoard;
        float percentage = distance / ( Player->Size.x / 2 );
        // Then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
        //Ball->Velocity.y = -Ball->Velocity.y;
        /*
         a small hack which is possible due to the fact that the we can assume we 
         always have a collision at the top of the paddle. Instead of reversing
         the y velocity we simply always return a positive y direction so whenever
         it does get stuck, it will immediately break free
        */
        Ball->Velocity.y = -1 * abs( Ball->Velocity.y );  
        Ball->Velocity = glm::normalize( Ball->Velocity ) * glm::length( oldVelocity );

        /*
         Here we set the ball's Stuck property equal to the ball's Sticky property. If the
         sticky effect is activated, the ball will end up stuck to the player paddle whenever
         it collides; the user then has to press the spacebar again to release the ball.
        */
        Ball->Stuck = Ball->Sticky; 

    }

    /*
     all powerups not yet destroyed, we check if the powerup either reached the bottom edge
     of the screen or collided with the paddle. In both cases, the powerup is destroyed,
     but when collided with the paddle, it is also activated.
    */
    for( PowerUp &powerUp : this->PowerUps ) {
        
        if( !powerUp.Destroyed ) {

            if( powerUp.Position.y >= this->Height ) {

                powerUp.Destroyed = true;

            }

            if( CheckCollision( *Player, powerUp ) ) {

                // Collided with player, now activate powerup
                ActivatePowerUp( powerUp );
                powerUp.Destroyed = true;
                powerUp.Activated = true;

            }

        }

    }
    

}

/*
 that while one of the powerup effects is active, another powerup of the same type collides
 with the player paddle. In that case we have more than 1 powerup of that type currently 
 active within the game's PowerUps vector. Then, whenever one of these powerups gets deactivated,
 we don't want to disable its effects yet since another powerup of the same type might still be active.
 For this reason we use the IsOtherPowerUpActive function to check if there is still another powerup 
 active of the same type. Only if this function returns false we deactivate the powerup. This way, 
 the powerup's duration of a given type is extended to the duration of its last activated powerup
*/
bool IsOtherPowerUpActive( std::vector<PowerUp> &powerUps, std::string type ) {

    for( const PowerUp &powerUp : powerUps ) {
        // function simply checks for all activated powerups if there is still any powerup
        // active of the same type and if so returns true    
        if( powerUp.Activated )
            if( powerUp.Type == type )
                return true;
    }
    
    return false;

}  

/*
 see that for each effect we disable it by resetting the relevant items to their original state.
 We also set the powerup's Activated property to false. At the end of the UpdatePowerUps we then 
 loop through the PowerUps vector and erase each powerup if they are destroyed and deactivated. 
 We use the remove_if function from the algorithm header to erase these items given a lambda predicate
*/
void Game::UpdatePowerUps( float dt ) {
    
    for( PowerUp &powerUp : this->PowerUps ) {
        
        powerUp.Position += powerUp.Velocity * dt;
        
        if( powerUp.Activated ) {
            
            powerUp.Duration -= dt;

            if( powerUp.Duration <= 0.0f ) {
                
                // Remove powerup from list (will later be removed)
                powerUp.Activated = false;
                // Deactivate effects
                if( powerUp.Type == "sticky" ) {
                    
                    if( !IsOtherPowerUpActive( this->PowerUps, "sticky" ) ) {	// Only reset if no other PowerUp of type sticky is active
                        
                        if( Ball->Sticky != Ball->Stuck )
                            Ball->Sticky = true;
                        Player->Color = glm::vec3( 1.0f );
                    
                    }
                
                } else if( powerUp.Type == "pass-through" ) {
                    
                    if( !IsOtherPowerUpActive( this->PowerUps, "pass-through" ) ) {	// Only reset if no other PowerUp of type pass-through is active
                        
                        Ball->PassThrough = false;
                        Ball->Color = glm::vec3( 1.0f );
                    
                    }
                
                } else if( powerUp.Type == "confuse" ) {
                    
                    if( !IsOtherPowerUpActive( this->PowerUps, "confuse" ) ) {	// Only reset if no other PowerUp of type confuse is active
                        
                        Effects->Confuse = false;
                    
                    }
                
                } else if( powerUp.Type == "chaos" ) {
                    
                    if( !IsOtherPowerUpActive( this->PowerUps, "chaos" ) ) {	// Only reset if no other PowerUp of type chaos is active
                        
                        Effects->Chaos = false;
                    
                    }
                
                }                
            
            }
        
        }
    
    }
    /*
     The remove_if function moves all elements for which the lambda predicate is true to the 
     end of the container object and returns an iterator to the start of this removed elements
     range. The container's erase function then takes this iterator and the vector's end iterator
     to remove all the elements between these two iterators
    */   
    this->PowerUps.erase( std::remove_if( this->PowerUps.begin(), this->PowerUps.end(),
            []( const PowerUp &powerUp ) { return powerUp.Destroyed && !powerUp.Activated; }
            ), this->PowerUps.end() );

}  

/*
 The function compares target to each of the direction vectors in the compass array. 
 The compass vector target is closest to in angle, is the direction returned to the function caller
*/
Direction Game::VectorDirection( glm::vec2 target ) {

    glm::vec2 compass[] = {
        
        glm::vec2( 0.0f, 1.0f ),	// up
        glm::vec2( 1.0f, 0.0f ),	// right
        glm::vec2( 0.0f, -1.0f ),	// down
        glm::vec2( -1.0f, 0.0f )	// left
    };
    
    float max = 0.0f;
    int best_match = -1;

    for( unsigned int i = 0; i < 4; i++ ) {

        float dot_product = glm::dot( glm::normalize( target ), compass[i] );

        if( dot_product > max ) {

            max = dot_product;
            best_match = i;

        }

    }

    return ( Direction )best_match;

}

void Game::ResetLevel() {
    
    if( this->Level == 0 ) { 
        
        this->Levels[0].Load( "levels/one.lvl", this->Width, this->Height * 0.5f );
        
        for( PowerUp &powerUp : this->PowerUps ) {

            powerUp.Destroyed = true;
            powerUp.Activated = false;

        }
    
    } else if( this->Level == 1 ) {
        
        this->Levels[1].Load( "levels/two.lvl", this->Width, this->Height * 0.5f );

        for( PowerUp &powerUp : this->PowerUps ) {

            powerUp.Destroyed = true;
            powerUp.Activated = false;

        }
    
    } else if( this->Level == 2 ) {
        
        this->Levels[2].Load( "levels/three.lvl", this->Width, this->Height * 0.5f );

        for( PowerUp &powerUp : this->PowerUps ) {

            powerUp.Destroyed = true;
            powerUp.Activated = false;

        }
    
    } else if( this->Level == 3 ) {
        
        this->Levels[3].Load( "levels/four.lvl", this->Width, this->Height * 0.5f );

        for( PowerUp &powerUp : this->PowerUps ) {

            powerUp.Destroyed = true;
            powerUp.Activated = false;

        }
    
    }

}

void Game::ResetPlayer() {
    
    // Reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2( this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y );
    Ball->Reset( Player->Position + glm::vec2( PLAYER_SIZE.x / 2 - BALL_RADIUS, -( BALL_RADIUS * 2 ) ), INITIAL_BALL_VELOCITY );
    // Also disable all active powerups
    Effects->Chaos = Effects->Confuse = false;
    Ball->PassThrough = Ball->Sticky = false;
    Player->Color = glm::vec3( 1.0f );
    Ball->Color = glm::vec3( 1.0f );

}