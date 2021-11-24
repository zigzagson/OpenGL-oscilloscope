#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec4 TexColor;
uniform sampler2D TexSampler;

void main()
{
	if(TexColor.w == 0.0f)
		FragColor = texture(TexSampler, TexCoord);
	else
	{
    	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(TexSampler, TexCoord).a);
    	FragColor = TexColor * sampled;
	}
}