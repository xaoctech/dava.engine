#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

vertex_in
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord0 : TEXCOORD0;
    
#if (USE_TEXCOORD_1)
    float2 texCoord1 : TEXCOORD1;
#endif
    
#if (SPEED_TREE_OBJECT)
    float4 pivot : TEXCOORD4;
#endif
    
#if (INPUT_VERTEX_COLOR)
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
    float4 varTexCoord : TEXCOORD0;
#if (WRITE_SHADOW_MAP == 0)
    float4 worldPosition : TEXCOORD1;
    float4 projectedPosition : TEXCOORD2;
    float4 shadowTexCoord : TEXCOORD3;
    float4 varToCamera : TEXCOORD4;
    float3 normal : NORMAL0;
#if (VERTEX_BAKED_AO)
    float vertexBakedAO : COLOR0;
#endif
#if (EMIT_VERTEX_COLOR)
    float4 vertexColor : COLOR1;
#endif
#endif
};

vertex_out vp_main(vertex_in input)
{
    float3 inputPosition = input.position.xyz;
    float3 inputNormal = input.normal;
    
#if SPEED_TREE_OBJECT
    float3 position = lerp(inputPosition.xyz, input.pivot.xyz, input.pivot.w);
    float3 billboardOffset = inputPosition.xyz - position.xyz;

    // rotate billboards
    float billboardAngle = flexibility.x * wind.w * (1.0 + 0.5 * sin(globalTime.x * dot(billboardOffset, 1.0)));
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
    }

    // rotate leafs
    float3 axis = normalize(float3(-wind.y, wind.x, 0.0)); // cross product with up-vector(0.0, 0.0, 1.0);
    float3 offset = inputPosition - input.pivot.xyz;
    float angle = flexibility.x * wind.w * (1.0 + 0.5 * sin(globalTime.x * (offset.x + offset.y + offset.z)));
    inputPosition = rotateVertex(position, input.pivot.xyz, axis, angle);

    inputPosition += mul(float4(billboardOffsetRotated * worldScale, 0.0), invWorldViewMatrix).xyz;
#endif
    
#if (HARD_SKINNING)

    int jIndex = int(input.index);
    float4 jP = jointPositions[jIndex];
    float4 jQ = jointQuaternions[jIndex];

    inputPosition = JointTransformPosition(inputPosition, jQ, jP);
    inputNormal = JointTransformTangent(inputNormal, jQ);
    
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

        indices = indices.yzwx;
        weights = weights.yzwx;
    }

    inputPosition = skinnedPosition;
    
#endif

    vertex_out output;
    
#if (WRITE_SHADOW_MAP)

    float4 worldPosition = mul(float4(inputPosition, 1.0), worldMatrix);
    output.varTexCoord = float4(input.texCoord0, 0.0, 0.0 /* lightmap not used in shadow write */);
    output.position = mul(worldPosition, viewProjMatrix);
    
#else
    
#if (USE_TEXCOORD_1)
    output.varTexCoord = float4(input.texCoord0, input.texCoord1 * uvScale + uvOffset);
#else
    output.varTexCoord = float4(input.texCoord0, 0.0, 0.0);
#endif
    
#if (VERTEX_BAKED_AO)
    output.vertexBakedAO = input.color.x;
#endif
    
#if (EMIT_VERTEX_COLOR)
    output.vertexColor = input.color;
#endif

    output.worldPosition = mul(float4(inputPosition, 1.0), worldMatrix);
    output.projectedPosition = mul(output.worldPosition, viewProjMatrix);
    output.normal = mul(float4(inputNormal, 0.0), worldInvTransposeMatrix).xyz;
    float3 toCamera = cameraPosition - output.worldPosition.xyz;
    
#if (FLIP_BACKFACE_NORMALS)
    output.normal *= sign(dot(output.normal, toCamera));
#endif

    output.varToCamera = float4(toCamera, 1.0);
    output.shadowTexCoord = mul(output.worldPosition, shadowView);
    output.position = output.projectedPosition;
#endif

    return output;
}
