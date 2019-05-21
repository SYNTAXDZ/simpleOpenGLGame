#version 330 core

in vec2 TexCoords;
in vec4 ParticleColor;

uniform sampler2D sprite;

out vec4 color;

void main() {

    color = ( texture( sprite, TexCoords ) * ParticleColor );

}