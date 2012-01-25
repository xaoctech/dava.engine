attribute vec4 inPosition;
attribute vec3 inNormal;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

uniform vec3 lightPosition0;

varying vec3 varColor;

void main()
{
	vec3 inn = vec3(0.0, 0.0, 0.0);

	varColor = inNormal;

//	vec3 normal = normalize(normalMatrix * inPosition.xyz);
//	vec4 PosView = modelViewMatrix * inPosition;
//	
//	vec3 LightVecView = lightPosition0 - PosView.xyz;
//	
////	if(dot(N.xyz, -LightVecView.xyz) < 0.0)
//	float dotValue = dot(normal, LightVecView);
//	if(dotValue < 0.0)
//	{
////		if(PosView.z > lightPosition0.z)
////		{
////			PosView.xyz += LightVecView * (1000.0 - PosView.z) / LightVecView.z;
////		}
////		else
////		{
////			PosView = vec4(LightVecView, 1.0);
////		}
//		gl_Position = projectionMatrix * PosView;
//	}
//	else
//	{
//		gl_Position = modelViewProjectionMatrix * inPosition;
//	}
	
	gl_Position = modelViewProjectionMatrix * inPosition;
}