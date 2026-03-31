#include "Common/SharedData.hlsli"

Texture2D<float4> PlayerMaskTexture : register(t0);
RWTexture2D<float4> MainRW : register(u0);

cbuffer OutlineSettings : register(b1)
{
	float Thickness;
	float3 pad;
};

[numthreads(8, 8, 1)] void main(uint3 dtid
									: SV_DispatchThreadID)
{
	uint2 dims = uint2(SharedData::BufferDim.xy);
	if (any(dtid.xy >= dims))
		return;

	// Skip pixels that are part of the player character (outline is drawn outside)
	if (any(PlayerMaskTexture[dtid.xy].rbg))
		return;

	int iThickness = (int)Thickness;
	bool foundPlayer = false;

	float4 outlineColor;
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

			if (any(PlayerMaskTexture[coord].rgb)) {
				foundPlayer = true;
				outlineColor = PlayerMaskTexture[coord];
			}
		}
	}


	if (foundPlayer) {
		MainRW[dtid.xy] = outlineColor;
	}
}
