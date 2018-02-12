#include "include/common.h"
#include "include/shading-options.h"

fragment_in
{
    float2 texCoord : TEXCOORD0;
};

fragment_out
{

#if COPY_GBUFFER_0
    float4 gBuffer0Copy : SV_TARGET0;  
#endif
#if COPY_GBUFFER_1
    float4 gBuffer1Copy : SV_TARGET1;  
#endif
#if COPY_GBUFFER_2
    float4 gBuffer2Copy : SV_TARGET2;  
#endif
#if COPY_GBUFFER_3
    float4 gBuffer3Copy : SV_TARGET3;  
#endif
};

#if COPY_GBUFFER_0
uniform sampler2D gBuffer0;
#endif
#if COPY_GBUFFER_1
uniform sampler2D gBuffer1;
#endif
#if COPY_GBUFFER_2
uniform sampler2D gBuffer2;
#endif
#if COPY_GBUFFER_3
uniform sampler2D gBuffer3;
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    #if COPY_GBUFFER_0
    output.gBuffer0Copy = tex2D(gBuffer0, input.texCoord);  
    #endif
    #if COPY_GBUFFER_1
    output.gBuffer1Copy = tex2D(gBuffer1, input.texCoord);  
    #endif
    #if COPY_GBUFFER_2
    output.gBuffer2Copy = tex2D(gBuffer2, input.texCoord);  
    #endif
    #if COPY_GBUFFER_3
    output.gBuffer3Copy = tex2D(gBuffer3, input.texCoord);  
    #endif

    return output;
}