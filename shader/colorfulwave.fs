#version 330 core
out vec4 FragColor;
in float colorGrade;

uniform vec3 backgroundColor;
uniform vec3 beginColor;
uniform vec3 secondColor;
uniform vec3 thirdColor;
uniform vec3 fourthColor;
uniform vec3 endColor;
uniform int maxGrade;

vec3 calculateColorFromGrade(float grade)
{
    vec3 stepColor;
    vec3 resultColor;
    int cgrade=int(grade);
    int mgrade=maxGrade/4;
    cgrade=(cgrade>maxGrade)?maxGrade:cgrade;
    if(cgrade<mgrade)
    {
        stepColor=(secondColor-beginColor)/maxGrade*4;
        resultColor=beginColor+cgrade*stepColor;
    }
    else if(cgrade<mgrade*2)
    {
        stepColor=(thirdColor-secondColor)/maxGrade*4;
        resultColor=secondColor+(cgrade-mgrade)*stepColor;
    }
    else if(cgrade<mgrade*3)
    {
        stepColor=(fourthColor-thirdColor)/maxGrade*4;
        resultColor=thirdColor+(cgrade-mgrade*2)*stepColor;
    }
    else
    {
        stepColor=(endColor-fourthColor)/maxGrade*4;
        resultColor=fourthColor+(cgrade-mgrade*3)*stepColor;
    }
    if(cgrade==0)
    {
        resultColor=backgroundColor;
    }
    return resultColor;
}

void main()
{
    FragColor=vec4(calculateColorFromGrade(colorGrade),1.f);
}