#ifndef __TUBUS_HLSLI__
#define __TUBUS_HLSLI__

namespace Tubus
{
	// Gradient vectors for 2D Perlin noise (unit circle directions).
	float2 PerlinGradient(float2 cell)
	{
		float h = dot(cell, float2(127.1, 311.7));
		h = frac(sin(h) * 43758.5453);
		float angle = h * 6.28318530718;
		return float2(cos(angle), sin(angle));
	}

	// Classic 2D Perlin noise. Returns smooth, spatially coherent values in [-1, 1].
	float PerlinNoise2D(float2 p)
	{
		float2 i = floor(p);
		float2 f = frac(p);

		// Quintic interpolation curve for C2 continuity
		float2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

		// Gradient dot products at four corners
		float d00 = dot(PerlinGradient(i + float2(0.0, 0.0)), f - float2(0.0, 0.0));
		float d10 = dot(PerlinGradient(i + float2(1.0, 0.0)), f - float2(1.0, 0.0));
		float d01 = dot(PerlinGradient(i + float2(0.0, 1.0)), f - float2(0.0, 1.0));
		float d11 = dot(PerlinGradient(i + float2(1.0, 1.0)), f - float2(1.0, 1.0));

		return lerp(lerp(d00, d10, u.x), lerp(d01, d11, u.x), u.y);
	}

	// Fractal Brownian Motion using Perlin noise. Produces organic, multi-scale patterns.
	// Returns values roughly in [-1, 1].
	float FBM(float2 p, int octaves)
	{
		float value = 0.0;
		float amplitude = 0.5;
		for (int i = 0; i < octaves; i++) {
			value += amplitude * PerlinNoise2D(p);
			p *= 2.0;
			amplitude *= 0.5;
		}
		return value;
	}

	void OcclusionDiscard(float4 positionCS, uint eyeIndex)
	{
		if (!ForceEnableCulling && (!SharedData::tubusSettings.ShouldEnableCulling || !EnableCulling))
			return;

		float radius = SharedData::tubusSettings.Radius;
		float edgeWidth = SharedData::tubusSettings.EdgeWidth;

		if (radius <= 0)
			return;

		// Reconstruct world position from positionCS (SV_Position) in camera-relative space
		float2 pixelUV = positionCS.xy / SharedData::BufferDim.xy;
		float4 positionNDC = float4(2 * float2(pixelUV.x, -pixelUV.y + 1) - 1, positionCS.z, 1);
		float4 positionWS = mul(FrameBuffer::CameraViewProjInverse[eyeIndex], positionNDC);
		float3 camAdj = FrameBuffer::CameraPosAdjust[eyeIndex].xyz;
		positionWS.xyz /= positionWS.w;

		if (positionWS.z + camAdj.z < SharedData::tubusSettings.PlayerWorldPosition.z + 75) {
			return;
		}

		// Capsule endpoints in camera-relative space
		float3 A = SharedData::tubusSettings.CameraPlayerCollision - camAdj;
		float3 B = SharedData::tubusSettings.PlayerCameraCollision - camAdj;

		// Distance from pixel to capsule (line segment A->B)
		float3 AB = B - A;
		float ABLenSq = dot(AB, AB);
		float t = ABLenSq > 0 ? saturate(dot(positionWS.xyz - A, AB) / ABLenSq) : 0;
		float3 closest = A + t * AB;
		float dist = length(positionWS.xyz - closest);

		// Inner core: hard discard (fully inside the capsule)
		float innerRadius = radius - edgeWidth;
		if (dist <= innerRadius)
			discard;

		// Edge band: Perlin noise island culling
		if (dist <= radius) {
			// edgeFactor: 1 at inner boundary, 0 at outer boundary
			float edgeFactor = 1.0 - saturate((dist - innerRadius) / edgeWidth);

			// Sample Perlin FBM at screen-space coordinates.
			// Scale controls island size — lower values produce larger islands.
			float2 noiseCoord = positionCS.xy * 0.04 + FrameBuffer::CameraPosAdjust[eyeIndex].yx * 0.02;
			float noise = FBM(noiseCoord, 3);

			// Remap from [-1,1] to [0,1]
			noise = (noise + 1) * 0.5;

			// Island culling: noise defines island shapes, edgeFactor controls density.
			float cullStrength = noise * edgeFactor;
			if (cullStrength > 0.35)
				discard;
		}
	}
}

#endif  // __TUBUS_HLSLI__
