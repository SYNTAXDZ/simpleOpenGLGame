#version 330 core
// <vec2 position, vec2 texCoords>
layout( location = 0 ) in vec4 vertex;

out vec2 TexCoords;

uniform bool chaos;
uniform bool shake;
uniform bool confuse;
uniform float time;

/*
 Based on whatever uniform is set to true the vertex shader can take different paths. If 
 either chaos or confuse is set to true the vertex shader will manipulate the texture 
 coordinates to move the scene around (either translate texture coordinates in a circle-like
 fashion or inverse the texture coordinates). Because we set the texture wrapping methods 
 to GL_REPEAT the chaos effect will cause the scene to repeat itself at various parts of 
 the quad. Additionally if shake is set to true it will move the vertex positions around by 
 only a small amount. Note that chaos and confuse shouldn't be true at the same time while 
 shake is able to work with any of the other effects on. 
*/
void main() {

    gl_Position = vec4( vertex.xy, 0.0f, 1.0f );
    vec2 texture = vertex.zw;

    if( chaos ) {

        float strength = 0.3f;
        vec2 pos = vec2( texture.x + sin( time ) * strength, texture.y + cos( time ) * strength );
        TexCoords = pos;

    } else if( confuse ) {

        TexCoords = vec2( 1.0f - texture.x, 1.0f - texture.y );

    } else {

        TexCoords = texture;

    }

    if( shake ) {

        float strength = 0.01f;

        gl_Position.x += cos( 10 * time ) * strength;
        gl_Position.y += cos( 15 * time ) * strength;

    }

}