#version 330 core
layout(location=0)in vec3 aPos;

uniform mat4 projection;
out float colorGrade;

void main()
{
    gl_Position=projection*vec4(aPos.xy,0.f,1.f);
    colorGrade=aPos.z;
}