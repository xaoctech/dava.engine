

float3 RangeMapping(float3 hdrVal_, float dynamicRangeMin_, float dynamicRangeMax_, float luminance_, float adaptedLuminance_)
{
    float ratio = adaptedLuminance_ / luminance_;
    dynamicRangeMin_ *= ratio;
    dynamicRangeMax_ *= ratio;
    float3 hdrR = hdrVal_ - dynamicRangeMin_;
    hdrR /= dynamicRangeMax_ - dynamicRangeMin_;

    float q = 0.0001f; // Must be the same as in inv.

    float3 hdrL = hdrVal_ * q / (dynamicRangeMin_ + q);

    //return hdrR;
    return max(hdrL, hdrR);
}

float3 RangeMappingInv(float3 hdrVal_, float dynamicRangeMin_, float dynamicRangeMax_, float luminance_, float adoptedLuminance_)
{
    float ratio = adoptedLuminance_ / luminance_;
    dynamicRangeMin_ *= ratio;
    dynamicRangeMax_ *= ratio;
    float3 hdrR = hdrVal_ * (dynamicRangeMax_ - dynamicRangeMin_);
    hdrR += dynamicRangeMin_;

    float q = 0.0001f; // must be synced with inv function

    float3 hdrL = hdrVal_ * (dynamicRangeMin_ + q) / q;

    //return hdrR;
    return min(hdrL, hdrR);
}

float3 ToneMapOpExp(float3 hdrColor)
{
    return 1.0f - exp2(-hdrColor);
}

float3 ToneMapOpExpInv(float3 ldrColor)
{
    return -log2(max(1.0 - ldrColor, 1e-37));
}

float3 ToneMapOpReinhard(float3 hdrColor)
{
    return hdrColor / (hdrColor + 1.0f);
}

float3 ToneMapOpReinhardInv(float3 ldrColor)
{
    return ldrColor / (1.0 - ldrColor);
}

float3 HDRtoLDR(float3 hdrVal, float dynamicRangeMin, float dynamicRangeMax, float luminance, float adoptedLuminance)
{
    float3 col = RangeMapping(hdrVal, dynamicRangeMin, dynamicRangeMax, luminance, adoptedLuminance);
    col = ToneMapOpExp(col);
    return col;
}

float3 LDRtoHDR(float3 ldrVal, float dynamicRangeMin, float dynamicRangeMax, float luminance, float adoptedLuminance)
{
    float3 col = ToneMapOpExpInv(ldrVal);
    col = RangeMappingInv(col, dynamicRangeMin, dynamicRangeMax, luminance, adoptedLuminance);
    return col;
}