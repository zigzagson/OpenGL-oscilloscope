#version 330 core
out vec4 FragColor;

uniform vec3 setColor;

void main()
{
    FragColor = vec4(setColor, 1.0f);
}