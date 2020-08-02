#version 330 core
layout(location=0)in vec2 pos;

uniform vec2 render_position;
uniform vec2 sprite_dimensions;
uniform mat4 projection;

void main()
{
    gl_Position=projection*vec4(pos.x*sprite_dimensions.x+render_position.x,pos.y*sprite_dimensions.y+render_position.y,0.,1.);
}