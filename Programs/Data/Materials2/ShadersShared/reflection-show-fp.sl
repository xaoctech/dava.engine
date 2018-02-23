#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;
    float3 varToCamera : TEXCOORD1;
    float3 tangentToFinal0 : TEXCOORD3;
    float3 tangentToFinal1 : TEXCOORD4;
    float3 tangentToFinal2 : TEXCOORD5;
};

fragment_out
{
#if (FOWARD_WITH_COMBINE)
    float4 color : SV_TARGET1;
#else
    float4 color : SV_TARGET0;
#endif
};

[material][a] property float specularLod = 0.0;
[material][a] property float specularContribution = 1.0;

fragment_out fp_main(fragment_in input)
{
    float3 v = normalize(input.varToCamera);
    float3 n = normalize(float3(input.tangentToFinal0.z, input.tangentToFinal1.z, input.tangentToFinal2.z));
    float3 r = reflect(-v, n);

    float4 diffuse = 0.0;
    diffuse += sphericalHarmonics[0] * (0.282095);
    diffuse += sphericalHarmonics[1] * (0.488603 * n.y);
    diffuse += sphericalHarmonics[2] * (0.488603 * n.z);
    diffuse += sphericalHarmonics[3] * (0.488603 * n.x);
    diffuse += sphericalHarmonics[4] * (1.092548 * n.x * n.y);
    diffuse += sphericalHarmonics[5] * (1.092548 * n.y * n.z);
    diffuse += sphericalHarmonics[6] * (0.315392 * (3.0 * n.z * n.z - 1.0));
    diffuse += sphericalHarmonics[7] * (1.092548 * n.x * n.z);
    diffuse += sphericalHarmonics[8] * (0.546274 * (n.x * n.x - n.y * n.y));

    float4 specular = texCUBElod(globalReflection, r, specularLod);

    fragment_out output;
    output.color = lerp(diffuse, specular, specularContribution);
    return output;
}
