#version 330 core
layout (location = 0) in vec3 pos; 
layout (location = 1) in vec2 in_uv_coord;

out vec2 uv_coord;

uniform mat4 projection;
uniform mat4 model;
uniform ivec2 sprite_dimensions;

void main()
{
    uv_coord = in_uv_coord;
    gl_Position = projection * model * vec4(pos.x * sprite_dimensions.x * 0.5f, pos.y * sprite_dimensions.y * 0.5f, pos.z, 1.0);
}