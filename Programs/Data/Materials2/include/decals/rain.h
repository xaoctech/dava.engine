//let albedo == smp_flow
//let normalmap == smp_ripples

float2 ComputeRipple(float2 uv, float time, float weight, RainDecalParameters rippleParams)
{
    float pi = 3.1415926535897932384626433832795;
    float4 ripple = tex2Dlod(normalmap, uv, 0.0);
    ripple.yz = ripple.yz * 2.0 - 1.0;
    float drop_frac = frac(ripple.w + time);
    float time_frac = drop_frac - 1.0 + ripple.x;
    float drop_factor = saturate(0.2 + weight * 0.8 - drop_frac);
    float final_factor = drop_factor * ripple.x * sin(clamp(time_frac * 9.0, 0.0, 3.0) * pi);

    return ripple.yz * final_factor * rippleParams.ripples.x;
}

float2 ComputeRipplesNormal(float3 worldPos, float2 decalUV, float3 w_normal_src, RainDecalParameters rainDecalParams)
{
    float time_ripples = rainDecalParams.time * rainDecalParams.ripples.z;
    float time_flow = rainDecalParams.time * rainDecalParams.flow.z;

    float3 w_coord = worldPos;
    w_coord.z += time_flow;

    // calculate ripples
    float4 weights = rainDecalParams.rain_intensity - float4(0, 0.25, 0.5, 0.75);
    weights = saturate(weights * 4.0);

    float4 time_mul = float4(1.0, 0.85, 0.93, 1.13);
    float4 time_add = float4(0.0, 0.20, 0.45, 0.70);
    float4 times = (time_ripples * time_mul + time_add);
    times = frac(times);

    float4 uv_offset = float4(w_coord.x * rainDecalParams.ripples.y + time_ripples / rainDecalParams.ripples.w,
                              w_coord.x * rainDecalParams.ripples.y - time_ripples / rainDecalParams.ripples.w,
                              w_coord.y * rainDecalParams.ripples.y + time_ripples / rainDecalParams.ripples.w,
                              w_coord.y * rainDecalParams.ripples.y - time_ripples / rainDecalParams.ripples.w);

    float2 ripple_0 = ComputeRipple(float2(uv_offset.x + 0.25, uv_offset.z + 0.00), times.x, weights.x, rainDecalParams);
    float2 ripple_1 = ComputeRipple(float2(uv_offset.x - 0.55, uv_offset.w + 0.30), times.y, weights.y, rainDecalParams);
    float2 ripple_2 = ComputeRipple(float2(uv_offset.y + 0.60, uv_offset.z + 0.85), times.z, weights.z, rainDecalParams);
    float2 ripple_3 = ComputeRipple(float2(uv_offset.y + 0.50, uv_offset.w - 0.75), times.w, weights.w, rainDecalParams);

    float2 ripples_normal = float2(weights.x * ripple_0 +
                                   weights.y * ripple_1 +
                                   weights.z * ripple_2 +
                                   weights.w * ripple_3);

    //ripples_normal = ripple_0;
    // calculate flow
    float2 wave_mask = float2(saturate(w_normal_src.z), saturate(w_normal_src.z + 0.8));

    float3 blendVal = abs(w_normal_src);
    //blendVal = normalize(max(blendVal, 0.00001));

    float2 uv_rnd = tex2Dlod(albedo, float2(decalUV + frac(time_flow * 0.025)), 0.0).xy;

    float2 x_normal = (tex2D(albedo, float2(w_coord.zy + uv_rnd * 0.2)).xy * 2.0 - 1.0) * rainDecalParams.flow.x;
    float2 y_normal = (tex2D(albedo, float2(w_coord.xz + uv_rnd * 0.2)).xy * 2.0 - 1.0) * rainDecalParams.flow.x;

    // blend normals
    float2 normal = x_normal * wave_mask.y * blendVal.x +
    ripples_normal * wave_mask.x * blendVal.z * blendVal.z +
    y_normal * wave_mask.y * blendVal.y;

    return normal;
}
