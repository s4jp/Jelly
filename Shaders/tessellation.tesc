#version 460 core

layout (vertices = 16) out;

void main() 
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    gl_TessLevelOuter[0] = 64.0;
    gl_TessLevelOuter[1] = 64.0;
    gl_TessLevelOuter[2] = 64.0;
    gl_TessLevelOuter[3] = 64.0;

    gl_TessLevelInner[0] = 64.0;
    gl_TessLevelInner[1] = 64.0;
}