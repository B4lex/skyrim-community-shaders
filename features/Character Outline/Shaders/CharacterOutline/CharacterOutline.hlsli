namespace CharacterOutline
{
	// Dedicated character mask UAV at PS slot 8 (after the 8 deferred MRTs).
	// Bound via OMSetRenderTargetsAndUnorderedAccessViews with UAVStartSlot=8.
	RWTexture2D<unorm float4> CharacterMaskUAV : register(u8);

	void WritePlayerMask(uint2 screenPos)
	{
		bool isPlayer = (Permutation::ExtraShaderDescriptor & Permutation::ExtraFlags::IsPlayerCharacter) != 0;
		bool isNPC = (Permutation::ExtraShaderDescriptor & Permutation::ExtraFlags::IsNPC) != 0;
		if (isPlayer || isNPC)
			CharacterMaskUAV[screenPos] = float4(1.0, 0, 0, 0);
	}
}
