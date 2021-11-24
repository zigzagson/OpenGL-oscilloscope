#version 330 core
out vec4 FragColor;

uniform vec4 setColor;

void main()
{
    FragColor = setColor;
}