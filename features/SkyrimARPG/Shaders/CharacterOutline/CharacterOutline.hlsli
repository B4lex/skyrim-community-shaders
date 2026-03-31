namespace CharacterOutline
{
	// Dedicated character mask UAV at PS slot 8 (after the 8 deferred MRTs).
	// Bound via OMSetRenderTargetsAndUnorderedAccessViews with UAVStartSlot=8.
	RWTexture2D<unorm float4> CharacterMaskUAV : register(u8);

	void WritePlayerMask(uint2 screenPos)
	{
		bool isOutlined = (Permutation::ExtraShaderDescriptor & Permutation::ExtraFlags::IsOutlined) != 0;
		if (isOutlined)
			CharacterMaskUAV[screenPos] = float4(1.0, 0, 0, 0);
	}
}
