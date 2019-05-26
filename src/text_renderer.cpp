#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "text_renderer.h"
#include "resource_manager.h"


TextRenderer::TextRenderer( unsigned int width, unsigned int height ) {
    // Load and configure shader
    this->TextShader = ResourceManager::LoadShader( "shaders/text.vert", "shaders/text.frag", nullptr, "text" );
    this->TextShader.SetMatrix4( "projection", glm::ortho( 0.0f, static_cast<float>( width ), static_cast<float>( height ), 0.0f ), GL_TRUE );
    this->TextShader.SetInteger( "text", 0 );
    // Configure VAO/VBO for texture quads
    glGenVertexArrays( 1, &this->VAO );
    glGenBuffers( 1, &this->VBO );
    glBindVertexArray( this->VAO );
    glBindBuffer( GL_ARRAY_BUFFER, this->VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( float ) * 6 * 4, NULL, GL_DYNAMIC_DRAW );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

}

void TextRenderer::Load( std::string font, unsigned int fontSize ) {
    // First clear the previously loaded Characters
    this->Characters.clear();
    // Then initialize and load the FreeType library
    FT_Library ft;    
    if( FT_Init_FreeType( &ft ) ) // All functions return a value different than 0 whenever an error occurred
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    // Load font as face
    FT_Face face;
    if( FT_New_Face( ft, font.c_str(), 0, &face ) )
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    // Set size to load glyphs as
    FT_Set_Pixel_Sizes( face, 0, fontSize );
    // Disable byte-alignment restriction
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); 
    // Then for the first 128 ASCII characters, pre-load/compile their characters and store them
    for( GLubyte c = 0; c < 128; c++ ) { // lol see what I did there 
        // Load character glyph 
        if( FT_Load_Char( face, c, FT_LOAD_RENDER ) ) {
            
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            
            continue;
        
        }
        // Generate texture
        unsigned int texture;
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
            );
        // Set texture options
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
       
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2( face->glyph->bitmap.width, face->glyph->bitmap.rows ),
            glm::ivec2( face->glyph->bitmap_left, face->glyph->bitmap_top ),
            face->glyph->advance.x
        };
        
        Characters.insert( std::pair<GLchar, Character>( c, character ) );
    
    }
    glBindTexture( GL_TEXTURE_2D, 0 );
    // Destroy FreeType once we're finished
    FT_Done_Face( face );
    FT_Done_FreeType( ft );

}

/*
 Since we now render downwards from RenderText's y parameter we calculate the vertical offset
 as the distance a glyph is pushed downwards from the top of the glyph space.
*/
void TextRenderer::RenderText( std::string text, float x, float y, float scale, glm::vec3 color ) {
    // Activate corresponding render state	
    this->TextShader.Use();
    this->TextShader.SetVector3f( "textColor", color );
    glActiveTexture( GL_TEXTURE0 );
    glBindVertexArray( this->VAO );

    // Iterate through all characters
    std::string::const_iterator c;
    for( c = text.begin(); c != text.end(); c++ ) {
        
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        /*
         calculate this vertical offset we need to get the top of the glyph space (basically the 
         length of the black vertical arrow from the origin). Unfortunately, FreeType has no such 
         metric for us. What we do know is that that some glyphs always touch this top edge; 
         characters like 'H', 'T' or 'X'. So what if we calculate the length of this red vector by
         subtracting bearingY from any of these top-reaching glyphs by bearingY of the glyph in question.
          This way, we push the glyph down based on how far its top point differs from the top edge.
        */
        float ypos = y + ( this->Characters['H'].Bearing.y - ch.Bearing.y ) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 1.0 },
            { xpos + w, ypos,       1.0, 0.0 },
            { xpos,     ypos,       0.0, 0.0 },

            { xpos,     ypos + h,   0.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 1.0 },
            { xpos + w, ypos,       1.0, 0.0 }
        };
        // Render glyph texture over quad
        glBindTexture( GL_TEXTURE_2D, ch.TextureID );
        // Update content of VBO memory
        glBindBuffer( GL_ARRAY_BUFFER, this->VBO );
        glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( vertices ), vertices ); // Be sure to use glBufferSubData and not glBufferData

        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        // Render quad
        glDrawArrays( GL_TRIANGLES, 0, 6 );
        // Now advance cursors for next glyph
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
    
    }
    
    glBindVertexArray( 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );

}

