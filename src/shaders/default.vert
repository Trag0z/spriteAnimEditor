#version 330 core
layout(location=0)in vec2 pos;
layout(location=1)in vec2 in_uv_coord;

out vec2 uv_coord;

uniform sampler2D texture1;
uniform vec2 render_position;
uniform mat4 projection;

void main()
{
    uv_coord=in_uv_coord;
    ivec2 sheet_dimensions=textureSize(texture1,0);
    gl_Position=projection*vec4(render_position.x+pos.x*float(sheet_dimensions.x),pos.y*float(sheet_dimensions.y)+render_position.y,0.,1.);
}