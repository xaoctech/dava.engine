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

    SphericalHarmonics conv = ConvoluteSphericalHarmonics(0.0);

    float isNegative = step(input.varTexCoord0.x, 0.0);
    float isPositive = step(0.0, input.varTexCoord0.x);
    float absoluteIndex = floor(abs(input.varTexCoord0.x) * 10.0);
    float t = 0.5 * (isNegative * (8.0 - absoluteIndex) + isPositive * (8.0 + absoluteIndex) + 1.0);
    int index = int(t);

    output.color = conv.sh[index];

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
