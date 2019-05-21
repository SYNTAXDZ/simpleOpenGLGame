#include "particle_generator.h"

ParticleGenerator::ParticleGenerator( Shader shader, Texture2D texture, unsigned int amount )
    : shader( shader ), texture( texture ), amount( amount ) {
    
    this->init();

}

/*
 first loop might look a little daunting. Because particles die over time we want to spawn nr_new_particles
 particles each frame, but since we've decided from the start that the total amount of particles we'll be
 using is nr_particles we can't simply push the new particles to the end of the list. This way we'll quickly
 get a list filled with thousands of particles which isn't really efficient considering only a small portion
 of that list has particles that are alive
*/
void ParticleGenerator::Update( float dt, GameObject &object, unsigned int newParticles, glm::vec2 offset ) {
    
    // Add new particles 
    for( unsigned int i = 0; i < newParticles; ++i ) {
        
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle( this->particles[unusedParticle], object, offset );
    
    }
    /*
     The second loop within the update function loops through all the particles and for each particle
     reduces its life by the delta time variable; this way each particle's life corresponds to exactly
     the second(s) it's allowed to live. Then we check if the particle is alive and if so, update its 
     position and color attributes. Here we slowly reduce the alpha component of each particle so it looks
     like they're slowly disappearing over time
    */
    // Update all particles
    for( unsigned int i = 0; i < this->amount; ++i ) {
        
        Particle &p = this->particles[i];
        p.Life -= dt; // reduce life
        
        if( p.Life > 0.0f ) {	// particle is alive, thus update
            
            p.Position -= p.Velocity * dt; 
            p.Color.a -= dt * 2.5;
        
        }
    
    }

}

/*
 Here, for each particle, we set their offset and color uniform values, bind the texture 
 and render the 2D quad. What is interesting to see here are the two calls to glBlendFunc.
 When rendering the particles, instead of the default destination blending mode of GL_ONE_MINUS_SRC_ALPHA
 we use the GL_ONE blending mode that gives the particles a very neat glow effect when stacked upon each other.
*/
// Render all particles
void ParticleGenerator::Draw() {
    
    // Use additive blending to give it a 'glow' effect
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    this->shader.Use();
    for( Particle particle : this->particles ) {
        
        if( particle.Life > 0.0f ) {
            
            this->shader.SetVector2f( "offset", particle.Position );
            this->shader.SetVector4f( "color", particle.Color );
            this->texture.Bind();
            glBindVertexArray( this->VAO );
            glDrawArrays( GL_TRIANGLES, 0, 6 );
            glBindVertexArray( 0 );
        
        }
    
    }
    // Don't forget to reset to default blending mode
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

}

void ParticleGenerator::init() {
    
    // Set up mesh and attribute properties
    unsigned int VBO;
    float particle_quad[] = {
        
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    
    }; 
    glGenVertexArrays( 1, &this->VAO );
    glGenBuffers( 1, &VBO );
    glBindVertexArray( this->VAO );
    // Fill mesh buffer
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( particle_quad ), particle_quad, GL_STATIC_DRAW );
    // Set mesh attributes
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), ( void* ) 0 );
    glBindVertexArray( 0 );

    // Create this->amount default particle instances
    for( unsigned int i = 0; i < this->amount; ++i )
        this->particles.push_back( Particle() );

}

// Stores the index of the last particle used (for quick access to next dead particle)
unsigned int lastUsedParticle = 0;
/*
 first loop might look a little daunting. Because particles die over time we want to spawn
 nr_new_particles particles each frame, but since we've decided from the start that the total
 amount of particles we'll be using is nr_particles we can't simply push the new particles 
 to the end of the list. This way we'll quickly get a list filled with thousands of particles
 which isn't really efficient considering only a small portion of that list has particles that are alive
*/
unsigned int ParticleGenerator::firstUnusedParticle() {
    
    // First search from last used particle, this will usually return almost instantly
    for( unsigned int i = lastUsedParticle; i < this->amount; ++i ) {
        
        if( this->particles[i].Life <= 0.0f ) {
            
            lastUsedParticle = i;
            
            return i;
        
        }
    
    }
    // Otherwise, do a linear search
    for( unsigned int i = 0; i < lastUsedParticle; ++i ) {
        
        if( this->particles[i].Life <= 0.0f ) {
            
            lastUsedParticle = i;
            
            return i;
        
        }
    
    }
    // All particles are taken, override the first one (note that if it repeatedly hits this case,
    // more particles should be reserved)
    lastUsedParticle = 0;
    
    return 0;

}

/*
 once the first dead particle in the list is found, we update its values by calling RespawnParticle
 that takes the particle, a GameObject and an offset vector.
 This function simply resets the particle's life to 1.0f, randomly gives it a brightness (via the color vector)
 starting from 0.5 and assigns a (slightly random) position and velocity based on the game object.
*/
void ParticleGenerator::respawnParticle( Particle &particle, GameObject &object, glm::vec2 offset ) {
    
    float random = ( ( rand() % 100 ) - 50) / 10.0f;
    float rColor = 0.5 + ((rand() % 100) / 100.0f);
    particle.Position = object.Position + random + offset;
    particle.Color = glm::vec4( rColor, rColor, rColor, 1.0f );
    particle.Life = 1.0f;
    particle.Velocity = object.Velocity * 0.1f;

}