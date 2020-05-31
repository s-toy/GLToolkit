#version 460 core

layout(location = 0) in vec2 _position;

layout(location = 0) out vec2 _fragCoord;

const vec2 coords[4] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } };

void main() 
{
    gl_Position = vec4(_position, 0.0, 1.0f);
    _fragCoord = coords[gl_VertexID];
}