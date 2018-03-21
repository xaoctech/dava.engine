#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"
#include "include/ibl.h"

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

fragment_out fp_main(fragment_in input)
{
    fragment_out output; 

#if (INTEGRATE_BRDF_LOOKUP)

    output.color = IntegrateBRDFLookupTexture(input.varTexCoord0);

#elif (DIFFUSE_SPHERICAL_HARMONICS)

    int index = int(clamp(input.varTexCoord0.x * 4.5 + 4.0, 0.0, 8.0));

    SphericalHarmonics conv = ConvoluteSphericalHarmonics(0.0);
    float3 sh[9];
    sh[0] = conv.sh0;
    sh[1] = conv.sh1;
    sh[2] = conv.sh2;
    sh[3] = conv.sh3;
    sh[4] = conv.sh4;
    sh[5] = conv.sh5;
    sh[6] = conv.sh6;
    sh[7] = conv.sh7;
    sh[8] = conv.sh8;
    output.color = float4(sh[index], 1.0);

#else

    Convolution convolution;
    convolution.cubemap_basis_a = cubemap_basis_a;
    convolution.cubemap_basis_b = cubemap_basis_b;
    convolution.cubemap_basis_c = cubemap_basis_c;
    convolution.cubemap_dst_last_mip = cubemap_dst_last_mip;
    convolution.cubemap_src_last_mip = cubemap_src_last_mip;
    convolution.cubemap_dst_mip_level = cubemap_dst_mip_level;
    convolution.cubemap_src_mip_level = cubemap_src_mip_level;
    convolution.cubemap_face_size = cubemap_face_size;
    convolution.dynamicHammersleySetSize = dynamicHammersleySetSize;
    convolution.dynamicHammersleySetIndex = dynamicHammersleySetIndex;

    #if (SPECULAR_CONVOLUTION)
    output.color = ConvoluteSpecular(input.varTexCoord0, convolution);
    #elif (DIFFUSE_CONVOLUTION)
    output.color = ConvoluteDiffuse(input.varTexCoord0, convolution);
    #elif (DOWNSAMPLING)
    output.color = Downsample(input.varTexCoord0, convolution);
    #else
    output.color = float4(1000.0, 0.0, 1000.0, 1.0);
    #endif

#endif

    return output;
}
