#include "include/common.h"
#include "include/shading-options.h"

#ensuredefined LANDSCAPE_LAYERS_COUNT 1
#ensuredefined DRAW_COPY_PASTE 0
#ensuredefined DRAW_TYPE 0

fragment_in
{
    float2 uv : TEXCOORD0;
};

fragment_out
{
    float4 color0 : SV_TARGET0;
#if LANDSCAPE_LAYERS_COUNT > 1
    float4 color1 : SV_TARGET1;
#endif
#if LANDSCAPE_LAYERS_COUNT > 2
    float4 color2 : SV_TARGET2;
#endif
#if LANDSCAPE_LAYERS_COUNT > 3
    float4 color3 : SV_TARGET3;
#endif
};

uniform sampler2D sourceTexture0;
#if LANDSCAPE_LAYERS_COUNT > 1
uniform sampler2D sourceTexture1;
#endif
#if LANDSCAPE_LAYERS_COUNT > 2
uniform sampler2D sourceTexture2;
#endif
#if LANDSCAPE_LAYERS_COUNT > 3
uniform sampler2D sourceTexture3;
#endif

uniform sampler2D toolTexture;

[material][instance] property float intensity = 1.0;
[material][instance] property float intensitySign = 1.0;
[material][instance] property float4 drawMask0 = float4(0.0, 0.0, 0.0, 0.0);
#if LANDSCAPE_LAYERS_COUNT > 1
[material][instance] property float4 drawMask1 = float4(0.0, 0.0, 0.0, 0.0);
#endif
#if LANDSCAPE_LAYERS_COUNT > 2
[material][instance] property float4 drawMask2 = float4(0.0, 0.0, 0.0, 0.0);
#endif
#if LANDSCAPE_LAYERS_COUNT > 3
[material][instance] property float4 drawMask3 = float4(0.0, 0.0, 0.0, 0.0);
#endif

[material][instance] property float2 copypasteOffset = float2(0.0, 0.0);

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float4 outColor0 = tex2D(sourceTexture0, input.uv);

#if LANDSCAPE_LAYERS_COUNT > 1
    float4 outColor1 = tex2D(sourceTexture1, input.uv);
#endif
#if LANDSCAPE_LAYERS_COUNT > 2
    float4 outColor2 = tex2D(sourceTexture2, input.uv);
#endif
#if LANDSCAPE_LAYERS_COUNT > 3
    float4 outColor3 = tex2D(sourceTexture3, input.uv);
#endif

    float toolValue = tex2D(toolTexture, input.uv).r;

#if DRAW_COPY_PASTE

    float4 sourceColor0 = tex2D(sourceTexture0, input.uv + copypasteOffset);
    outColor0 = lerp(outColor0, sourceColor0, toolValue);
#if LANDSCAPE_LAYERS_COUNT > 1
    float4 sourceColor1 = tex2D(sourceTexture1, input.uv + copypasteOffset);
    outColor1 = lerp(outColor1, sourceColor1, toolValue);
#endif
#if LANDSCAPE_LAYERS_COUNT > 2
    float4 sourceColor2 = tex2D(sourceTexture2, input.uv + copypasteOffset);
    outColor2 = lerp(outColor2, sourceColor2, toolValue);
#endif
#if LANDSCAPE_LAYERS_COUNT > 3
    float4 sourceColor3 = tex2D(sourceTexture3, input.uv + copypasteOffset);
    outColor3 = lerp(outColor3, sourceColor3, toolValue);
#endif

#else

    outColor0 += drawMask0 * toolValue * (intensity * intensitySign);
#if LANDSCAPE_LAYERS_COUNT > 1
    outColor1 += drawMask1 * toolValue * (intensity * intensitySign);
#endif
#if LANDSCAPE_LAYERS_COUNT > 2
    outColor2 += drawMask2 * toolValue * (intensity * intensitySign);
#endif
#if LANDSCAPE_LAYERS_COUNT > 3
    outColor3 += drawMask3 * toolValue * (intensity * intensitySign);
#endif

#endif

    output.color0 = min(outColor0, float4(1.0, 1.0, 1.0, 1.0));
#if LANDSCAPE_LAYERS_COUNT > 1
    output.color1 = min(outColor1, float4(1.0, 1.0, 1.0, 1.0));
#endif
#if LANDSCAPE_LAYERS_COUNT > 2
    output.color2 = min(outColor2, float4(1.0, 1.0, 1.0, 1.0));
#endif
#if LANDSCAPE_LAYERS_COUNT > 3
    output.color3 = min(outColor3, float4(1.0, 1.0, 1.0, 1.0));
#endif
    return output;
}
