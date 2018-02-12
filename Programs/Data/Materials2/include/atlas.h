uniform sampler2D textureBlendMask;
[material][a] property float borderSize = 64.0f;
[material][a] property float atlasSize = 2048.0f;
[material][a] property float tileSize = 1024.0f;

const float2 atlasOffsets[4] =
{
  float2(0.0f, 0.0f),
  float2(0.5f, 0.0f),
  float2(0.0f, 0.5f),
  float2(0.5f, 0.5f)
};

struct AtlasParams
{
    int lowerTileIndex;
    int upperTileIndex;
    float heightBias;
};

struct TilesUv
{
    float2 lowerTileUv;
    float2 upperTileUv;
};

struct AtlasSampleResult
{
    float4 color;
    float2 sampleUv;
    float2 uvddx;
    float2 uvddy;
};

AtlasParams GetAtlasParams(float sampleFromMask)
{
    float sampleBiased = sampleFromMask * 3.0f;

    AtlasParams atlasParams;
    atlasParams.lowerTileIndex = floor(sampleBiased);
    atlasParams.upperTileIndex = ceil(sampleBiased);
    atlasParams.heightBias = frac(sampleBiased);

    return atlasParams;
}

TilesUv GetTilesUv(float2 originalUv, AtlasParams params)
{
    float2 offset = float2(borderSize / atlasSize);
    float2 scale = float2(1.0f - 2.0f * borderSize / tileSize);

    TilesUv uvs;
    float2 biasedUv = float2(frac(originalUv) * 0.5f);
    biasedUv = biasedUv * scale + offset;
    uvs.lowerTileUv = biasedUv + atlasOffsets[params.lowerTileIndex];
    uvs.upperTileUv = biasedUv + atlasOffsets[params.upperTileIndex];

    return uvs;
}

AtlasSampleResult SampleAtlas(float4 uv) // uv.xy - tiled coords, uv.zv - coords of the mask.
{
    float maskSample = tex2Dlod(textureBlendMask, uv.zw, 0).a;

    AtlasParams atlasParams = GetAtlasParams(maskSample);
    TilesUv tuv = GetTilesUv(uv.xy, atlasParams);

    float tileScale = tileSize / atlasSize;
    float2 uvddx = ddx(uv.xy) * tileScale;
    float2 uvddy = ddy(uv.xy) * tileScale;

    float4 baseColorSample1 = tex2Dgrad(albedo, tuv.lowerTileUv, uvddx, uvddy); // We need to tell rasterizer not to panic and not to select mip based on partial derivative from teared uv, so we feed dx, dy from the original smooth uv.
    float4 baseColorSample2 = tex2Dgrad(albedo, tuv.upperTileUv, uvddx, uvddy);

    float f = step(baseColorSample1.w, baseColorSample2.w + atlasParams.heightBias);

    AtlasSampleResult result;
    result.color = lerp(baseColorSample1, baseColorSample2, f);
    result.sampleUv = lerp(tuv.lowerTileUv, tuv.upperTileUv, f);
    result.uvddx = uvddx;
    result.uvddy = uvddy;
    return result;
}