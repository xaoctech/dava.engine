[material][a] property float4 landCursorPosition = float4(0.0, 0.0, 1.0, 1.0);
[material][a] property float3 landCursorColor = float4(0.5, 0.5, 1.0);
[material][a] property float2 cursorRotation = 0.0;
[material][a] property float invertFactor = 0.0;

uniform sampler2D landCursorTexture;

float GetBrushMaskFactor(float2 inTexCoord, float4 curPosSize, float2 cursorRot, float invFactor)
{
    float result = 0.0;
    if (curPosSize.z > 0.0 && curPosSize.w > 0.0)
    {
        float2 cursorCoord = (inTexCoord - curPosSize.xy) / curPosSize.zw + float2(0.5, 0.5);
        if (cursorCoord.x == saturate(cursorCoord.x) && cursorCoord.y == saturate(cursorCoord.y))
        {
            float inv = step(0.5, invFactor);
            cursorCoord.y = inv * (1.0 - cursorCoord.y) + cursorCoord.y * (1.0 - inv);

            float2 uv = cursorCoord;
            uv -= 0.5;
            uv = float2(uv.x * cursorRot.y - uv.y * cursorRot.x, uv.x * cursorRot.x + uv.y * cursorRot.y);
            cursorCoord = uv + 0.5;

            result = tex2D(landCursorTexture, cursorCoord).a;
        }
    }
    return result;
}

float SampleToHeight(float4 sampleH)
{
    return dot(sampleH.xy, float2(0.0038910506, 0.9961089494));
}
