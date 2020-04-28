#version 430 core

layout(location = 0) in vec2 _inPosition;

void main()
{
    gl_Position = vec4(_inPosition, 0.0, 1.0f);
}