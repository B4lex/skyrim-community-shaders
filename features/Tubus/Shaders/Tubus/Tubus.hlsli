#ifndef __TUBUS_HLSLI__
#define __TUBUS_HLSLI__

namespace Tubus
{
	void OcclusionDiscard(float4 positionCS, uint eyeIndex)
	{
	    #if defined(SKINNED)
		return;
		#endif
		float radius = SharedData::tubusSettings.Radius;
		if (radius <= 0)
			return;

		// Player position in camera model space
		float3 playerCameraSpace = SharedData::tubusSettings.PlayerWorldPosition -
		                           FrameBuffer::CameraPosAdjust[eyeIndex].xyz;

		// Project player to clip space
		float4 playerClip = mul(FrameBuffer::CameraViewProj[eyeIndex],
			float4(playerCameraSpace, 1.0));

		float zoomMult = length(playerCameraSpace) * 0.0005;
		radius = radius / zoomMult - 0.2;
		// Player screen UV and depth
		float2 playerUV = (playerClip.xy / playerClip.w) * float2(0.5, -0.5) + float2(0.5, 0.45);



		float playerDepth = playerClip.z / playerClip.w;

		// Current pixel screen UV
		float2 pixelUV = positionCS.xy / SharedData::BufferDim.xy;

		// Screen-space distance with aspect ratio correction
		float2 diff = pixelUV - playerUV;
		diff.x *= SharedData::BufferDim.x / SharedData::BufferDim.y;
		float dist = length(diff);

		// Soft edge via smoothstep
		float falloff = SharedData::tubusSettings.Falloff;
		float alpha = smoothstep(radius - falloff, radius, dist);

		// Discard if within circle AND pixel is between camera and player
		if (alpha < 1.0 && positionCS.z < playerDepth - 0.0255 * radius)
			discard;
	}
}

#endif  // __TUBUS_HLSLI__
