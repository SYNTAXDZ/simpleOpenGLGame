#include "ball_object.h"

BallObject::BallObject() : GameObject(), Radius( 12.5f ), Stuck( true ) { }

BallObject::BallObject( glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite )
                    : GameObject( pos, glm::vec2( radius * 2, radius * 2 ), sprite, glm::vec3( 1.0f ), velocity ),
                      Radius( radius ), Stuck( true ) { }


// Move function that moves the ball based on its velocity and checks if it reaches any of the scene's edges and if so reverses the ball's velocity
glm::vec2 BallObject::Move( float dt, unsigned int window_width ) {

    // if the player to stuck
    if( !this->Stuck ) {

        this->Position += this->Velocity * dt;

        if( this->Position.x < 0.0f ) {

            this->Velocity.x *= -1;
            this->Position.x = 0.0f;

        } else if( this->Position.x + this->Size.x >= window_width ) {

            this->Velocity.x *= -1;
            this->Position.x = window_width - this->Size.x;

        }

        if( this->Position.y <= 0.0f ) {

            this->Velocity.y *= -1;
            this->Position.y = 0.0f;

        }

    }

    return this->Position;

}

// Resets the ball to initial Stuck Position (if ball is outside window bounds)
void BallObject::Reset( glm::vec2 position, glm::vec2 velocity ) {
    
    this->Position = position;
    this->Velocity = velocity;
    this->Stuck = true;

}

