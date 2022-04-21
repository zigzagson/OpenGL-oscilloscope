#version 330 core
out vec4 FragColor;
in float colorGrade;

uniform vec3 backgroundColor;
uniform vec3 beginColor;
uniform vec3 endColor;
uniform int maxGrade;

vec3 calculateColorFromGrade(float grade)
{
    vec3 stepColor=(endColor-beginColor)/maxGrade;
    int cgrade=int(grade);
    cgrade=(cgrade>maxGrade)?maxGrade:cgrade;
    if(cgrade==0)
    {
        return backgroundColor;
    }
    return beginColor+cgrade*stepColor;
}

void main()
{
    FragColor=vec4(calculateColorFromGrade(colorGrade),1.f);
}