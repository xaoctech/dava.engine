#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

vertex_in
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 texCoord0 : TEXCOORD0;
#if (USE_BAKED_LIGHTING)
    float2 texCoord1 : TEXCOORD1;
#endif

#if (SPEED_TREE_OBJECT)
    float4 pivot : TEXCOORD4;
#endif

#if (VERTEX_BAKED_AO || VERTEX_COLOR)
    float4 color : COLOR0;
#endif

#if (SOFT_SKINNING)
    float4 index : BLENDINDICES;
    float4 weight : BLENDWEIGHT;
#elif (HARD_SKINNING)
    float index : BLENDINDICES;
#endif
};

vertex_out
{
    float4 position : SV_POSITION;
    float4 uv : TEXCOORD0;
    float3 tbnToWorld0 : TEXCOORD1;
    float4 tbnToWorld1 : TEXCOORD2; // w - distance to camera.
    float4 tbnToWorld2 : TEXCOORD3;
    float4 projectedPosition : TEXCOORD4;
#if (VERTEX_BAKED_AO)
    float vertexBakedAO : COLOR0;
#endif
#if (VERTEX_COLOR)
    float4 vertexColor : COLOR1;
#endif
};

#if (SOFT_SKINNING) || (HARD_SKINNING)
[auto][jpos] property float4 jointPositions[MAX_JOINTS] : "bigarray"; // (x, y, z, scale)
[auto][jrot] property float4 jointQuaternions[MAX_JOINTS] : "bigarray";
#endif

vertex_out vp_main(vertex_in input)
{
    float3 inputPosition = input.position.xyz;
    float3 inputNormal = input.normal;
    float3 inputTangent = input.tangent;
    float3 inputBinormal = input.binormal;

#if SPEED_TREE_OBJECT
    float3 position = lerp(inputPosition.xyz, input.pivot.xyz, input.pivot.w);
    float3 billboardOffset = inputPosition.xyz - position.xyz;

    // rotate billboards
    float billboardAngle = flexibility * wind.w * (1.0 + 0.5 * sin(globalTime * (billboardOffset.x + billboardOffset.y + billboardOffset.z)));
    float sinAngle = sin(billboardAngle);
    float cosAngle = cos(billboardAngle);
    float3 billboardOffsetRotated;
    billboardOffsetRotated.x = cosAngle * billboardOffset.x + sinAngle * billboardOffset.y;
    billboardOffsetRotated.y = -sinAngle * billboardOffset.x + cosAngle * billboardOffset.y;
    billboardOffsetRotated.z = billboardOffset.z;

    // GFX_COMPLETE why not bake TBN for billboards?
    if (input.pivot.w > 0.5)
    {
        inputNormal = normalize(inputPosition + input.pivot.xyz);
        float3 up = (abs(inputNormal.z) < 0.95) ? float3(0.0, 0.0, 1.0) : float3(0.0, -1.0, 0.0);
        inputTangent = normalize(cross(up, inputNormal));
        inputBinormal = cross(inputNormal, inputTangent);
    }

    // rotate leafs
    float3 axis = normalize(float3(-wind.y, wind.x, 0.0)); //cross product with up-vector(0.0, 0.0, 1.0);
    float3 offset = inputPosition - input.pivot.xyz;
    float angle = flexibility * wind.w * (1.0 + 0.5 * sin(globalTime * (offset.x + offset.y + offset.z)));
    inputPosition = rotateVertex(position, input.pivot.xyz, axis, angle);
    inputPosition += mul(float4(billboardOffsetRotated * worldScale, 0.0), invWorldViewMatrix).xyz;
#endif

#if (HARD_SKINNING)

    int jIndex = int(input.index);
    float4 jP = jointPositions[jIndex];
    float4 jQ = jointQuaternions[jIndex];

    inputPosition = JointTransformPosition(inputPosition, jQ, jP);
    inputNormal = JointTransformTangent(inputNormal, jQ);
    inputTangent = JointTransformTangent(inputTangent, jQ);
    inputBinormal = JointTransformTangent(inputBinormal, jQ);

#elif (SOFT_SKINNING)

    float4 indices = input.index;
    float4 weights = input.weight;
    float3 skinnedPosition = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < SOFT_SKINNING; ++i)
    {
        int jIndex = int(indices.x);

        float4 jP = jointPositions[jIndex];
        float4 jQ = jointQuaternions[jIndex];

        skinnedPosition += JointTransformPosition(inputPosition, jQ, jP, weights.x);
        inputNormal = JointTransformTangent(inputNormal, jQ, weights.x);
        inputTangent = JointTransformTangent(inputTangent, jQ, weights.x);
        inputBinormal = JointTransformTangent(inputBinormal, jQ, weights.x);

        indices = indices.yzwx;
        weights = weights.yzwx;
    }

    inputPosition = skinnedPosition;

#endif

    float3 worldNormal = normalize(mul(float4(inputNormal, 0.0), worldInvTransposeMatrix).xyz);
    float3 worldTangent = normalize(mul(float4(inputTangent, 0.0), worldInvTransposeMatrix).xyz);
    float3 worldBinormal = normalize(mul(float4(inputBinormal, 0.0), worldInvTransposeMatrix).xyz);
    float4 worldPosition = mul(float4(inputPosition, 1.0), worldMatrix);
    float3 toEye = cameraPosition - worldPosition.xyz;

    vertex_out output;
    output.position = mul(worldPosition, viewProjMatrix);
    output.tbnToWorld0 = float3(worldTangent.x, worldBinormal.x, worldNormal.x);
    output.tbnToWorld1.xyz = float3(worldTangent.y, worldBinormal.y, worldNormal.y);
    output.tbnToWorld1.w = length(toEye);
    output.tbnToWorld2.xyz = float3(worldTangent.z, worldBinormal.z, worldNormal.z);
    output.tbnToWorld2.w = dot(worldNormal, toEye);
    output.projectedPosition = output.position;

#if (USE_BAKED_LIGHTING)
    output.uv = float4(texCoordScale * input.texCoord0, shadowaoUV.xy + shadowaoUV.zw * input.texCoord1);
#else
    output.uv = float4(input.texCoord0, 0.0, 0.0);
#endif

#if (VERTEX_BAKED_AO)
    output.vertexBakedAO = input.color.x;
#endif

#if (VERTEX_COLOR)
    output.vertexColor = input.color;
#endif

    return output;
}
