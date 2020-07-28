#version 330 core
layout(location=0)in vec2 pos;
layout(location=1)in vec2 in_uv_coord;

out vec2 uv_coord;

uniform vec2 render_position;

void main()
{
    uv_coord=in_uv_coord;
    gl_Position=vec4(render_position+pos,0.,1.);
}