#version 330 core
layout(location=0)in vec2 pos;
layout(location=1)in vec2 in_uv_coord;

out vec2 uv_coord;

uniform vec2 render_position;
uniform vec2 sprite_dimensions;
uniform mat4 projection;

void main()
{
    uv_coord=in_uv_coord;
    gl_Position=projection*vec4(pos.x*sprite_dimensions.x+render_position.x,pos.y*sprite_dimensions.y+render_position.y,0.,1.);
}