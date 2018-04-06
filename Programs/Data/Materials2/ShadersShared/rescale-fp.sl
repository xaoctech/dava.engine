

fragment_in
{
    float2 uv : TEXCOORD0;
};

fragment_out
{
    float4 target0 : SV_TARGET0;
};

uniform sampler2D dynamicTextureLdrCurrent;

fragment_out fp_main(fragment_in input)
{
    fragment_out output;
    output.target0 = tex2D(dynamicTextureLdrCurrent, input.uv);
    //output.target0 = float4(0.0, 1.0, 1.0, 1.0);
    return output;
}