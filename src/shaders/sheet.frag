#version 330 core
in vec2 uv_coord;

out vec4 frag_color;

uniform sampler2D texture1;
uniform vec2 sprite_dimensions;
uniform int sprite_index;

void main()
{
    ivec2 sheet_dimensions=textureSize(texture1,0);
    vec2 sprite_uv_dimensions={sprite_dimensions.x/sheet_dimensions.x,sprite_dimensions.y/sheet_dimensions.y};
    
    vec2 new_uv_coord;
    new_uv_coord.x=sprite_uv_dimensions.x*float(sprite_index)+(uv_coord.x*sprite_uv_dimensions.x);
    new_uv_coord.y=1.-(sprite_uv_dimensions.y*float(sprite_index))+(uv_coord.y*sprite_uv_dimensions.y);
    frag_color=texture(texture1,new_uv_coord);
}