#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"

vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
    float3 direction : TEXCOORD0;
    float2 normalizedCoordinates : TEXCOORD1;
#if (DIRECTIONAL_LIGHT)
    float lightSolidAngle : TEXCOORD2;
#endif
};

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

#if (DIRECTIONAL_LIGHT)
    float cameraFov = 2.0 * atan(1.0 / projMatrix[0][0]);
    float sunAngularSize = 0.53 * (_PI / 180.0);
    float visibleAngularSize = 2.0 * (sunAngularSize / cameraFov) * step(0.0, dot(cameraDirection, lightPosition0.xyz));
    output.lightSolidAngle = 2.0 * _PI * (1.0 - cos(sunAngularSize));
    float4 center = mul(float4(lightPosition0.xyz, 0.0), viewProjMatrix);
    float2 position = center.xy / center.w + input.position.xy * visibleAngularSize * float2(1.0, viewportSize.x / viewportSize.y);
    float4 unprojected = mul(float4(position, distantDepthValue, 1.0), invProjMatrix);
    float3 direction = mul(float4(unprojected.xyz / unprojected.w, 0.0), invViewMatrix).xyz;
#else
    float4 unprojected = mul(float4(input.position.xy, distantDepthValue, 1.0), invProjMatrix);
    float3 direction = mul(float4(unprojected.xyz / unprojected.w, 0.0), invViewMatrix).xyz;
    float2 position = input.position.xy;
#endif

    output.normalizedCoordinates = input.position.xy;
    output.position = float4(position, distantDepthValue, 1.0);
    output.direction = direction;
    return output;
}
