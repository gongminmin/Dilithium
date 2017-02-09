// -*- mode: hlsl; hlsl-entry: VSMain; hlsl-target: vs_6_0; hlsl-args: /Ges /O1; -*-
void VSMain(float4 pos : POSITION,
	float2 tex : TEXCOORD0,
	out float2 oTex : TEXCOORD0,
	out float4 oPos : SV_Position)
{
	oTex = tex;
	oPos = pos;
}
