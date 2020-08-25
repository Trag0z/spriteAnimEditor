#version 330 core
in vec2 uv_coord;

out vec4 frag_color;

uniform sampler2D texture1;
uniform vec2 sprite_dimensions;
uniform vec2 sprite_position_on_sheet;

void main()
{
    ivec2 sheet_dimensions=textureSize(texture1,0);
    vec2 sprite_uv_dimensions=sprite_dimensions/vec2(sheet_dimensions);
    
    vec2 new_uv_coord=(sprite_position_on_sheet+uv_coord)*sprite_uv_dimensions;
    frag_color=texture(texture1,new_uv_coord);
}