uniform sampler2D albedo1;
uniform sampler2D albedo2;
uniform sampler2D albedo3;

uniform sampler2D normalmap1;
uniform sampler2D normalmap2;
uniform sampler2D normalmap3;

struct TexturesBlendResult
{
    float4 albedo;
    float4 normal;
};

TexturesBlendResult BlendTexturesVertexColor(float2 uvIn, float4 color)
{
    float2 uv = frac(uvIn);

    float2 uvddx = ddx(uvIn);
    float2 uvddy = ddy(uvIn);

    float4 diffuseColor0 = tex2Dgrad(albedo, uv, uvddx, uvddy);
    float4 diffuseColor1 = tex2Dgrad(albedo1, uv, uvddx, uvddy);
    diffuseColor0.w += color.x;
    diffuseColor1.w += color.y;
    float4 normalSample0 = tex2Dgrad(normalmap, uv, uvddx, uvddy);
    float4 normalSample1 = tex2Dgrad(normalmap1, uv, uvddx, uvddy);
    float weight1 = step(diffuseColor0.w, diffuseColor1.w);

    float4 d1 = lerp(diffuseColor0, diffuseColor1, weight1);
    float4 n1 = lerp(normalSample0, normalSample1, weight1);
    TexturesBlendResult res;

#if (VERTEX_BLEND_4_TEXTURES == 0)
    res.albedo = d1;
    res.normal = n1;
#else
    float4 diffuseColor2 = tex2Dgrad(albedo2, uv, uvddx, uvddy);
    float4 diffuseColor3 = tex2Dgrad(albedo3, uv, uvddx, uvddy);
    diffuseColor2.w += color.z;
    diffuseColor3.w += color.w;
    float weight2 = step(diffuseColor2.w, diffuseColor3.w);
    float4 d2 = lerp(diffuseColor2, diffuseColor3, weight2);
    float finalWeigth = step(d1.w, d2.w);
    res.albedo = lerp(d1, d2, finalWeigth);

    float4 normalSample2 = tex2Dgrad(normalmap2, uv, uvddx, uvddy);
    float4 normalSample3 = tex2Dgrad(normalmap3, uv, uvddx, uvddy);
    float4 n2 = lerp(normalSample2, normalSample3, weight2);
    res.normal = lerp(n1, n2, finalWeigth);
#endif
    return res;
}