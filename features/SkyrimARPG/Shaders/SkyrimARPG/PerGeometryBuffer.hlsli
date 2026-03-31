cbuffer PerGeometryTubus : register(SKYRIM_ARPG_PER_GEOMETRY_REGISTER)
{
	float4 OutlineColor : packoffset(c0);
	bool EnableCulling : packoffset(c1.x);
	bool ForceEnableCulling : packoffset(c1.y);
}
