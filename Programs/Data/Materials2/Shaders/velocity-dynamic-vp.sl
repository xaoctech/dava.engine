#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

[auto][global] property float4x4 prevViewProjMatrix;

vertex_in
{
    float3 position : POSITION;

#if (SPEED_TREE_OBJECT)
    float4 pivot : TEXCOORD4;
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
    float4 currPosition : TEXCOORD0;
    float4 prevPosition : TEXCOORD1;
    float2 scale : TEXCOORD2;
};

float3 AnimateSpeedTree(float3 inputPosition_, float4 pivot_, float flex, float4 wind_, float time, float3 worldScale_, float4x4 invWV)
{
    float3 position = lerp(inputPosition_.xyz, pivot_.xyz, pivot_.w);
    float3 billboardOffset = inputPosition_.xyz - position.xyz;

    // rotate billboards
    float billboardAngle = flex * wind_.w * (1.0 + 0.5 * sin(time * dot(billboardOffset, 1.0)));
    float sinAngle = sin(billboardAngle);
    float cosAngle = cos(billboardAngle);
    float3 billboardOffsetRotated;
    billboardOffsetRotated.x = cosAngle * billboardOffset.x + sinAngle * billboardOffset.y;
    billboardOffsetRotated.y = -sinAngle * billboardOffset.x + cosAngle * billboardOffset.y;
    billboardOffsetRotated.z = billboardOffset.z;

    // rotate leafs
    float3 axis = normalize(float3(-wind_.y, wind_.x, 0.0)); // cross product with up-vector(0.0, 0.0, 1.0);
    float3 offset = inputPosition_ - pivot_.xyz;
    float angle = flex * wind_.w * (1.0 + 0.5 * sin(time * (offset.x + offset.y + offset.z)));
    float3 outPos = rotateVertex(position, pivot_.xyz, axis, angle);
    outPos += mul(float4(billboardOffsetRotated * worldScale_, 0.0), invWV).xyz;
    return outPos;
}

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float3 inputPosition = input.position.xyz;
    float3 inputPositionPrev = inputPosition;

#if SPEED_TREE_OBJECT
    inputPosition = AnimateSpeedTree(inputPosition, input.pivot, flexibility.x, wind, globalTime.x, worldScale, invWorldViewMatrix);
    inputPositionPrev = AnimateSpeedTree(inputPositionPrev, input.pivot, flexibility.y, prevWind, globalTime.y, worldScale, invWorldViewMatrix);
#endif

#if (HARD_SKINNING)

    int jIndex = int(input.index);

    float4 jPPrev = prevJointPositions[jIndex];
    float4 jQPrev = prevJointQuaternions[jIndex];
    inputPositionPrev = JointTransformPosition(inputPosition, jQPrev, jPPrev);

    float4 jP = jointPositions[jIndex];
    float4 jQ = jointQuaternions[jIndex];
    inputPosition = JointTransformPosition(inputPosition, jQ, jP);

#elif (SOFT_SKINNING)

    float4 indices = input.index;
    float4 weights = input.weight;
    float3 skinnedPosition = float3(0.0, 0.0, 0.0);
    float3 skinnedPositionPrev = skinnedPosition;

    for (int i = 0; i < SOFT_SKINNING; ++i)
    {
        int jIndex = int(indices.x);

        float4 jP = jointPositions[jIndex];
        float4 jQ = jointQuaternions[jIndex];
        skinnedPosition += JointTransformPosition(inputPosition, jQ, jP, weights.x);

        float4 jPPrev = prevJointPositions[jIndex];
        float4 jQPrev = prevJointQuaternions[jIndex];
        skinnedPositionPrev += JointTransformPosition(inputPosition, jQPrev, jPPrev, weights.x);

        indices = indices.yzwx;
        weights = weights.yzwx;
    }

    inputPosition = skinnedPosition;
    inputPositionPrev = skinnedPositionPrev;
#endif

    float4 worldPositionCurrent = mul(float4(inputPosition, 1.0f), worldMatrix);
    float4 worldPositionPrev = mul(float4(inputPositionPrev, 1.0f), prevWorldMatrix);

    float4 currPos = mul(worldPositionCurrent, viewProjMatrix);
    output.position = currPos;
    output.currPosition = currPos;

    output.prevPosition = mul(worldPositionPrev, prevViewProjMatrix);

    output.scale = viewportSize / renderTargetSize;

    return output;
}
