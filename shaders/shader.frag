#version 140
#include "kwin-effects-particleeffect/brightnessadjust.frag"

in vec2 uv;
in vec4 particlecolor;

out vec4 fragColor;

uniform sampler2D sampler;
uniform bool useTexture;

void main()
{
    vec4 texCol = texture(sampler, uv);
    vec4 result = particlecolor;
    if(useTexture)
    {
        result = texCol * particlecolor;
    }
    else
    {
        result = particlecolor;
    }
    fragColor = adjustBrightness(result);
}
