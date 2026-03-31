#include "Common/SharedData.hlsli"

Texture2D<float4> PlayerMaskTexture : register(t0);
RWTexture2D<float4> MainRW : register(u0);

cbuffer OutlineSettings : register(b1)
{
	float4 OutlineColor;
	float Thickness;
	float3 pad;
};

[numthreads(8, 8, 1)] void main(uint3 dtid
									: SV_DispatchThreadID)
{
	uint2 dims = uint2(SharedData::BufferDim.xy);
	if (any(dtid.xy >= dims))
		return;

	float centerMask = PlayerMaskTexture[dtid.xy].r;

	// Skip pixels that are part of the player character (outline is drawn outside)
	if (centerMask > 0.5)
		return;

	int iThickness = (int)Thickness;
	bool foundPlayer = false;

	// Search neighboring pixels for player mask within thickness radius
	[loop] for (int y = -iThickness; y <= iThickness && !foundPlayer; y++)
	{
		[loop] for (int x = -iThickness; x <= iThickness && !foundPlayer; x++)
		{
			if (x == 0 && y == 0)
				continue;

			// Circular shape: skip corners outside the radius
			if (x * x + y * y > iThickness * iThickness)
				continue;

			int2 coord = (int2)dtid.xy + int2(x, y);

			// Bounds check
			if (any(coord < 0) || any(coord >= (int2)dims))
				continue;

			if (PlayerMaskTexture[coord].r > 0.5)
				foundPlayer = true;
		}
	}

	if (foundPlayer) {
		float4 currentColor = MainRW[dtid.xy];
		MainRW[dtid.xy] = float4(lerp(currentColor.rgb, OutlineColor.rgb, OutlineColor.a), currentColor.a);
	}
}
