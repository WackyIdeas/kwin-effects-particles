#version 140

in vec2 uv;
in vec4 particlecolor;

out vec4 fragColor;

uniform sampler2D sampler;
uniform bool useTexture;

void main()
{
    vec4 texCol = texture(sampler, uv);
    fragColor = particlecolor;
    if(useTexture)
    {
        fragColor = texCol * particlecolor;
    }
    else
    {
        fragColor = particlecolor;
    }
}
