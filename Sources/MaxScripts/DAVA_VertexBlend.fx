string ParamID = "0x003";

float Script : STANDARDSGLOBAL <
    string UIWidget = "none";
    string ScriptClass = "object";
    string ScriptOrder = "standard";
    string ScriptOutput = "color";
    string Script = "Technique=Main;";
> = 0.8;

int texcoord1 : Texcoord
<
	int Texcoord = 1;
	int MapChannel = 0;
	string UIWidget = "None";
>;

int texcoord2 : Texcoord
<
	int Texcoord = 2;
	int MapChannel = 10;
	string UIWidget = "None";
>;

int texcoord3 : Texcoord
<
	int Texcoord = 3;
	int MapChannel = -1;
	string UIWidget = "None";
>;

float3 Lamp0Pos : POSITION <
    string Object = "PointLight0";
    string UIName =  "Light Position";
    string Space = "World";
	int refID = 0;
> = {-0.5f,2.0f,1.25f};

float3 Lamp0Color : LIGHTCOLOR
<
	int LightRef = 0;
	string UIWidget = "None";
> = float3(1.0f, 1.0f, 1.0f);

/////////////////////////// Params ///////////
bool g_UseFourSample <
	string UIName = "Use four textures blend";
> = false;

bool g_ShowOnlyDiffuse <
	string UIName = "Show only diffuse";
> = false;

Texture2D <float4> g_DiffuseT0<
	string UIName = "Diffuse Tile 0";
	string ResourceType = "2D";
	int Texcoord = 0;
	int MapChannel = 1;
>;

Texture2D <float4> g_DiffuseT1<
	string UIName = "Diffuse Tile 1";
	string ResourceType = "2D";
	int Texcoord = 0;
	int MapChannel = 1;
>;

Texture2D <float4> g_DiffuseT2<
	string UIName = "Diffuse Tile 2";
	string ResourceType = "2D";
	int Texcoord = 0;
	int MapChannel = 1;
>;

Texture2D <float4> g_DiffuseT3<
	string UIName = "Diffuse Tile 3";
	string ResourceType = "2D";
	int Texcoord = 0;
	int MapChannel = 1;
>;

bool g_FlipGreen <
	string UIName = "Flip Green in Normal Map";
> = false;

bool orthogonalizeTangentBitangentPerPixel
<
	string UIName = "Orthogonalize per Pixel (Set by 3ds Max)";
> = false;

Texture2D <float4> g_NormalT0 < 
	string UIName = "Normal Tile 0";
	string ResourceType = "2D";
>;

Texture2D <float4> g_NormalT1 < 
	string UIName = "Normal Tile 1";
	string ResourceType = "2D";
>;

Texture2D <float4> g_NormalT2 < 
	string UIName = "Normal Tile 2";
	string ResourceType = "2D";
>;

Texture2D <float4> g_NormalT3 < 
	string UIName = "Normal Tile 3";
	string ResourceType = "2D";
>;


float g_BumpScale <
	string UIName = "Bump Amount";
	string UIWidget = "slider";
	float UIMin = 0.0f;
	float UIMax = 10.0f;
	float UIStep = 0.01f;
>   = 1.0f;

int n<
	string UIName = "Specular Power";
	string UIWidget = "slider";
	float UIMin = 0.0f;
	float UIMax = 50.0f;	
>  = 15;

float4 k_s  <
	string UIName = "Specular";
	string UIWidget = "Color";
> = float4( 1.0f, 1.0f, 1.0f, 1.0f );    // diffuse    // specular

float4 k_a  <
	string UIName = "Ambient";
	string UIWidget = "Color";
> = float4( 0.0f, 0.0f, 0.0f, 1.0f );    // ambient
	
//////////////////////////////////////////////
/////////////////// Samplers /////////////////
SamplerState g_DiffuseSampler
{
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Wrap;
    AddressV = Wrap;	
};
SamplerState g_NormalSampler
{
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Wrap;
    AddressV = Wrap;	
};
//////////////////////////////////////////////
float4x4 WorldITXf : WorldInverseTranspose < string UIWidget="None"; >;
float4x4 WvpXf : WorldViewProjection < string UIWidget="None"; >;
float4x4 WorldXf : World < string UIWidget="None"; >;
float4x4 ViewIXf : ViewInverse < string UIWidget="None"; >;

/* data from application vertex buffer */
struct appdata 
{
	float4 Position		: POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Binormal		: BINORMAL;
	float2 UV0		: TEXCOORD0;	
	float4 Color		: TEXCOORD1;
	float3 Alpha		: TEXCOORD2;
	float3 Illum		: TEXCOORD3;
	float3 UV1		: TEXCOORD4;
	float3 UV2		: TEXCOORD5;
	float3 UV3		: TEXCOORD6;
	float3 UV4		: TEXCOORD7;
};

/* data passed from vertex shader to pixel shader */
struct vertexOutput {
    float4 HPosition	: SV_Position;
    float4 UV0		: TEXCOORD0;
    // The following values are passed in "World" coordinates since
    //   it tends to be the most flexible and easy for handling
    //   reflections, sky lighting, and other "global" effects.
    float3 LightVec	: TEXCOORD1;
    float3 WorldNormal	: TEXCOORD2;
    float3 WorldTangent	: TEXCOORD3;
    float3 WorldBinormal : TEXCOORD4;
    float3 WorldView	: TEXCOORD5;
	float4 Color		: TEXCOORD6;
	float4 UV2		: TEXCOORD7;
	float4 wPos		: TEXCOORD8;
};

vertexOutput vs(appdata i)
{
	vertexOutput o = (vertexOutput)0;
	o.WorldNormal = mul(float4(i.Normal, 0.0f), WorldITXf).xyz;
    o.WorldTangent = mul(float4(i.Tangent, 0.0f),WorldITXf).xyz;
    o.WorldBinormal = mul(float4(i.Binormal, 0.0f),WorldITXf).xyz;
    float4 Po = float4(i.Position.xyz,1);
    float3 Pw = mul(Po,WorldXf).xyz;
    o.LightVec = (Lamp0Pos - Pw);
    o.WorldView = normalize(ViewIXf[3].xyz - Pw);
    o.HPosition = mul(Po,WvpXf);
	o.wPos = mul(i.Position, WorldXf);

	o.UV0.xy = i.UV0.xy;
	o.Color.rgb = i.Color.rgb;
	o.Color.a = i.Alpha.x;
	return o;
}

///////////////////////////////////////
void phong_shading(vertexOutput IN,
		    float3 LightColor,
		    float3 Nn,
		    float3 Ln,
		    float3 Vn,
		    out float3 DiffuseContrib,
		    out float3 SpecularContrib)
{
	float3 specLevel = float3(1.0,1.0,1.0);
		
    float3 Hn = normalize(Vn + Ln);
    float4 litV = lit(dot(Ln,Nn),dot(Hn,Nn),n);
    DiffuseContrib = litV.y * LightColor;
    SpecularContrib = litV.y * litV.z * k_s * LightColor * specLevel;
}
////////////////////////////////////////

float4 ps(vertexOutput i) : SV_Target
{
	float3 diffContrib;
    float3 specContrib;
    float3 Ln = normalize(i.LightVec);
    float3 Vn = normalize(i.WorldView);
    float3 Nn = normalize(i.WorldNormal);
    float3 Tn = normalize(i.WorldTangent);
    float3 Bn = normalize(i.WorldBinormal);

    float2 uv = frac(i.UV0.xy);

    float2 uvddx = ddx(i.UV0.xy);
    float2 uvddy = ddy(i.UV0.xy);

	float4 diffuseColor0 = g_DiffuseT0.SampleGrad(g_DiffuseSampler, frac(uv), uvddx, uvddy);
	float4 diffuseColor1 = g_DiffuseT1.SampleGrad(g_DiffuseSampler, frac(uv), uvddx, uvddy);
	diffuseColor0.w += i.Color.x;
	diffuseColor1.w += i.Color.y;
	float4 normalSample0 = g_NormalT0.SampleGrad(g_NormalSampler, uv, uvddx, uvddy);
	float4 normalSample1 = g_NormalT1.SampleGrad(g_NormalSampler, uv, uvddx, uvddy);
    normalSample0.w = diffuseColor0.w;
    normalSample1.w = diffuseColor1.w;

	float4 d1 = lerp(diffuseColor0, diffuseColor1, step(diffuseColor0.w, diffuseColor1.w));
    float4 n1 = lerp(normalSample0, normalSample1, step(normalSample0.w, normalSample1.w));
	float4 diffRes;
	float4 normRes;

	if (!g_UseFourSample)
	{
		diffRes = d1;
		normRes = n1;
	}
	else
	{
		float4 diffuseColor2 = g_DiffuseT2.SampleGrad(g_DiffuseSampler, frac(uv), uvddx, uvddy);
		float4 diffuseColor3 = g_DiffuseT3.SampleGrad(g_DiffuseSampler, frac(uv), uvddx, uvddy);
		diffuseColor2.w += i.Color.z;
		diffuseColor3.w += i.Color.w;
		float4 d2 = lerp(diffuseColor2, diffuseColor3, step(diffuseColor2.w, diffuseColor3.w));
		diffRes = lerp(d1, d2, step(d1.w, d2.w));

		float4 normalSample2 = g_NormalT2.SampleGrad(g_NormalSampler, uv, uvddx, uvddy);
		float4 normalSample3 = g_NormalT3.SampleGrad(g_NormalSampler, uv, uvddx, uvddy);
	    normalSample2.w = diffuseColor2.w;
	    normalSample3.w = diffuseColor3.w;
		float4 n2 = lerp(normalSample2, normalSample3, step(normalSample2.w, normalSample3.w));
		normRes = lerp(n1, n2, step(n1.w, n2.w));
	}


    float3 bump = 2.0f * (normRes.rgb - 0.5f);

	// Flip green value because by default green means -y in the normal map generated by 3ds Max.
	bump.g = -bump.g;

	if (g_FlipGreen)
	{
		bump.g = -bump.g;
	}

	if (orthogonalizeTangentBitangentPerPixel)
	{
		float3 bitangent = normalize(cross(Nn, Tn));
		Tn = normalize(cross(bitangent, Nn));
		// Bitangent need to be flipped if the map face is flipped. We don't have map face handedness in shader so make
		// the calculated bitangent point in the same direction as the interpolated bitangent which has considered the flip.
		Bn = sign(dot(bitangent, Bn)) * bitangent;
	}

	Nn = g_BumpScale * (bump.x * Tn + bump.y * Bn) + bump.z * Nn;

	Nn = normalize(Nn);

	phong_shading(i, Lamp0Color, Nn, Ln, Vn, diffContrib, specContrib);
	float3 ambientColor = k_a;

	float3 result = specContrib + (diffContrib * diffRes.xyz) + ambientColor;
	if (g_ShowOnlyDiffuse)
		result = diffRes.xyz;

	return float4(result.xyz, 1.0f);
}

technique11 Main_11 <
	string Script = "Pass=p0;";
> {
	pass p0 <
	string Script = "Draw=geometry;";
    > 
    {
        SetVertexShader(CompileShader(vs_5_0,vs()));
        SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0,ps()));
    }
}

technique10 Main_10 <
	string Script = "Pass=p0;";
> {
	pass p0 <
	string Script = "Draw=geometry;";
    > 
    {
        SetVertexShader(CompileShader(vs_4_0,vs()));
        SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_4_0,ps()));
    }
}
