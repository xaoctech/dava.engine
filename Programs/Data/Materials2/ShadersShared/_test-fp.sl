
color_mask = rgba;

blending
{
    src = one dst = one
}

fragment_in
{
    float4 p_pos : TEXCOORD0;
    float4 lightPosition : TEXCOORD1; //(pos.xyz, radius)
    float4 lightParams : TEXCOORD2; //(color.rgb, shadowIndex)
};

fragment_out
{
    float4 color : SV_TARGET0;
};

uniform sampler2D gBuffer0;
uniform sampler2D gBuffer1;
uniform sampler2D gBuffer2;

[auto][global] property float2 viewportSize;
[auto][global] property float2 viewportOffset;
[auto][global] property float2 renderTargetSize;
[material][instance] property float testprop;

float4 blablabla = float4(0.1, 0.2, 0.3, 0.4);

[material][jpos] property float4 colorsArr[10] : "bigarray"; // (x, y, z, scale)

float4 bar(float2 texturePos)
{
    float4 g2 = tex2D(gBuffer2, texturePos);
    float4 c7 = FramebufferFetch(7);
    float4 c17 = FramebufferFetch(17);

    return (g2 + c7 + c17) * 0.33;
}

float4 foo(float4 inVal, float2 texturePosition)
{
    float4 fbCol = FramebufferFetch(13);
    float4 g1 = tex2D(gBuffer1, texturePosition);
    return (inVal + fbCol + g1 + bar(texturePosition)) * 0.25;
}

fragment_out fp_main(fragment_in input)
{
    float3 ndcPos = input.p_pos.xyz / input.p_pos.w;
    float2 texPos = ndcPos.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
    texPos = (texPos * viewportSize + viewportOffset) / renderTargetSize;

    float4 g0 = tex2D(gBuffer0, texPos) + blablabla;
    float4 fbCol = FramebufferFetch(17);
    float4 fbCol2 = FramebufferFetch(3);

    float4 blendedCol = foo(fbCol + fbCol2, texPos);
    for (int i = 0; i < 10; ++i)
        blendedCol += colorsArr[i];

    fragment_out output;
    output.color = blendedCol;
    return output;
}
