#ensuredefined SHOW_DISTANCE 0

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

uniform sampler2D dynamicShadowmap;

fragment_out fp_main(fragment_in input)
{
    float sampledValue = tex2D(dynamicShadowmap, input.varTexCoord0).x;

    fragment_out output;
#if (SHOW_DISTANCE)
    output.color = exp(-0.01 * sampledValue);
#else
    output.color = pow(sampledValue, 16.0);
#endif
    return output;
}
