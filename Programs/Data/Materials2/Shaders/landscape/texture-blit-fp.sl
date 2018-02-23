#include "include/common.h"

#ensuredefined TEXTURE_COUNT 1
#ensuredefined REMAP_TEX_COORD 0

fragment_in
{
    float2 texCoord : TEXCOORD0;
};

fragment_out
{
    float4 target0 : SV_TARGET0;

#if TEXTURE_COUNT > 1
    float4 target1 : SV_TARGET1;
#endif

#if TEXTURE_COUNT > 2
    float4 target2 : SV_TARGET2;
#endif

#if TEXTURE_COUNT > 3
    float4 target3 : SV_TARGET3;
#endif
};

uniform sampler2D dynamicTextureSrc0;

#if TEXTURE_COUNT > 1
uniform sampler2D dynamicTextureSrc1;
#endif
#if TEXTURE_COUNT > 2
uniform sampler2D dynamicTextureSrc2;
#endif
#if TEXTURE_COUNT > 3
uniform sampler2D dynamicTextureSrc3;
#endif

#if REMAP_TEX_COORD
[material][a] property float4 remapRect;
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

#if REMAP_TEX_COORD
    float2 texCoord;
    texCoord.x = lerp(remapRect.x, remapRect.y, input.texCoord.x);
    texCoord.y = lerp(remapRect.z, remapRect.w, input.texCoord.y);
#else
    float2 texCoord = input.texCoord;
#endif

    output.target0 = tex2D(dynamicTextureSrc0, texCoord);

#if TEXTURE_COUNT > 1
    output.target1 = tex2D(dynamicTextureSrc1, texCoord);
#endif

#if TEXTURE_COUNT > 2
    output.target2 = tex2D(dynamicTextureSrc2, texCoord);
#endif

#if TEXTURE_COUNT > 3
    output.target3 = tex2D(dynamicTextureSrc3, texCoord);
#endif

    return output;
}
