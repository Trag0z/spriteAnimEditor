#version 330 core
in vec2 uv_coord;

out vec4 frag_color;

uniform sampler2D texture1;
uniform vec2 sprite_dimensions;
uniform vec2 sprite_position_on_sheet;

float modulus(float x,float y){
    return x-y*floor(x/y);
}

void main()
{
    ivec2 sheet_dimensions=textureSize(texture1,0);
    vec2 sprite_uv_dimensions={sprite_dimensions.x/sheet_dimensions.x,sprite_dimensions.y/sheet_dimensions.y};
    
    vec2 new_uv_coord=vec2(sprite_position_on_sheet.x*sprite_uv_dimensions.x,sprite_position_on_sheet.y*sprite_uv_dimensions.y);
    frag_color=texture(texture1,new_uv_coord);
}