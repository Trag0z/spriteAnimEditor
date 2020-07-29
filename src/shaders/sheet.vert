#version 330 core
layout(location=0)in vec3 pos;
layout(location=1)in vec2 in_uv_coord;

out vec2 uv_coord;

void main()
{
    uv_coord=in_uv_coord;
    gl_Position=vec4(pos.x,pos.y,0.,1.);
}