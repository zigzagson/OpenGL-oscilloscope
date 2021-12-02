#version 330 core

in vec2 vertPos;
out vec4 FragColor;

uniform uvec3 type;
uniform vec3 setColor;

void main()
{
    uint bit;
    vec2 Pos = vertPos;
    Pos.y += 0.2f;
    if(type.y == 0U&&type.z == 0U)
    {
        if(type.x == 0U)
            bit = uint((Pos.x+1.001f)*300) % 10U;
        else
            bit = uint((Pos.y+1.001f)*250) % 10U;
        
        if((248U & (1U << bit)) != 0U)
            discard;
    }
    if(type.y == 1U&&type.z == 0U)
    {
        if(type.x == 0U)
            bit = uint((Pos.x+1.001f)*300) % 50U;
        else
            bit = uint((Pos.y+1.001f)*250) % 50U;

        if(bit!=0U&&bit!=1U&&bit!=49U)
            discard;
    }
    FragColor = vec4(setColor, 0.0f);
    if(type.z==1U)
        FragColor = vec4(setColor, 0.0f);
}