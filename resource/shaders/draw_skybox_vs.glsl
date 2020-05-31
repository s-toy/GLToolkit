#version 460 core

layout(location = 0) in vec3 _inPosition;

layout(location = 0) out vec3 _outTexCoords;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;

void main()
{
    _outTexCoords = _inPosition;
    gl_Position = uProjectionMatrix * mat4(mat3(uViewMatrix)) * vec4(_inPosition, 1.0);
}